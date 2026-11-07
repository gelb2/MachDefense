import {chromium} from 'playwright';
import fs from 'node:fs/promises';import path from 'node:path';import assert from 'node:assert/strict';import crypto from 'node:crypto';
const [mode,root,url,expectedVersion]=process.argv.slice(2);
assert(['prepare','verify'].includes(mode));await fs.mkdir(root,{recursive:true});
const context=await chromium.launchPersistentContext(path.resolve(root,'update-profile'),{headless:true,executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',viewport:{width:420,height:912}});
let page=await context.newPage();const result={mode,url};
const boot=async()=>{await page.goto(url,{waitUntil:'networkidle',timeout:120000});await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(60));};
try{
 await boot();
 if(mode==='prepare'){
  const saved=Array.from(await fs.readFile(path.join(root,'iphone/MachDefense.sav')));
  assert(await page.evaluate(b=>MDStore.write(new Uint8Array(b)),saved));
  result.version=await page.evaluate(async()=> (await (await fetch('build.json')).json()).version);
  await fs.writeFile(path.join(root,'update-before.json'),JSON.stringify({version:result.version,sha:crypto.createHash('sha256').update(new Uint8Array(saved)).digest('hex')}));
 }else{
  const before=JSON.parse(await fs.readFile(path.join(root,'update-before.json'),'utf8'));
  await page.evaluate(async()=>{const r=await navigator.serviceWorker.getRegistration();await r.update();if(r.installing)await new Promise((resolve,reject)=>{const w=r.installing;const t=setTimeout(()=>reject(new Error("Worker install timed out")),120000);w.addEventListener("statechange",()=>{if(["installed","activated"].includes(w.state)){clearTimeout(t);resolve();}else if(w.state==="redundant"){clearTimeout(t);reject(new Error("Worker redundant"));}});});});
  // Closing every old client lets the waiting service worker activate.
  for(const p of context.pages())await p.close();page=await context.newPage();await boot();
  assert.equal(await page.evaluate(async()=>(await(await fetch('build.json')).json()).version),expectedVersion);
  const saved=await page.evaluate(()=>Array.from(MDStore.current));
  assert.equal(crypto.createHash('sha256').update(new Uint8Array(saved)).digest('hex'),before.sha);
  result.version=expectedVersion;result.previousVersion=before.version;result.savePreserved=true;
  const names=['Tables/WeaponInfo.tsv','Tables/MachList.tsv','Tables/BaseUpgrade.tsv','Tables/MachBuild_Name.tsv'];
  for(const name of names){const remote=await page.evaluate(n=>Array.from(MDModule.FS.readFile('/assets/'+n)),name);assert.equal(crypto.createHash('sha256').update(new Uint8Array(remote)).digest('hex'),crypto.createHash('sha256').update(await fs.readFile('Resources/'+name)).digest('hex'));}
  result.runtimeTablesMatch=true;
  await context.setOffline(true);await boot();assert.deepEqual(await page.evaluate(()=>Array.from(MDStore.current)),saved);result.offlineSavePreserved=true;
  const keys=await page.evaluate(()=>caches.keys());assert(keys.includes('machdefense-assets-'+expectedVersion));assert(keys.includes('machdefense-assets-'+before.version));result.previousCacheRetained=true;
 }
 result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;}finally{await fs.writeFile(path.join(root,'release-update-'+mode+'.json'),JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));await context.close();}
