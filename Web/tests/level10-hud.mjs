import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const out=process.argv[2],url=process.argv[3],engine=process.env.MD_BROWSER||'chromium';
await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:820,height:1180}}),errors=[],result={engine};page.on('pageerror',e=>errors.push(e.message));
const call=(name,types=[],args=[],ret='number')=>page.evaluate(async({name,types,args,ret})=>MDTest.call(name,types,args,ret),{name,types,args,ret});
const advance=n=>page.evaluate(n=>MDTest.advance(n),n),cmd=n=>call('md_command',['number'],[n]),state=()=>page.evaluate(()=>MDTest.state());
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await advance(60);
 const before=await state();result.sweep=await call('md_level10_sweep',[],[],'string');assert(result.sweep.startsWith('Level10/HUD:'),result.sweep);assert.equal((await state()).writes,before.writes);await page.screenshot({path:out+'/level10-sweep-'+engine+'.png'});
 result.content=await call('md_content_sweep',[],[],'string');assert(result.content.startsWith('Content: builds=280 placements=120 specials=100'),result.content);assert.equal((await state()).writes,before.writes);
 await cmd(3);
 // Walk all fourteen original production categories; existing starter rows toggle off.
 for(let i=0;i<14;i++){await cmd(204);await cmd(205);await cmd(201);}await cmd(6);await advance(240);await page.mouse.click(400,1000);await advance(180);await cmd(12);
 let sprites=await page.evaluate(()=>MDTest.sprites());let rows=sprites.filter(s=>s.image==='Btn_BuildMach.png');assert(rows.length>2);const first=JSON.stringify(rows),s=await state();const canvas=await page.locator('#game').boundingBox();const x=canvas.x+(s.width-25)/s.width*canvas.width,y=canvas.y+150/s.height*canvas.height;
 await page.mouse.move(x,y);await page.mouse.wheel(0,3000);await page.waitForTimeout(120);await advance(1);rows=(await page.evaluate(()=>MDTest.sprites())).filter(s=>s.image==='Btn_BuildMach.png');assert.notEqual(JSON.stringify(rows),first,'Wheel did not scroll production HUD');
 const bottom=JSON.stringify(rows);await page.screenshot({path:out+'/hud-bottom-'+engine+'.png'});
 await page.mouse.wheel(0,-3000);await page.waitForTimeout(120);await advance(1);assert.notEqual(JSON.stringify((await page.evaluate(()=>MDTest.sprites())).filter(s=>s.image==='Btn_BuildMach.png')),bottom);
 const writes=(await state()).writes;await page.mouse.move(x,y+400);await page.mouse.down();await page.mouse.move(x,y,{steps:12});await page.mouse.up();await advance(1);assert.equal((await state()).writes,writes);result.pointerAndWheel=true;result.noSaveWrites=true;
 assert.deepEqual(errors,[]);result.errors=errors;result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;await page.screenshot({path:out+'/level10-failed-'+engine+'.png'});}finally{await fs.writeFile(out+'/level10-'+engine+'.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));await browser.close();}
