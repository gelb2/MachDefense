import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const [out,url]=process.argv.slice(2),engine=process.env.MD_BROWSER||'chromium';await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:800,height:800}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
const result={engine,levels:[]};
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(1));
 const call=async(command,mode=3)=>JSON.parse(await page.evaluate(([c,m])=>MDTest.call('md_aircraft_scene',['number','number'],[c,m],'string'),[command,mode]));
 for(const level of [1,10]){
  await call(20,level);let state;for(let i=0;i<600;++i)state=await call(1);
  const stopped=state.actors.filter(a=>a.height>0);assert.equal(stopped.length,4);assert(stopped.every(a=>a.speed===0));
  await page.locator('#game').screenshot({path:`${out}/range-level-${level}.png`});
  for(let i=0;i<120;++i)state=await call(1);
  const after=state.actors.filter(a=>a.height>0);
  for(let i=0;i<4;++i){assert.equal(after[i].x,stopped[i].x);assert.equal(after[i].y,stopped[i].y);assert.equal(after[i].speed,0);}
  assert(state.sounds>0&&state.writes===0);assert(state.actors.filter(a=>a.height===0).every(a=>a.hp<10000000));
  result.levels.push({level,stopped,after,sounds:state.sounds,writes:state.writes});
 }
 assert.deepEqual(errors,[]);result.ok=true;
}catch(error){result.error=error.stack;process.exitCode=1;}
finally{result.errors=errors;await fs.writeFile(`${out}/result.json`,JSON.stringify(result,null,2));console.log(JSON.stringify({ok:result.ok,error:result.error,engine}));await browser.close();}
