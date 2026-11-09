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
  const hpC=[850,936,1062,1190,1360,1530,1700,1956,2210,2550],hpB=[1500,1750,2026,2300,2700,3300,3800,4450,5200,6100];
  const cellsC=[216,258,300,342,390,450,510,582,660,750],cellsB=[378,444,510,576,645,720,795,876,960,1050];
  const upC=[420,900,1800,3600,7200,14400,28800,57600,115200,0],upB=[660,1350,2700,5400,10800,21600,43200,86400,172800,0];
  const source=await page.evaluate(()=>Object.fromEntries(['MachList','MachBuild_Name'].map(n=>[n,new TextDecoder().decode(MDModule.FS.readFile('/assets/Tables/'+n+'.tsv'))])));
  const table=s=>new Map(s.trim().split(/\r?\n/).map(l=>{const r=l.split('\t').map(c=>c.trim());return [r[0],r];}));
  const mach=table(source.MachList),build=table(source.MachBuild_Name);result.runtimeLevels=[];
  for(let level=1;level<=10;level++){
    const suffix='_'+String(level).padStart(2,'0');
    const scene=JSON.parse(await page.evaluate(level=>MDTest.call('md_aircraft_scene',['number','number'],[20,level],'string'),level));
    const chey=scene.actors.find(a=>a.name==='AircraftDuct'+suffix),beluga=scene.actors.find(a=>a.name==='AircraftTransport'+suffix);
    assert.equal(chey.hp,hpC[level-1]);assert.equal(beluga.hp,hpB[level-1]);assert.equal(scene.writes,0);
    for(const [name,hp,cells,upgrade] of [['Orca',hpC[level-1],cellsC[level-1],upC[level-1]],['OrcaBomber',hpB[level-1],cellsB[level-1],upB[level-1]]]){
      const r=build.get(name+suffix);assert.equal(Number(r[1]),cells);assert.equal(Number(r[2]),upgrade);assert.equal(Number(mach.get(r[7])[1])*2,hp);
    }
    result.runtimeLevels.push({level,cheyenneHP:chey.hp,belugaHP:beluga.hp,cheyenneCells:cellsC[level-1],belugaCells:cellsB[level-1]});
  }
  await command(3);await command(9);for(let i=0;i<20;i++)await command(8);
  for(let i=0;i<18;i++){await command(204);await command(201);}await command(500);
  await command(6);await advance(240);let s=await state();await tap(s.width/2,s.height-100);await advance(180);
  s=await state();await tap(s.width-26,s.height-77);await advance(5);
  const cards=uniqueCards(await sprites());assert.equal(cards.length,18);result.upgrades=[];
  for(const [name,costs] of [['Orca',upC],['OrcaBomber',upB]]){
    const inv=await inventory(),index=inv.units.findIndex(u=>u.name===name);assert(index>=0);
    const card=cards[index];await tap(card.x+card.w/2,card.y+card.h/2);
    for(let level=1;level<=10;level++){
      const before=await inventory();assert.equal(before.units[index].level,level-1);
      if([1,7,10].includes(level))await page.screenshot({path:`${out}/${name}-level-${level}-${engine}.png`});
      await press('UpgradeBtn_Upgrade.png');const after=await inventory();
      assert.equal(before.credits-after.credits,costs[level-1]);
      assert.equal(after.units[index].level,Math.min(level,9));
      assert.deepEqual(after.units.filter((_,j)=>j!==index),before.units.filter((_,j)=>j!==index));
      result.upgrades.push({name,fromLevel:level,charged:before.credits-after.credits,nextLevel:after.units[index].level+1});
    }
  }
  const before=await state();await advance(180);assert.equal((await state()).writes,before.writes);
  result.noFrameSaveWrites=true;result.level10Cap=true;assert.deepEqual(errors,[]);result.ok=true;
} catch(e) {result.error=e.stack;process.exitCode=1;await page.screenshot({path:`${out}/failure-${engine}.png`});}
finally {result.errors=errors;await fs.writeFile(`${out}/result-${engine}.json`,JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));await browser.close();}
