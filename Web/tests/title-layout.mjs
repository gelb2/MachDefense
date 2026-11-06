import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';
import assert from 'node:assert/strict';
const url=process.argv[2]||'http://127.0.0.1:8953/',out=process.argv[3]||'.local/title-audit';
await fs.mkdir(out,{recursive:true});
const engine=process.env.MD_BROWSER||'chromium';
const browser=await (engine==='webkit'?webkit.launch({headless:true}):chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true}));
const results=[];
try {
for(const [name,w,h] of [['reference',320,480],['iphone',390,844],['ipad',820,1180],['desktop',1280,800],['wide',1600,900]]){
 const context=await browser.newContext({viewport:{width:w,height:h}}),page=await context.newPage(),errors=[];
 page.on('pageerror',e=>errors.push(e.message));
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});
 await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden,{timeout:120000});
 await page.evaluate(()=>MDTest.advance(20));
 const state=await page.evaluate(()=>MDTest.state()),sprites=await page.evaluate(()=>MDTest.sprites());
 const titles=sprites.filter(x=>x.image==='Title.jpg');assert.equal(titles.length,1);
 assert.equal(titles[0].w,320);assert.equal(titles[0].h,480);assert.equal(state.writes,0);
 assert.equal(sprites.some(x=>x.image.startsWith('Generated/Title/')),name!=='reference');
 await page.screenshot({path:`${out}/${name}-title.png`});
 await page.evaluate(async()=>{await MDTest.tap(.5,.5);await MDTest.advance(160);});
 assert((await page.evaluate(()=>MDTest.sprites())).some(x=>x.image==='Common_Btn_NEW.png'));
 assert.equal((await page.evaluate(()=>MDTest.state())).writes,0);assert.deepEqual(errors,[]);
 await page.screenshot({path:`${out}/${name}-slots.png`});
 results.push({name,w,h,originalTitleCount:titles.length,originalSize:[titles[0].w,titles[0].h],slotsOpened:true,writes:0,errors});
 await context.close();
}
}finally{await browser.close();}
await fs.writeFile(`${out}/title-layout.json`,JSON.stringify(results,null,2));console.log(JSON.stringify(results));
