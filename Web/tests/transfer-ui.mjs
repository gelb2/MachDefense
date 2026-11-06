import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const root=process.argv[2],url=process.argv[3],engine=process.env.MD_BROWSER||'chromium';
const code=await fs.readFile(root+'/iphone/recovery.txt','utf8');
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const context=await browser.newContext({viewport:{width:390,height:844}});if(engine==='chromium')await context.grantPermissions(['clipboard-read','clipboard-write']);
const page=await context.newPage();const results={engine,sizes:[]};
async function tick(){await page.evaluate(()=>MDTest.advance(1));}
async function tap(x,y){const r=await page.locator('#game').boundingBox();await page.mouse.click(r.x+x*r.width,r.y+y*r.height);await tick();}
async function state(){return page.evaluate(()=>MDTest.state());}
async function open(){const s=await state();await tap((s.width/2+75)/s.width,((s.height-480)/2+72)/s.height);await page.locator('#transferInput').waitFor({state:'visible'});}
try{
 await page.goto(url,{waitUntil:'networkidle'});await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(60));await tap(.5,.5);await page.evaluate(()=>MDTest.advance(160));
 await open();await page.locator('#transferInput').fill(code);
 for(const size of [{width:390,height:844},{width:820,height:1180},{width:1180,height:820},{width:360,height:740}]){
  await page.setViewportSize(size);await page.waitForTimeout(100);await tick();const s=await state();const r=await page.locator('#transferInput').boundingBox();assert(r.x>=0&&r.y>=0&&r.x+r.width<=size.width+.1&&r.y+r.height<=size.height+.1);assert.equal(await page.locator('#transferInput').inputValue(),code);assert.equal(s.writes,0);results.sizes.push({size,field:r});
 }
 await page.screenshot({path:root+'/transfer-input-'+engine+'.png'});
 // Simulate the visible viewport shrinking for an IME. Game actions stay reachable.
 await page.setViewportSize({width:360,height:420});await page.waitForTimeout(100);await tick();let s=await state();let r=await page.locator('#transferInput').boundingBox();assert(r.y+r.height<420);await tap((s.width/2+75)/s.width,71/s.height);await page.waitForTimeout(100);assert(await page.locator('#transferInput').isHidden());assert.equal((await state()).writes,0);results.keyboardResizeCancel=true;
 await page.setViewportSize({width:390,height:844});await page.waitForTimeout(100);await tick();s=await state();await tap((s.width/2-75)/s.width,((s.height-480)/2+72)/s.height);const exported=await page.locator('#transferInput').inputValue();assert(exported.startsWith('MD1-'));assert(await page.locator('#transferInput').evaluate(e=>e.readOnly));s=await state();await tap((s.width/2-75)/s.width,71/s.height);
 if(engine==='chromium'){await page.waitForTimeout(100);assert.equal(await page.evaluate(()=>navigator.clipboard.readText()),exported);results.clipboard=true;}
 assert.equal((await state()).writes,0);results.noWrites=true;results.ok=true;
}catch(e){results.error=e.stack;process.exitCode=1;}finally{await fs.writeFile(root+'/transfer-ui-'+engine+'.json',JSON.stringify(results,null,2));console.log(JSON.stringify(results));await browser.close();}

process.exit(process.exitCode||0);
