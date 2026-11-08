import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';
import assert from 'node:assert/strict';
const [out,url]=process.argv.slice(2),engine=process.env.MD_BROWSER||'chromium';
await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:900,height:900}}),errors=[];
page.on('pageerror',error=>errors.push(error.message));
const result={engine,url,poses:[]};
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');
 await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);
 await page.evaluate(()=>MDTest.advance(1));
 const draw=mode=>page.evaluate(mode=>MDTest.call('md_aircraft_scene',['number','number'],[8,mode],'string'),mode);
 for(let tile=0;tile<7;++tile){
  await draw(10+tile);await page.locator('#game').screenshot({path:`${out}/pose-${tile}.png`});
  result.poses.push({tile,bankInputDegrees:[0,-8,-16,-24,8,16,24][tile]});
 }
 const start=await page.evaluate(()=>MDTest.state());
 for(let i=0;i<140;++i)await draw(10+i%7);
 const end=await page.evaluate(()=>MDTest.state());
 result.uploadsAfterWarmup=end.uploads-start.uploads;
 assert.equal(result.uploadsAfterWarmup,0);assert.deepEqual(errors,[]);result.ok=true;
}catch(error){result.error=error.stack;process.exitCode=1;}
finally{result.errors=errors;await fs.writeFile(`${out}/result.json`,JSON.stringify(result,null,2));console.log(JSON.stringify(result));await browser.close();}
