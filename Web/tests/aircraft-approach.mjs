import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const [out,url]=process.argv.slice(2),engine=process.env.MD_BROWSER||'chromium';await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:800,height:800}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
await page.addInitScript(()=>{window.audioLifetime={created:0,closed:0};for(const name of ['AudioContext','webkitAudioContext']){const Original=window[name];if(!Original)continue;window[name]=new Proxy(Original,{construct(target,args){++audioLifetime.created;const ctx=Reflect.construct(target,args);const close=ctx.close.bind(ctx);ctx.close=(...args)=>{++audioLifetime.closed;return close(...args);};return ctx;}});}});
let result={engine,url};
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(1));
 const call=async(command,mode=3)=>JSON.parse(await page.evaluate(([c,m])=>MDTest.call('md_aircraft_scene',['number','number'],[c,m],'string'),[command,mode]));
 const initialAudio=await page.evaluate(()=>({...audioLifetime}));result.initialAudio=initialAudio;
 result.levels=[];
 for(const level of [1,2,3,4,5,6,7,8,9,10]){
  const initial=await call(20,level);assert.deepEqual(initial.actors.filter(a=>a.height>0).map(a=>a.weapons),[2,5,2,4]);
  const approach={};for(const a of initial.actors.filter(a=>a.height>0))approach[a.name]={minDistance:1e9,minOwnTargetDistance:1e9,path:0,maxSpeed:0,previous:{x:a.x,y:a.y}};
  let state;for(let i=0;i<600;++i){state=await call(1);
   for(const a of state.actors.filter(a=>a.height>0)){const m=approach[a.name];m.path+=Math.hypot(a.x-m.previous.x,a.y-m.previous.y);m.previous={x:a.x,y:a.y};m.maxSpeed=Math.max(m.maxSpeed,a.speed);const own=state.actors[state.actors.findIndex(x=>x.name===a.name)+1];m.minOwnTargetDistance=Math.min(m.minOwnTargetDistance,Math.hypot(a.x-own.x,a.y-own.y));for(const t of state.actors.filter(a=>a.height===0))m.minDistance=Math.min(m.minDistance,Math.hypot(a.x-t.x,a.y-t.y));}
if(i===180)await page.locator('#game').screenshot({path:`${out}/level-${level}.png`});}
  assert(state.sounds>0&&state.writes===0);assert(state.actors.filter(a=>a.height===0).every(a=>a.hp<10000000));for(const [name,m] of Object.entries(approach)){const radius=name.startsWith("AircraftLight")?12:name.startsWith("AircraftHeavy")?28:14;assert(m.path>10&&m.maxSpeed>0&&m.minOwnTargetDistance>=radius*2);delete m.previous;}result.levels.push({level,initial,final:state,approach});
 }
 await call(0);await call(7);let seen=false;
 for(let i=0;i<900;++i){let s=await call(1);if(!seen&&s.countermeasureBursts>0){for(let j=0;j<15;++j)s=await call(1);await page.locator('#game').screenshot({path:`${out}/dodge-flare.png`});seen=true;}}
 assert(seen);result.dodge=true;
 result.audio=await page.evaluate(()=>({...audioLifetime}));assert.deepEqual(result.audio,initialAudio);assert.deepEqual(errors,[]);result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;await page.screenshot({path:`${out}/failed.png`});}
finally{result.errors=errors;await fs.writeFile(`${out}/result.json`,JSON.stringify(result,null,2));console.log(JSON.stringify({ok:result.ok,error:result.error,performance:result.performance,audio:result.audio}));await browser.close();}
