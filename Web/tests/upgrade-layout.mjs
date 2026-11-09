import {chromium, webkit} from 'playwright';
import fs from 'node:fs/promises';
import assert from 'node:assert/strict';

const [out, url] = process.argv.slice(2), engine = process.env.MD_BROWSER || 'chromium';
await fs.mkdir(out, {recursive:true});
const browser = await (engine === 'webkit' ? webkit : chromium).launch({headless:true,
  ...(engine === 'chromium' ? {executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'} : {})});
const page = await browser.newPage({viewport:{width:393,height:852}});
const result = {engine,url,sizes:[]}, errors = [];
page.on('pageerror', e => errors.push(e.message));
const advance = n => page.evaluate(n => MDTest.advance(n), n);
const state = () => page.evaluate(() => MDTest.state());
const sprites = () => page.evaluate(() => MDTest.sprites());
const command = n => page.evaluate(n => MDTest.call('md_command',['number'],[n]), n);
async function point(x,y) {
  const s=await state(), b=await page.locator('#game').boundingBox();
  return {x:b.x+x/s.width*b.width,y:b.y+y/s.height*b.height};
}
async function tap(x,y) { const p=await point(x,y);await page.mouse.click(p.x,p.y);await advance(1); }
async function press(name) {
  const b=(await sprites()).find(s=>s.image===name);assert(b,'Missing '+name);
  await tap(b.x+b.w/2,b.y+b.h/2);
}
// Read the disposable test save using the original Darwin random()/save layout.
// No user save is imported or changed by this test.
async function inventory() {
  // Upgrade actions intentionally remain in memory until a save boundary.
  // Flush only this test profile through the existing validation save command.
  await command(500);
  const b=Buffer.from(await page.evaluate(()=>Array.from(MDStore.current)));
  assert.equal(b.length,42496);
  const rng=new Uint32Array(31);rng[0]=b.readUInt32LE(0);let front=3,rear=0;
  for(let i=1;i<31;i++){let v=rng[i-1]|0;if(v===0)v=123459876;v=16807*(v%127773)-2836*Math.trunc(v/127773);if(v<0)v+=2147483647;rng[i]=v;}
  const next=()=>{rng[front]=(rng[front]+rng[rear])>>>0;const v=rng[front]>>>1;front=(front+1)%31;rear=(rear+1)%31;return v;};
  for(let i=0;i<310;i++)next();for(let i=4;i<b.length-4;i++)b[i]^=next()&255;
  let sum=0;for(let i=4;i<b.length-4;i++)sum+=b.readInt8(i);assert.equal(sum,b.readInt32LE(b.length-4));
  const slot=0x408, count=b.readUInt32LE(slot+0xb44), units=[];
  for(let i=0;i<count;i++){const at=slot+0xb48+i*0x28;units.push({name:b.subarray(at+1,at+33).toString().split('\0')[0],level:b.readUInt32LE(at+0x24),installed:!!(b[at]&1)});}
  return {units,credits:b.readUInt32LE(slot+0x3c)};
}
function uniqueCards(ss) {
  return [...new Map(ss.filter(s=>s.image==='Btn_MachBuild.png').map(s=>[`${s.x},${s.y}`,s])).values()];
}
async function wheel(delta) {
  const title=(await sprites()).find(s=>s.image==='CommonUI_Upgrade.png');
  const p=await point(title.x+100,title.y+70);await page.mouse.move(p.x,p.y);
  await page.mouse.wheel(0,delta);await page.waitForTimeout(120);await advance(1);
}
try {
  await page.goto(url);await page.waitForSelector('#play:not([disabled])',{timeout:120000});
  await page.click('#play');await page.waitForFunction(()=>window.MDModule&&document.querySelector('#launcher').hidden);await advance(60);
  assert.equal((await state()).cheat,true);
  await command(3);await command(9);for(let i=0;i<20;i++)await command(8);
  for(let i=0;i<18;i++){await command(204);await command(201);}await command(500);
  const initial=await inventory();assert.equal(initial.units.length,18);
  await command(6);await advance(240);let s=await state();await tap(s.width/2,s.height-100);await advance(180);
  s=await state();await tap(s.width-26,s.height-77);await advance(5);
  const fullCards=uniqueCards(await sprites()),detail=(await sprites()).find(s=>s.image==='UpgradeEquip_MachInfo.png');
  await page.screenshot({path:`${out}/initial-${engine}.png`});
  assert.equal(fullCards.length,18,'Tall screen must show every catalog entry');
  assert(Math.max(...fullCards.map(s=>s.y+s.h))<=detail.y,'Last unit row overlaps detail panel');
  // Click every actual catalog card and verify the selected row and persisted action target.
  for(let i=0;i<18;i++){
    const c=fullCards[i];await tap(c.x+c.w/2,c.y+c.h/2);
    const marker=(await sprites()).find(s=>s.image==='UpgradeEquip_SelMach.png');assert(marker,'No selection '+i);
    assert(Math.abs(marker.x+marker.w/2-(c.x+c.w/2))<1&&Math.abs(marker.y+marker.h/2-(c.y+c.h/2))<1,'Wrong selection '+i);
    const before=await inventory();await press('UpgradeBtn_Upgrade.png');const after=await inventory();
    assert.equal(after.units[i].level,before.units[i].level+1,'Upgrade target '+before.units[i].name);
    assert(after.credits<before.credits);assert.deepEqual(after.units.filter((_,j)=>j!==i),before.units.filter((_,j)=>j!==i));
    await press(after.units[i].installed?'UpgradeBtn_Uninstall.png':'UpgradeBtn_Install.png');
    assert.equal((await inventory()).units[i].installed,!after.units[i].installed);
    await press(after.units[i].installed?'UpgradeBtn_Install.png':'UpgradeBtn_Uninstall.png');
    assert.equal((await inventory()).units[i].installed,after.units[i].installed);
  }
  result.selectedAndUpgraded=initial.units.map(u=>u.name);result.installUninstall=true;
  // Last row must remain accessible on short and landscape viewports; drag must not buy anything.
  for(const size of [{width:393,height:852},{width:320,height:480},{width:1200,height:800},{width:820,height:1180}]){
    await page.setViewportSize(size);await page.evaluate(()=>new Promise(r=>requestAnimationFrame(()=>requestAnimationFrame(r))));await advance(2);
    await wheel(5000);let ss=await sprites();const cards=uniqueCards(ss),last=cards.at(-1),panel=ss.find(s=>s.image==='UpgradeEquip_MachInfo.png');
    assert(last.y+last.h<=panel.y,'Scrolled last row overlaps detail');
    await tap(last.x+last.w/2,last.y+last.h/2);assert((await sprites()).some(s=>s.image==='UpgradeEquip_SelMach.png'));
    const before=await inventory();await press('UpgradeBtn_Upgrade.png');assert.equal((await inventory()).units.at(-1).level,before.units.at(-1).level+1);
    const saved=await inventory(), writes=(await state()).writes;
    const a=await point(last.x+last.w/2,last.y+last.h/2),b=await point(last.x+last.w/2,last.y-100);
    await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:10});await page.mouse.up();await advance(1);
    assert.equal((await state()).writes,writes);assert.deepEqual(await inventory(),saved);
    await page.screenshot({path:`${out}/${size.width}x${size.height}-${engine}.png`});
    await press('Common_Btn_CLOSE.png');assert(!(await sprites()).some(s=>s.image==='CommonUI_Upgrade.png'));
    s=await state();await tap(s.width-26,s.height-77);await advance(2);
    result.sizes.push({...size,lastRowSelectable:true,dragDoesNotPurchase:true,closeWorks:true});
  }
  // Repeated upgrades stop at level 10 without charging again.
  await wheel(5000);const last=uniqueCards(await sprites()).at(-1);await tap(last.x+last.w/2,last.y+last.h/2);
  for(let i=0;i<12;i++)await press('UpgradeBtn_Upgrade.png');
  const capped=await inventory();assert.equal(capped.units.at(-1).level,9);await press('UpgradeBtn_Upgrade.png');assert.deepEqual(await inventory(),capped);result.level10Cap=true;
  assert.deepEqual(errors,[]);result.ok=true;
} catch(e) {result.error=e.stack;process.exitCode=1;await page.screenshot({path:`${out}/failure-${engine}.png`});}
finally {result.errors=errors;await fs.writeFile(`${out}/result-${engine}.json`,JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));await browser.close();}
