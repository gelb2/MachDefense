import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';
import assert from 'node:assert/strict';
const out=process.argv[2],url=process.argv[3],engine=process.env.MD_BROWSER||'chromium';await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:640,height:640}}),errors=[],result={engine,scenes:[]};page.on('pageerror',e=>errors.push(e.message));
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(1));
 for(const [mode,w,h] of [[0,640,640],[1,320,480],[2,768,1024]]){
  await page.setViewportSize({width:w,height:h});await page.evaluate(()=>new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve))));await page.evaluate(()=>MDTest.advance(1));
  const call=async command=>JSON.parse(await page.evaluate(([command,mode])=>MDTest.call('md_aircraft_scene',['number','number'],[command,mode],'string'),[command,mode]));
  const initial=await call(0);assert.equal(initial.actors.length,4);const before=await page.evaluate(()=>MDTest.state());
  await page.evaluate(mode=>{window.aircraftCapture=true;async function draw(){if(!window.aircraftCapture)return;await MDTest.call('md_aircraft_scene',['number','number'],[5,mode],'string');requestAnimationFrame(draw);}requestAnimationFrame(draw);},mode);
  await page.locator('#game').screenshot({path:`${out}/aircraft-${engine}-${mode}.png`});await page.evaluate(()=>window.aircraftCapture=false);
  await call(2);const frozen=await call(1);assert.equal(frozen.time,initial.time);assert.equal(frozen.actors[1].rotor,initial.actors[1].rotor);await call(3);
  if(mode===0&&engine==='chromium'){
   const encoded=await page.evaluate(async()=>{
    const stream=document.querySelector('#game').captureStream(60),rec=new MediaRecorder(stream,{mimeType:'video/webm;codecs=vp9',videoBitsPerSecond:1500000}),chunks=[];
    rec.ondataavailable=e=>chunks.push(e.data);const done=new Promise(resolve=>rec.onstop=resolve);rec.start();
    for(let i=0;i<240;++i){await new Promise(requestAnimationFrame);await MDTest.call('md_aircraft_scene',['number','number'],[1,0],'string');}
    rec.stop();await done;stream.getTracks().forEach(t=>t.stop());return await new Promise(resolve=>{let r=new FileReader();r.onload=()=>resolve(r.result.split(',')[1]);r.readAsDataURL(new Blob(chunks,{type:'video/webm'}));});
   });await fs.writeFile(`${out}/rotors.webm`,Buffer.from(encoded,'base64'));
  }
  await call(4);const moving=JSON.parse(await page.evaluate(async mode=>{let s;for(let i=0;i<240;++i)s=await MDTest.call('md_aircraft_scene',['number','number'],[1,mode],'string');return s;},mode));
  assert.notEqual(moving.actors[1].x,initial.actors[1].x);assert.equal(moving.writes,0);assert.equal(moving.actors.length,4);const after=await page.evaluate(()=>MDTest.state());assert.equal(after.uploads,before.uploads);assert.equal(after.writes,before.writes);
  await call(6);
  const combat=JSON.parse(await page.evaluate(async mode=>{let s;for(let i=0;i<600;++i)s=await MDTest.call('md_aircraft_scene',['number','number'],[1,mode],'string');return s;},mode));
  assert.ok(combat.actors[0].hp<100000&&combat.actors[2].hp<100000);assert.ok(combat.sounds>0);assert.equal(combat.writes,0);
  const warm=await page.evaluate(()=>MDTest.state());
  if(mode===0&&engine==='chromium'){
   const encoded=await page.evaluate(async()=>{
    const stream=document.querySelector('#game').captureStream(60),rec=new MediaRecorder(stream,{mimeType:'video/webm;codecs=vp9',videoBitsPerSecond:1800000}),chunks=[];
    rec.ondataavailable=e=>chunks.push(e.data);const done=new Promise(resolve=>rec.onstop=resolve);rec.start();
    for(let i=0;i<360;++i){await new Promise(requestAnimationFrame);await MDTest.call('md_aircraft_scene',['number','number'],[1,0],'string');}
    rec.stop();await done;stream.getTracks().forEach(t=>t.stop());return await new Promise(resolve=>{let r=new FileReader();r.onload=()=>resolve(r.result.split(',')[1]);r.readAsDataURL(new Blob(chunks,{type:'video/webm'}));});
   });await fs.writeFile(`${out}/weapons.webm`,Buffer.from(encoded,'base64'));
  }
  const end=await page.evaluate(()=>MDTest.state());assert.equal(end.uploads,warm.uploads);
  result.scenes.push({mode,initial,moving,combat,uploadsDuringMotion:after.uploads-before.uploads,uploadsDuringWarmCombat:end.uploads-warm.uploads,pause:true});
 }
 assert.deepEqual(errors,[]);result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;await page.screenshot({path:`${out}/failed-${engine}.png`});}finally{result.errors=errors;await fs.writeFile(`${out}/check-${engine}.json`,JSON.stringify(result,null,2));console.log(JSON.stringify(result));await browser.close();}
