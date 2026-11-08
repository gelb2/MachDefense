import {chromium,webkit} from '../../../Web/tests/node_modules/playwright/index.mjs';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const [out,url]=process.argv.slice(2),engine=process.env.MD_BROWSER||'chromium';await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:800,height:800}}),errors=[];page.on('pageerror',e=>errors.push(e.message));let result={engine,url,testOnlyChange:'md_aircraft_scene command21 production firing=true; original source unmodified',cases:[]};
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(1));
 await page.evaluate(()=>{window.imageReads=[];const original=MDModule.FS.open;MDModule.FS.open=function(path,...args){if(typeof path==='string'&&path.includes('/Images/'))imageReads.push({path,at:Date.now()});return original.call(this,path,...args);};});
 for(const enabled of [false,true]){
  const metric=await page.evaluate(async enabled=>{
   const call=async(c,m=3)=>JSON.parse(await MDTest.call('md_aircraft_scene',['number','number'],[c,m],'string'));
   const initial=await call(21,100);if(enabled)await call(22);
   for(let i=0;i<120;++i)await call(1);
   const gl=document.querySelector('#game').getContext('webgl2'),ext=gl.getExtension('EXT_disjoint_timer_query_webgl2'),debug=gl.getExtension('WEBGL_debug_renderer_info');
   const pending=[],gpu=[],cpu=[],frames=[],longFrames=[];let prior=0,disjoint=0,maxParticles=0,maxProjectiles=0,state;
   await new Promise(requestAnimationFrame);await new Promise(requestAnimationFrame);const readsStart=imageReads.length,startUTC=new Date().toISOString();const start=await MDTest.state(),heapStart=MDModule.HEAPU8.buffer.byteLength;
   const collect=()=>{if(ext&&gl.getParameter(ext.GPU_DISJOINT_EXT)){++disjoint;for(const q of pending)gl.deleteQuery(q);pending.length=0;gpu.length=0;return;}while(pending.length&&gl.getQueryParameter(pending[0],gl.QUERY_RESULT_AVAILABLE)){const q=pending.shift();gpu.push(gl.getQueryParameter(q,gl.QUERY_RESULT)/1e6);gl.deleteQuery(q);}};
   for(let i=0;i<600;++i){await new Promise(resolve=>requestAnimationFrame(now=>{if(prior){frames.push(now-prior);if(now-prior>25)longFrames.push({frame:i,ms:now-prior,at:new Date().toISOString()});}prior=now;resolve();}));collect();const q=ext?gl.createQuery():null;if(q)gl.beginQuery(ext.TIME_ELAPSED_EXT,q);const begin=performance.now();if(enabled&&i%120===0)await call(22);state=await call(1);cpu.push(performance.now()-begin);if(q){gl.endQuery(ext.TIME_ELAPSED_EXT);pending.push(q);}maxParticles=Math.max(maxParticles,state.countermeasureParticles);maxProjectiles=Math.max(maxProjectiles,state.projectiles);}
   for(let i=0;i<120&&pending.length;++i){await new Promise(requestAnimationFrame);collect();}const uncollected=pending.length;for(const q of pending)gl.deleteQuery(q);
   const end=await MDTest.state();const summary=a=>{a.sort((x,y)=>x-y);return {samples:a.length,p50:a[Math.floor((a.length-1)*.5)]??null,p95:a[Math.floor((a.length-1)*.95)]??null,p99:a[Math.floor((a.length-1)*.99)]??null,max:a.at(-1)??null};};
   return {startUTC,endUTC:new Date().toISOString(),imageReadsDuring:imageReads.slice(readsStart),longFrames,enabled,count:100,initial,final:state,frameMs:summary(frames),cpuCallMs:summary(cpu),gpuDrawMs:summary(gpu),over25ms:frames.filter(x=>x>25).length,gpuTimerSupported:!!ext,disjoint,uncollected,renderer:debug?gl.getParameter(debug.UNMASKED_RENDERER_WEBGL):gl.getParameter(gl.RENDERER),maxParticles,maxProjectiles,uploadsDuring:end.uploads-start.uploads,heapStart,heapEnd:MDModule.HEAPU8.buffer.byteLength,writes:state.writes};
  },enabled);
  assert.equal(metric.initial.actors.filter(a=>a.height>0).length,100);assert(metric.initial.actors.filter(a=>a.height>0).every(a=>a.weapons>0));assert(metric.final.sounds>0);assert(metric.maxProjectiles>0);assert.equal(metric.writes,0);result.cases.push(metric);await page.locator('#game').screenshot({path:`${out}/combat-${enabled}.png`});
 }
 assert.deepEqual(errors,[]);result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;}finally{result.errors=errors;await fs.writeFile(`${out}/result.json`,JSON.stringify(result,null,2));console.log(JSON.stringify({ok:result.ok,error:result.error,cases:result.cases.map(({initial,final,...m})=>m)}));const timer=setTimeout(()=>process.exit(process.exitCode||0),5000);await browser.close();clearTimeout(timer);}
