import {chromium,webkit} from 'playwright';
import http from 'node:http';import fs from 'node:fs/promises';import path from 'node:path';import assert from 'node:assert/strict';import crypto from 'node:crypto';
const dist=path.resolve(process.argv[2]||'build/web-normal/dist'),out=process.argv[3]||'.local/update-integrity.json';
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const original=JSON.parse(await fs.readFile(path.join(dist,'asset-manifest.json'))),sw=await fs.readFile(path.join(dist,'sw.js'),'utf8');
const diskFiles=new Map();for(const entry of await fs.readdir(dist,{withFileTypes:true})){if(entry.isFile())diskFiles.set(entry.name,await fs.readFile(path.join(dist,entry.name)));else if(entry.name==='chunks')for(const name of await fs.readdir(path.join(dist,'chunks')))diskFiles.set('chunks/'+name,await fs.readFile(path.join(dist,'chunks',name)));}
let edition=0,corrupt=null;const requests=[];
const title=await fs.readFile(path.join(dist,'title.jpg'));
const changedData=Buffer.from(diskFiles.get('game.data'));changedData[changedData.length-1]=changedData.at(-1)===32?10:32;
const dataFile=original.files.find(f=>f.url==='game.data'),lastPart=dataFile.parts.at(-1);
const changedPart=Buffer.from(diskFiles.get(lastPart.url));changedPart[changedPart.length-1]=changedData.at(-1);
const changedPartHash=hash(changedPart),changedPartURL='chunks/'+changedPartHash+'.bin';diskFiles.set(changedPartURL,changedPart);
function manifest(){const m=structuredClone(original);if(edition){m.version=original.version+'-'+edition;const f=m.files.find(f=>f.url==='title.jpg');const b=Buffer.concat([title,Buffer.from('update'+edition)]);f.size=b.length;f.sha256=hash(b);}if(edition>=3){const d=m.files.find(f=>f.url==='game.data');d.sha256=hash(changedData);Object.assign(d.parts.at(-1),{url:changedPartURL,sha256:changedPartHash});}return m;}
const types={'.html':'text/html','.js':'text/javascript','.json':'application/json','.wasm':'application/wasm','.css':'text/css','.webmanifest':'application/manifest+json'};
const server=http.createServer(async(req,res)=>{try{
 const name=decodeURIComponent(new URL(req.url,'http://localhost').pathname.slice(1))||'index.html';if(name.includes('..'))throw Error('path');requests.push(name);
 let bytes=diskFiles.get(name);if(!bytes)throw Error('missing');const m=manifest();
 if(name==='sw.js')bytes=Buffer.from(sw.replace(`VERSION='${original.version}'`,`VERSION='${m.version}'`).replace(/const MANIFEST=.*;\n/,`const MANIFEST=${JSON.stringify(m)};\n`));
 if(name==='asset-manifest.json')bytes=Buffer.from(JSON.stringify(m));
 if(name==='build.json')bytes=Buffer.from(JSON.stringify({version:m.version,cheat:m.cheat}));
 if(name==='game.data'&&edition>=3)bytes=changedData;
 if(name==='title.jpg'&&edition)bytes=Buffer.concat([title,Buffer.from('update'+edition)]);
 if(name===corrupt)bytes=Buffer.from('damaged');
 res.writeHead(200,{'Content-Type':types[path.extname(name)]||'application/octet-stream','Cache-Control':'no-store'});res.end(bytes);
 }catch{res.writeHead(404);res.end();}});
await new Promise(r=>server.listen(0,'127.0.0.1',r));const url=`http://127.0.0.1:${server.address().port}/`;
const kind=process.env.MD_BROWSER||'chrome';const browser=await (kind==='webkit'?webkit:chromium).launch(kind==='webkit'?{headless:true}:{channel:'chrome',headless:true});
const context=await browser.newContext();let page=await context.newPage();const result={browser:kind,errors:[]};page.on("response",async r=>{if(r.status()===503)result.errors.push(r.url()+": "+await r.text().catch(()=>"?"));});page.on("pageerror",e=>result.errors.push(e.message));page.on("console",e=>{if(e.type()==="error")result.errors.push(e.text());});
await page.addInitScript(()=>navigator.serviceWorker?.addEventListener('message',e=>{if(e.data.kind==='resource-error')console.error(JSON.stringify(e.data));}));
async function ready(){await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:180000});}
try{
 // Reject a bad first install before showing the Play button.
 corrupt='title.jpg';await page.goto(url);await page.waitForSelector('#error:not([hidden])',{timeout:120000});assert(await page.$eval('#play',e=>e.disabled));result.corruptInstallRejected=true;
 corrupt=null;await ready();const initial=await page.evaluate(()=>MDResourceCheck);result.storage=await page.evaluate(()=>navigator.storage.estimate());assert.equal(initial.version,original.version);result.initial=initial;result.cacheCount=await page.evaluate(async()=>(await caches.keys()).length);
 await page.click('#play');await page.waitForFunction(()=>!!window.MDModule);await page.evaluate(()=>MDTest.advance(60));result.gameStarted=true;
 const seed=Array.from(Buffer.alloc(42496));assert(await page.evaluate(b=>MDStore.write(new Uint8Array(b)),seed));
 // While a game is open, the new worker must wait.
 edition=1;await page.evaluate(async()=>{const r=await navigator.serviceWorker.getRegistration();await r.update();});await page.waitForFunction(async()=>!!(await navigator.serviceWorker.getRegistration()).waiting,null,{timeout:180000});
 assert.equal(await page.evaluate(async()=>(await(await fetch('build.json')).json()).version),original.version);result.runningVersionHeld=true;
 const other=await context.newPage();await other.goto(url);await other.waitForSelector('#play:not([disabled])',{timeout:180000});assert.equal(await other.evaluate(()=>MDUpdateWaiting),true);await other.close();result.otherPlayingTabHeld=true;
 requests.length=0;await ready();assert.equal(await page.evaluate(()=>MDResourceCheck.version),manifest().version);assert.deepEqual(await page.evaluate(()=>Array.from(MDStore.current)),seed);result.firstLauncherUsesNewVersion=true;result.savePreserved=true;
 assert(!requests.some(n=>['game.data','game.wasm','game.js'].includes(n)||n.startsWith('chunks/')));result.unchangedGameNotDownloaded=true;
 // A broken update leaves the complete previous release playable.
 edition=2;corrupt='title.jpg';await ready();assert.equal(await page.evaluate(()=>MDResourceCheck.version),original.version+'-1');result.badUpdateKeepsPrevious=true;corrupt=null;
 // Corrupt cached wasm; it must be repaired and checked, not silently executed.
 await page.evaluate(async()=>{const c=await caches.open('machdefense-assets-'+MDResourceCheck.version);await c.put(new URL('game.wasm',location.href),new Response('bad'));});
 edition=1;requests.length=0;await ready();assert(requests.includes('game.wasm')||await page.evaluate(()=>MDResourceCheck.reused>0));
 const wasm=await page.evaluate(async()=>Array.from(new Uint8Array(await(await fetch('game.wasm')).arrayBuffer())));assert.equal(hash(Buffer.from(wasm)),original.files.find(f=>f.url==='game.wasm').sha256);result.corruptCacheRepaired=true;
 edition=3;requests.length=0;await ready();assert.equal(await page.evaluate(()=>MDResourceCheck.version),manifest().version);
 assert.deepEqual(requests.filter(n=>n.startsWith('chunks/')),[changedPartURL]);assert(!requests.includes('game.data'));result.changedChunkOnly=true;result.changedChunkBytes=changedPart.length;result.fullPackageBytes=changedData.length;
 const assembled=await page.evaluate(async()=>Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',await(await fetch('game.data')).arrayBuffer())),x=>x.toString(16).padStart(2,'0')).join(''));assert.equal(assembled,hash(changedData));result.assembledPackageVerified=true;
 await context.setOffline(true);await ready();assert.deepEqual(await page.evaluate(()=>Array.from(MDStore.current)),seed);await page.click('#play');await page.waitForFunction(()=>!!window.MDModule);await page.evaluate(()=>MDTest.advance(60));result.offlineGameAndSave=true;
 result.ok=true;
}catch(error){result.error=error.stack;result.screen=await page.locator("#error").textContent().catch(()=>null);process.exitCode=1;}finally{await fs.mkdir(path.dirname(out),{recursive:true});await fs.writeFile(out,JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));await browser.close();await new Promise(r=>server.close(r));}
