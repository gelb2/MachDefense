import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const [out,url,fixture]=process.argv.slice(2),engine=process.env.MD_BROWSER||'chromium';
await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:820,height:1180}}),errors=[],result={engine};page.on('pageerror',e=>errors.push(e.message));
const advance=n=>page.evaluate(n=>MDTest.advance(n),n),state=()=>page.evaluate(()=>MDTest.state()),saved=()=>page.evaluate(()=>MDTest.save());
async function tap(x,y){const s=await state(),r=await page.locator('#game').boundingBox();await page.mouse.click(r.x+x/s.width*r.width,r.y+y/s.height*r.height);await advance(1);}
async function launch(){await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await advance(60);}
async function select(){const s=await state();await tap(s.width/2,s.height/2);await advance(160);await tap(s.width/2,(s.height-480)/2+140);await tap(s.width/2,(s.height-480)/2+140);await advance(160);}
try{
 await page.goto(url,{waitUntil:'networkidle',timeout:120000});await page.waitForSelector('#play:not([disabled])',{timeout:120000});
 assert(await page.evaluate(b=>MDStore.write(new Uint8Array(b)),Array.from(await fs.readFile(fixture))));
 await launch();const before=await saved();assert(before[0].active&&before[0].stage>=30);await select();const after=await saved();assert.equal(after[0].builds,before[0].builds+1);assert.equal(after[0].credits,before[0].credits);assert.equal(after[0].stage,before[0].stage);assert.deepEqual(after.slice(1),before.slice(1));
 const writes=(await state()).writes;await advance(600);assert.equal((await state()).writes,writes);result.singleGrant=true;result.noFrameWrites=true;result.otherSlotsUnchanged=true;
 await page.reload({waitUntil:'networkidle'});await page.waitForSelector('#play:not([disabled])',{timeout:120000});await launch();assert.deepEqual(await saved(),after);await select();assert.deepEqual(await saved(),after);result.relaunchNoDuplicate=true;
 assert.deepEqual(errors,[]);result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;}finally{result.errors=errors;await fs.writeFile(out+'/laser-unlock-'+engine+'.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));await browser.close();}
