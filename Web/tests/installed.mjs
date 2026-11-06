// Installed-window check, adapted from HeavyMach Distribution/Web/tests/pwa.mjs.
import puppeteer from 'puppeteer-core';
import fs from 'node:fs/promises';
import assert from 'node:assert/strict';
const url=process.argv[2],out=process.argv[3];
await fs.mkdir(out,{recursive:true});
const browser=await puppeteer.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:false,pipe:true,defaultViewport:{width:500,height:900}});
let page=await browser.newPage();const result={url,steps:[]};
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:180000});
 const cdp=await page.createCDPSession();const manifest=await cdp.send('Page.getAppManifest');
 assert.equal(manifest.manifest.id,url);result.installability=await cdp.send('Page.getInstallabilityErrors');
 await cdp.send('PWA.install',{manifestId:url});
 result.osApp=await cdp.send('PWA.getOsAppState',{manifestId:url});
 await cdp.send('PWA.changeAppUserSettings',{manifestId:url,displayMode:'standalone'});
 const old=page;await cdp.send('PWA.launch',{manifestId:url});
 page=await(await browser.waitForTarget(t=>t.type()==='page'&&t.url()===url&&t!==old.target(),{timeout:30000})).page();
 await old.close();await page.waitForSelector('#play:not([disabled])',{timeout:180000});
 assert(await page.evaluate(()=>matchMedia('(display-mode: standalone)').matches));result.steps.push('installed standalone');
 await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden,{timeout:120000});
 await page.evaluate(()=>MDTest.advance(60));result.title=await page.evaluate(()=>MDTest.state());
 await page.evaluate(async()=>{await MDTest.tap(.5,.5);await MDTest.advance(120);});
 await page.screenshot({path:out+'/installed-slots.png'});result.steps.push('title and slots');
 await page.setOfflineMode(true);await page.reload({waitUntil:'networkidle0'});await page.waitForSelector('#play:not([disabled])',{timeout:60000});await page.click('#play');
 await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden,{timeout:120000});
 await page.evaluate(()=>MDTest.advance(60));await page.screenshot({path:out+'/installed-offline.png'});result.steps.push('offline relaunch');result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;await page.screenshot({path:out+'/failure.png'}).catch(()=>{});}finally{await browser.close();await fs.writeFile(out+'/installed.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result));}
