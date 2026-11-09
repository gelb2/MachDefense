import {chromium,webkit} from 'playwright';
import fs from 'node:fs/promises';import assert from 'node:assert/strict';
const artMode=Number(process.env.MD_ART_MODE||0);
const [out,url]=process.argv.slice(2),engine=process.env.MD_BROWSER||'chromium';await fs.mkdir(out,{recursive:true});
const browser=await(engine==='webkit'?webkit:chromium).launch({headless:true,...(engine==='chromium'?{executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}:{})});
const page=await browser.newPage({viewport:{width:640,height:800}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await page.evaluate(()=>MDTest.advance(1));
 await page.evaluate(artMode=>{window.artLoop=true;function draw(){if(!window.artLoop)return;MDTest.call('md_aircraft_scene',['number','number'],[8,artMode],'string');requestAnimationFrame(draw);}requestAnimationFrame(draw);},artMode);
 await page.waitForTimeout(150);await page.locator('#game').screenshot({path:out+'/body-comparison-'+engine+'.png'});
 await page.evaluate(()=>window.artLoop=false);assert.deepEqual(errors,[]);
 await fs.writeFile(out+'/visual-'+engine+'.json',JSON.stringify({ok:true,engine,errors,artMode,uniformScale:2,rows:['MachineGun10 body / light body','E_Tank_28 whole / gunship fixed body','E_Turret_18 whole / assembled gunship']}));
}finally{await browser.close();}
