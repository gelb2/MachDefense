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
 for(const level of [1,10]){
  const initial=await call(20,level);assert.deepEqual(initial.actors.filter(a=>a.height>0).map(a=>a.weapons),[2,5,2,4]);
  let state;for(let i=0;i<600;++i){state=await call(1);if(i===180)await page.locator('#game').screenshot({path:`${out}/level-${level}.png`});}
  assert(state.sounds>0&&state.writes===0);assert(state.actors.filter(a=>a.height===0).every(a=>a.hp<10000000));result.levels.push({level,initial,final:state});
 }
 await call(0);await call(7);let seen=false;
 for(let i=0;i<900;++i){let s=await call(1);if(!seen&&s.countermeasureBursts>0){for(let j=0;j<15;++j)s=await call(1);await page.locator('#game').screenshot({path:`${out}/dodge-flare.png`});seen=true;}}
 assert(seen);result.dodge=true;
 result.performance=[];
 for(const count of (process.env.MD_SKIP_PERF?[]:[40,100]))for(const enabled of [false,true]){
  await call(21,count);
  if(enabled)await call(22);
  for(let i=0;i<120;++i)await call(1);
  const metric=await page.evaluate(async({count,enabled})=>{
   const times=[],cpu=[];let prior=0,maxParticles=0,state;const start=await MDTest.state();const heapStart=MDModule.HEAPU8.buffer.byteLength;
   for(let i=0;i<600;++i){await new Promise(resolve=>requestAnimationFrame(now=>{if(prior)times.push(now-prior);prior=now;resolve();}));const begin=performance.now();
    if(enabled&&i%120===0)await MDTest.call('md_aircraft_scene',['number','number'],[22,3],'string');
    state=JSON.parse(await MDTest.call('md_aircraft_scene',['number','number'],[1,3],'string'));cpu.push(performance.now()-begin);maxParticles=Math.max(maxParticles,state.countermeasureParticles);
   }
   const end=await MDTest.state(),heapEnd=MDModule.HEAPU8.buffer.byteLength;times.sort((a,b)=>a-b);cpu.sort((a,b)=>a-b);const pct=(a,p)=>a[Math.floor((a.length-1)*p)];
   return {count,enabled,frames:600,p50:pct(times,.5),p95:pct(times,.95),p99:pct(times,.99),over25ms:times.filter(x=>x>25).length,cpuP99:pct(cpu,.99),maxParticles,uploadsDuring:end.uploads-start.uploads,heapStart,heapEnd,writes:state.writes};
  },{count,enabled});
  assert.equal(metric.writes,0);assert.equal(metric.uploadsDuring,0);assert.equal(metric.heapEnd,metric.heapStart);assert(metric.maxParticles<=96);if(enabled)assert.equal(metric.maxParticles,84);
  result.performance.push(metric);
  if(enabled){await call(22);for(let j=0;j<20;++j)await call(1);await page.locator('#game').screenshot({path:`${out}/stress-${count}.png`});}
 }
 result.audio=await page.evaluate(()=>({...audioLifetime}));assert.deepEqual(result.audio,initialAudio);assert.deepEqual(errors,[]);result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;await page.screenshot({path:`${out}/failed.png`});}
finally{result.errors=errors;await fs.writeFile(`${out}/result.json`,JSON.stringify(result,null,2));console.log(JSON.stringify({ok:result.ok,error:result.error,performance:result.performance,audio:result.audio}));await browser.close();}
