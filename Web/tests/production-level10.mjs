import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const [out,url,fixture]=process.argv.slice(2),engine=process.env.MD_BROWSER||'chromium';
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const ctx=await browser.newContext({viewport:{width:820,height:1180}}),page=await ctx.newPage(),errors=[],result={engine,url};page.on('pageerror',e=>errors.push(e.message));
const advance=n=>page.evaluate(n=>MDTest.advance(n),n),sprites=()=>page.evaluate(()=>MDTest.sprites()),state=()=>page.evaluate(()=>MDTest.state());
async function tap(x,y){const s=await state(),r=await page.locator('#game').boundingBox();await page.mouse.click(r.x+x/s.width*r.width,r.y+y/s.height*r.height);await advance(1);}
async function press(name){const b=(await sprites()).find(b=>b.image===name);assert(b,'Missing '+name);await tap(b.x+b.w/2,b.y+b.h/2);}
try{
 await page.goto(url,{waitUntil:'networkidle',timeout:120000});await page.waitForSelector('#play:not([disabled])',{timeout:120000});
 assert(await page.evaluate(b=>MDStore.write(new Uint8Array(b)),Array.from(await fs.readFile(fixture))));
 await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await advance(60);assert.equal((await state()).cheat,false);
 const s=await state();await tap(s.width/2,s.height/2);await advance(160);await tap(s.width/2,(s.height-480)/2+140);await advance(160);await press('Common_Btn_OK.png');await advance(240);
 let rows=(await sprites()).filter(x=>x.image==='Btn_BuildMach.png');assert(rows.length>2);const top=JSON.stringify(rows);const r=await page.locator('#game').boundingBox();await page.mouse.move(r.x+r.width-30,r.y+250);await page.mouse.wheel(0,5000);await page.waitForTimeout(150);await advance(1);rows=(await sprites()).filter(x=>x.image==='Btn_BuildMach.png');assert.notEqual(JSON.stringify(rows),top);result.normalProductionScroll=true;
 await page.screenshot({path:out+'/production-level10-'+engine+'.png'});
 await tap(s.width-26,s.height-77);await advance(5);await tap(170+(s.width-320)/2,132.25+(s.height-480)/2);assert((await sprites()).some(x=>x.image==='UpgradeBtn_Uninstall.png'));result.level10UpgradeDialog=true;
 await page.screenshot({path:out+'/production-upgrades-'+engine+'.png'});
 assert.deepEqual(errors,[]);result.errors=errors;result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;await page.screenshot({path:out+'/production-failure-'+engine+'.png'});}finally{await fs.writeFile(out+'/production-level10-'+engine+'.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));await browser.close();}
