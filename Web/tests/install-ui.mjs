import {chromium,webkit} from 'playwright';
import assert from 'node:assert/strict';
import fs from 'node:fs/promises';
const url=process.argv[2]||'http://127.0.0.1:8941/',out=process.argv[3]||'.local/web-install-scroll';
await fs.mkdir(out,{recursive:true});
const results=[];
for(const engine of ['chromium','webkit']){
 const browser=await (engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
 try{
  const context=await browser.newContext({viewport:{width:1024,height:768}}),page=await context.newPage(),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});
  // Inspect each browser's actual no-prompt path, then simulate platform identity only.
  await page.evaluate(()=>{installPrompt=null;updateInstall();});
  assert.equal(await page.locator('#install').textContent(),'HOW TO INSTALL');
  await page.click('#install');assert(await page.locator('#installHelp').evaluate(e=>document.activeElement===e));
  assert.match(await page.locator('#installHelp').textContent(),engine==='webkit'?/Add to Dock/:/Install page as app/);
  await page.evaluate(()=>{
   Object.defineProperty(navigator,'userAgent',{configurable:true,value:'Mozilla/5.0 (Macintosh) Version/17.0 Safari/605.1.15'});
   Object.defineProperty(navigator,'maxTouchPoints',{configurable:true,value:5});updateInstall();
  });
  assert.match(await page.locator('#installHelp').textContent(),/Add to Home Screen/);
  await page.evaluate(()=>{Object.defineProperty(navigator,'standalone',{configurable:true,value:true});updateInstall();});
  assert(await page.locator('#install').isHidden());assert.equal(await page.locator('#play').textContent(),'PLAY');
  await page.evaluate(()=>{Object.defineProperty(navigator,'standalone',{configurable:true,value:false});updateInstall();});
  for(const outcome of ['dismissed','accepted']){
   await page.evaluate(outcome=>{
    const event=new Event('beforeinstallprompt',{cancelable:true});window.promptCalls=0;
    event.prompt=async()=>{window.promptCalls++;};event.userChoice=Promise.resolve({outcome});dispatchEvent(event);
   },outcome);
   assert.equal(await page.locator('#install').textContent(),'INSTALL GAME');
   await page.click('#install');await page.waitForFunction(()=>!document.querySelector('#install').disabled);
   assert.equal(await page.evaluate(()=>window.promptCalls),1);
   if(outcome==='dismissed')assert.equal(await page.locator('#install').textContent(),'HOW TO INSTALL');
   else assert(await page.locator('#install').isHidden());
  }
  await page.evaluate(()=>dispatchEvent(new Event('appinstalled')));assert(await page.locator('#install').isHidden());
  await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);
  await page.evaluate(()=>MDTest.advance(2));assert((await page.evaluate(()=>MDTest.sprites())).some(s=>s.image==='Title.jpg'));
  assert.equal((await page.evaluate(()=>MDTest.state())).writes,0);assert.deepEqual(errors,[]);
  await page.screenshot({path:`${out}/${engine}-title.png`});
  results.push({engine,noPromptHelp:true,macHelp:true,ipadHelp:true,standalone:true,promptDismissed:true,promptAccepted:true,appinstalled:true,gameStarted:true,noSaveWrites:true});
  await context.close();
 }finally{await browser.close();}
}
await fs.writeFile(`${out}/install-ui.json`,JSON.stringify(results,null,2));console.log(JSON.stringify(results));
