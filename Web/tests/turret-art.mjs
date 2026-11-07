import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';
import assert from 'node:assert/strict';
const out=process.argv[2],url=process.argv[3],engine=process.env.MD_BROWSER||'chromium';
await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:800,height:640}}),errors=[],result={engine,frames:[]};
page.on('pageerror',e=>errors.push(e.message));
try {
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');
 await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);
 await page.evaluate(()=>MDTest.advance(60));
 const before=await page.evaluate(()=>MDTest.state());assert(before.cheat);
 for(const [family,name] of ['rifle','missile','vulcan','ion','laser','plasma'].entries()){
  const check=await page.evaluate(family=>MDTest.call('md_art_frame',['number'],[family],'string'),family);
  assert(check.startsWith('Art: 24 poses;'),check);
  await page.locator('#game').screenshot({path:`${out}/${name}-${engine}.png`});
  result.frames.push({family:name,check});
 }
 assert.equal((await page.evaluate(()=>MDTest.state())).writes,before.writes);
 assert.deepEqual(errors,[]);result.poses=144;result.noSaveWrites=true;result.ok=true;
}catch(e){result.error=e.stack;process.exitCode=1;await page.screenshot({path:`${out}/art-failed-${engine}.png`});}
finally{result.errors=errors;await fs.writeFile(`${out}/turret-art-${engine}.json`,JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));await browser.close();}
