import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';
import assert from 'node:assert/strict';
const url=process.argv[2]||'http://127.0.0.1:8953/',out=process.argv[3]||'.local/title-audit';
await fs.mkdir(out,{recursive:true});
const engine=process.env.MD_BROWSER||'chromium';
const browser=await (engine==='webkit'?webkit.launch({headless:true}):chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true}));
const results=[];
try {
for(const [name,w,h] of [['reference',320,480],['iphone',390,844],['ipad',820,1180],['desktop',1280,800],['wide',1600,900],['ultrawide',3440,1440]]){
 const context=await browser.newContext({viewport:{width:w,height:h}}),page=await context.newPage(),errors=[];
 page.on('pageerror',e=>errors.push(e.message));
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});
 await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden,undefined,{timeout:120000});
 await page.evaluate(()=>MDTest.advance(20));
 const state=await page.evaluate(()=>MDTest.state()),sprites=await page.evaluate(()=>MDTest.sprites());
 const titles=sprites.filter(x=>x.image==='Title.jpg');assert.equal(titles.length,name==='reference'?1:9);
 const bounds={x:Math.min(...titles.map(x=>x.x)),y:Math.min(...titles.map(x=>x.y)),right:Math.max(...titles.map(x=>x.x+x.w)),bottom:Math.max(...titles.map(x=>x.y+x.h))};assert(Math.abs(bounds.right-bounds.x-320)<.001);assert(Math.abs(bounds.bottom-bounds.y-480)<.001);assert.equal(state.writes,0);
 const surrounds=sprites.filter(x=>x.image.startsWith('Generated/Title/'));
 assert.equal(surrounds.length,name==='reference'?0:2);
 assert(surrounds.every(x=>x.image===(w/h>2/3?'Generated/Title/Wide.png':'Generated/Title/Tall.png')));
 await page.screenshot({path:`${out}/${name}-title.png`});
 let seam=null;
 if(name==='desktop'||name==='iphone'){
  const pts=name==='desktop'?[[905,20],[906,20],[907,20]]:[[10,713],[10,714],[10,715]];
  const png=await page.screenshot();
  const rgb=await page.evaluate(async({data,pts})=>{const im=new Image;im.src=data;await im.decode();const c=document.createElement('canvas');c.width=im.width;c.height=im.height;const ctx=c.getContext('2d');ctx.drawImage(im,0,0);return pts.map(([x,y])=>Array.from(ctx.getImageData(x,y,1,1).data).slice(0,3));},{data:'data:image/png;base64,'+png.toString('base64'),pts});
  const maxDifference=Math.max(...rgb[1].map((v,i)=>Math.abs(v-(rgb[0][i]+rgb[2][i])/2)));
  assert(maxDifference<8,`Dark title seam: ${name} ${JSON.stringify(rgb)}`);seam={points:pts,rgb,maxDifference};
 }
 await page.evaluate(async()=>{await MDTest.tap(.5,.5);await MDTest.advance(160);});
 assert((await page.evaluate(()=>MDTest.sprites())).some(x=>x.image==='Common_Btn_NEW.png'));
 assert.equal((await page.evaluate(()=>MDTest.state())).writes,0);assert.deepEqual(errors,[]);
 await page.screenshot({path:`${out}/${name}-slots.png`});
 results.push({name,w,h,originalTitleCount:titles.length,originalSize:[bounds.right-bounds.x,bounds.bottom-bounds.y],surroundings:surrounds.map(x=>x.image),seam,slotsOpened:true,writes:0,errors});
 await context.close();
}
}finally{await browser.close();}
await fs.writeFile(`${out}/title-layout.json`,JSON.stringify(results,null,2));console.log(JSON.stringify(results));
