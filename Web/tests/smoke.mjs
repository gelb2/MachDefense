import puppeteer from 'puppeteer-core';
import fs from 'node:fs/promises';
const out=process.argv[2]||'.local/pwa-20261106';
await fs.mkdir(out,{recursive:true});
const browser=await puppeteer.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--autoplay-policy=no-user-gesture-required'],userDataDir:out+'/chrome-'+Date.now()});
const page=await browser.newPage();await page.setViewport({width:390,height:844,deviceScaleFactor:1});
page.on('pageerror',e=>console.log('PAGEERROR',e.message));page.on('console',m=>{if(m.type()==='error')console.log('CONSOLE',m.text());});
try{
 await page.goto('http://127.0.0.1:8940/',{waitUntil:'networkidle0',timeout:120000});await page.waitForFunction(()=>!document.querySelector('#play').disabled,{timeout:120000});
 await page.screenshot({path:out+'/launcher.png'});await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden,{timeout:120000});
 await page.evaluate(()=>MDTest.advance(60));console.log('START',await page.evaluate(()=>MDTest.state()));await page.screenshot({path:out+'/title.png'});
 await page.evaluate(async()=>{await MDTest.tap(.5,.5);await MDTest.advance(180);});await page.screenshot({path:out+'/slots.png'});console.log('SLOTS',await page.evaluate(()=>MDTest.state()));
 // Original NEW profile button is anchored to bottom of portrait viewport.
 const s=await page.evaluate(()=>MDTest.state());const y=(s.height-30)/s.height,x=(s.width/2+80)/s.width;
 await page.evaluate(({x,y})=>{MDTest.tap(x,y);},{x,y});await page.waitForSelector('#textDialog[open]');await page.type('#textInput','WEB TEST');await page.click('button[value="ok"]');await page.evaluate(()=>MDTest.advance(20));
 console.log('NEW SAVE',await page.evaluate(()=>MDTest.save()));await page.screenshot({path:out+'/profile.png'});
 const centerY=(s.height-480)/2;await page.evaluate(async({s,centerY})=>{await MDTest.tap(.5,(140+centerY)/s.height);await MDTest.advance(180);await MDTest.tap((s.width/2+80)/s.width,(s.height-30)/s.height);await MDTest.advance(180);await MDTest.tap(.5,(s.height-30)/s.height);await MDTest.advance(60);},{s,centerY});
 await page.screenshot({path:out+'/battle.png'});console.log('BATTLE',await page.evaluate(()=>MDTest.state()));
 await page.evaluate(async()=>{await MDTest.call('md_command',['number'],[12]);await MDTest.advance(1);});
 // Production icon uses original top-right HUD coordinates.
 await page.evaluate(async({s})=>{await MDTest.tap((s.width-28)/s.width,52/s.height);await MDTest.advance(300);},{s});console.log('PRODUCED',await page.evaluate(()=>MDTest.state()));
 await page.screenshot({path:out+'/produced.png'});
 await fs.writeFile(out+'/browser-first.json',JSON.stringify(await page.evaluate(()=>MDTest.state()),null,2));
}catch(e){console.log('FAIL',e.stack);await page.screenshot({path:out+'/failure.png'});process.exitCode=1;}finally{await browser.close();}
