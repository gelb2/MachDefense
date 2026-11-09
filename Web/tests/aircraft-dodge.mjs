import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const out=process.argv[2],url=process.argv[3],engine=process.env.MD_BROWSER||'chromium';await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:640,height:640}});const errors=[];page.on('pageerror',e=>errors.push(e.message));let result={engine};
try{await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(1));
const call=async command=>JSON.parse(await page.evaluate(command=>MDTest.call('md_aircraft_scene',['number','number'],[command,3],'string'),command));await call(0);await call(7);
const trace=[];let captured=false;
for(let i=0;i<900;++i){const s=await call(1);trace.push(s);if(!captured&&s.actors.some(a=>a.dodging)){await page.locator('#game').screenshot({path:out+'/dodge-active.png'});captured=true;}}
assert(captured);assert(trace.some(s=>s.actors.some(a=>a.dodges>=2)));assert(trace.every(s=>s.writes===0));assert.deepEqual(errors,[]);
let protectedFrames=0,damageFrames=0;const previous=new Map();
for(const s of trace)for(const a of s.actors.filter(a=>a.height>0)){const key=a.name+':'+s.actors.indexOf(a);const old=previous.get(key);if(old){if(a.dodging&&old.dodging&&a.hp===old.hp)++protectedFrames;if(a.hp<old.hp)++damageFrames;}previous.set(key,a);}
assert(protectedFrames>0);result={...result,ok:true,protectedFrames,damageFrames,last:trace.at(-1),errors};await fs.writeFile(out+'/trace.json',JSON.stringify(trace));
}catch(e){result.error=e.stack;process.exitCode=1;await page.screenshot({path:out+'/failed.png'});}finally{await fs.writeFile(out+'/result.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result));await browser.close();}
