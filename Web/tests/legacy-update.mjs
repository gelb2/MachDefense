import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import http from 'node:http';import path from 'node:path';import assert from 'node:assert/strict';import crypto from 'node:crypto';
const [legacy,current,out,savePath]=process.argv.slice(2);let active=path.resolve(legacy);
const mime={'.html':'text/html','.js':'text/javascript','.json':'application/json','.wasm':'application/wasm','.css':'text/css','.webmanifest':'application/manifest+json'};
const server=http.createServer(async(req,res)=>{try{const name=decodeURIComponent(new URL(req.url,'http://localhost').pathname).slice(1)||'index.html';if(name.includes('..'))throw Error();const b=await fs.readFile(path.join(active,name));res.writeHead(200,{'Content-Type':mime[path.extname(name)]||'application/octet-stream','Cache-Control':'no-store'});res.end(b);}catch{res.writeHead(404);res.end();}});
await new Promise(r=>server.listen(0,'127.0.0.1',r));const url=`http://127.0.0.1:${server.address().port}/`,engine=process.env.MD_BROWSER||'chromium';
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='webkit'?{}:{channel:'chrome'})});const context=await browser.newContext();let page=await context.newPage();const result={engine,url};
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const boot=async()=>{await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:180000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden,null,{timeout:120000});await page.evaluate(()=>MDTest.advance(60));};
try{
 await boot();const old=JSON.parse(await fs.readFile(path.join(legacy,'build.json'))),next=JSON.parse(await fs.readFile(path.join(current,'build.json')));result.before=old.version;result.after=next.version;
 const save=await fs.readFile(savePath);assert(await page.evaluate(b=>MDStore.write(new Uint8Array(b)),Array.from(save)));
 active=path.resolve(current);await page.evaluate(async()=>{const r=await navigator.serviceWorker.getRegistration();await r.update();});await page.evaluate(async()=>{window.MDTestRegistration=await navigator.serviceWorker.getRegistration();});await page.waitForFunction(()=>!!window.MDTestRegistration.waiting,null,{timeout:180000});
 assert.equal(await page.evaluate(async()=>(await(await fetch('build.json')).json()).version),old.version);result.oldGameNotReplaced=true;
 result.beforeClose=await page.evaluate(async()=>{const r=await navigator.serviceWorker.getRegistration();return {active:r.active?.state,waiting:r.waiting?.state};});await page.close();page=await context.newPage();await boot();result.afterOpen=await page.evaluate(async()=>{const r=await navigator.serviceWorker.getRegistration();return {active:r.active?.state,waiting:r.waiting?.state,controller:navigator.serviceWorker.controller?.state,check:window.MDResourceCheck,caches:await caches.keys()};});
 assert.equal(await page.evaluate(async()=>(await(await fetch('build.json')).json()).version),next.version);
 assert.equal(hash(new Uint8Array(await page.evaluate(()=>Array.from(MDStore.current)))),hash(save));result.savePreserved=true;
 const table=await page.evaluate(()=>Array.from(MDModule.FS.readFile('/assets/Tables/MachBuild_Name.tsv')));assert.equal(hash(new Uint8Array(table)),hash(await fs.readFile('Resources/Tables/MachBuild_Name.tsv')));result.runtimePriceTableMatches=true;
 await context.setOffline(true);await boot();assert.equal(hash(new Uint8Array(await page.evaluate(()=>Array.from(MDStore.current)))),hash(save));result.offline=true;
 const caches=await page.evaluate(()=>globalThis.caches.keys());assert(caches.includes('machdefense-assets-'+next.version));assert(caches.includes('machdefense-assets-'+old.version));result.previousCacheRetained=true;result.pass=true;
}catch(e){result.error=e.stack;process.exitCode=1;}finally{await fs.writeFile(out,JSON.stringify(result,null,2));console.log(JSON.stringify(result));await browser.close();server.close();}
