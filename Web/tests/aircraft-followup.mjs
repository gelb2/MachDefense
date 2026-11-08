import {chromium} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const [out,url]=process.argv.slice(2);await fs.mkdir(out,{recursive:true});
const browser=await chromium.launch({headless:true,executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'});
const page=await browser.newPage({viewport:{width:900,height:900}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
const result={};
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(1));
 const call=(c,m=3)=>page.evaluate(([c,m])=>MDTest.call('md_aircraft_scene',['number','number'],[c,m],'string'),[c,m]);
 if(!process.env.MD_PERF_ONLY){
 await call(8,7);await page.locator('#game').screenshot({path:`${out}/painted-assembly.png`});
 await call(21,1);await call(22);
 for(let i=0;i<30;i++){await call(1);if(i===14||i===29)await page.locator('#game').screenshot({path:`${out}/flare-${i+1}.png`});}
 }
 result.performance=[];
 for(const count of (process.env.MD_PERF_ONLY?[100]:[40,100]))for(const enabled of [false,true]){
  await call(21,count);await call(22);for(let i=0;i<120;i++)await call(1);
  const metric=await page.evaluate(async({count,enabled})=>{
   const times=[];let prior=0,maxParticles=0,state;const start=await MDTest.state();const heap=MDModule.HEAPU8.buffer.byteLength;
   for(let i=0;i<600;i++){
    await new Promise(resolve=>requestAnimationFrame(now=>{if(prior)times.push(now-prior);prior=now;resolve();}));
    if(enabled&&i%120===0)await MDTest.call('md_aircraft_scene',['number','number'],[22,3],'string');
    state=JSON.parse(await MDTest.call('md_aircraft_scene',['number','number'],[1,3],'string'));maxParticles=Math.max(maxParticles,state.countermeasureParticles);
   }
   times.sort((a,b)=>a-b);const end=await MDTest.state();return {count,enabled,frames:600,p99:times[Math.floor(times.length*.99)],over25ms:times.filter(t=>t>25).length,maxParticles,uploads:end.uploads-start.uploads,heapStable:heap===MDModule.HEAPU8.buffer.byteLength,writes:state.writes};
  },{count,enabled});
  result.performance.push(metric);assert.equal(metric.maxParticles,enabled?132:0);assert.equal(metric.uploads,0);assert(metric.heapStable);assert.equal(metric.writes,0);
 }
 assert.deepEqual(errors,[]);result.ok=true;
}finally{result.errors=errors;await fs.writeFile(`${out}/result.json`,JSON.stringify(result,null,2));console.log(JSON.stringify(result));await browser.close();}
