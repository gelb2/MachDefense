let boxResult=null;
const boxOut=document.getElementById('boxresult'),boxCanvas=document.getElementById('boxcanvas');
function downloadBlob(blob,name){const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(a.href),5000);}
document.getElementById('boxes').onclick=async()=>{try{
 if(!window.MDModule)throw Error('먼저 게임을 시작하세요.');
 await MDTest.advance(1);const game=document.getElementById('game');game.width=900;game.height=900;
 await MDTest.call('md_resize',['number','number','number','number','number','number'],[900,900,900,900,0,0]);
 await MDTest.call('md_aircraft_scene',['number','number'],[8,4],'string');
 const ctx=boxCanvas.getContext('2d');ctx.drawImage(game,0,0);const pixels=ctx.getImageData(0,0,900,900).data;
 const background=Array.from(pixels.slice(0,3)),rows=[];
 for(let row=0;row<3;row++){
  const name=['AH-10','Beluga','Cheyenne'][row],factor=[1.1,.9,.9][row],boxes=[];
  for(let column=0;column<2;column++){
   let left=900,top=900,right=-1,bottom=-1;
   for(let y=row*300;y<(row+1)*300;y++)for(let x=column*450;x<(column+1)*450;x++){
    const i=(y*900+x)*4;
    if(pixels[i]!==background[0]||pixels[i+1]!==background[1]||pixels[i+2]!==background[2]){left=Math.min(left,x);right=Math.max(right,x);top=Math.min(top,y);bottom=Math.max(bottom,y);}
   }
   if(right<0)throw Error(name+' has no rendered pixels');
   const box={left,top,right:right+1,bottom:bottom+1,width:right-left+1,height:bottom-top+1};boxes.push(box);
   ctx.strokeStyle=column?'#6effa3':'#ffcf68';ctx.lineWidth=1;ctx.strokeRect(left-.5,top-.5,box.width+1,box.height+1);
   ctx.fillStyle=ctx.strokeStyle;ctx.font='14px monospace';ctx.textAlign='center';ctx.fillText(box.width+' x '+box.height,left+box.width/2,Math.min((row+1)*300-7,bottom+18));
  }
  const widthError=boxes[1].width-boxes[0].width*factor,heightError=boxes[1].height-boxes[0].height*factor;
  rows.push({name,factor,before:boxes[0],after:boxes[1],widthRatio:boxes[1].width/boxes[0].width,heightRatio:boxes[1].height/boxes[0].height,widthErrorPixels:widthError,heightErrorPixels:heightError,pass:Math.abs(widthError)<=2&&Math.abs(heightError)<=2});
  ctx.fillStyle='#ffffff';ctx.font='15px sans-serif';ctx.textAlign='center';ctx.fillText(name+' BEFORE',225,row*300+18);ctx.fillText(name+' '+(factor*100).toFixed(0)+'%',675,row*300+18);
 }
 boxResult={baseline:'b01d751',worldZoom:2,render:'Web cheat command 8 / mode 4; north; rotor angle 0; shadows excluded',method:'Bounds of actual non-background rendered pixels; PNG alpha 0 contributes no pixels',rasterTolerancePixels:2,rows,userAgent:navigator.userAgent};
 boxCanvas.hidden=false;boxOut.textContent=JSON.stringify(rows.map(r=>({name:r.name,before:r.before.width+' × '+r.before.height,after:r.after.width+' × '+r.after.height,target:r.factor,pass:r.pass})),null,2);
}catch(e){boxOut.textContent=String(e);}};
document.getElementById('exportboxes').onclick=()=>boxCanvas.toBlob(b=>downloadBlob(b,'machdefense-scale2-20261108.png'));
document.getElementById('exportstats').onclick=()=>downloadBlob(new Blob([JSON.stringify(boxResult,null,2)],{type:'application/json'}),'machdefense-scale2-20261108.json');

document.getElementById('refs').onclick=async()=>{try{
 await MDTest.advance(1);const game=document.getElementById('game');game.width=900;game.height=900;
 await MDTest.call('md_resize',['number','number','number','number','number','number'],[900,900,900,900,0,0]);
 await MDTest.call('md_aircraft_scene',['number','number'],[8,5],'string');
 const ctx=boxCanvas.getContext('2d');ctx.drawImage(game,0,0);const pixels=ctx.getImageData(0,0,900,900).data,background=Array.from(pixels.slice(0,3));const cells=[];
 for(let row=0;row<3;row++)for(let column=0;column<3;column++){
 let left=900,top=900,right=-1,bottom=-1;
 for(let y=row*300;y<(row+1)*300;y++)for(let x=column*300;x<(column+1)*300;x++){
 const i=(y*900+x)*4;if(pixels[i]!==background[0]||pixels[i+1]!==background[1]||pixels[i+2]!==background[2]){left=Math.min(left,x);right=Math.max(right,x);top=Math.min(top,y);bottom=Math.max(bottom,y);}}
 if(right<0)throw Error('Missing reference');const width=right-left+1,height=bottom-top+1;
 const name=column===0?'E_Tank_32':column===1?'MachineGun Lv.10':['AH-10','Beluga','Cheyenne'][row];
 cells.push({row,column,name,left,top,width,height});ctx.strokeStyle=column===2?'#6effa3':'#ffcf68';ctx.strokeRect(left-.5,top-.5,width+1,height+1);
 ctx.fillStyle=ctx.strokeStyle;ctx.font='14px monospace';ctx.textAlign='center';ctx.fillText(width+' x '+height,column*300+150,Math.min(row*300+287,bottom+20));ctx.fillStyle='white';ctx.font='15px sans-serif';ctx.fillText(name,column*300+150,row*300+18);
 }
 boxResult={worldZoom:2,rotorAngle:0,shadows:false,cells,userAgent:navigator.userAgent};boxCanvas.hidden=false;boxOut.textContent=JSON.stringify(cells.map(c=>({name:c.name,width:c.width,height:c.height})),null,2);
 document.getElementById('exportboxes').onclick=()=>boxCanvas.toBlob(b=>downloadBlob(b,'machdefense-reference-scale2-20261108.png'));
 document.getElementById('exportstats').onclick=()=>downloadBlob(new Blob([JSON.stringify(boxResult,null,2)],{type:'application/json'}),'machdefense-reference-scale2-20261108.json');
 }catch(e){boxOut.textContent=String(e);}};

let reviewData={}; const output=document.getElementById("reviewresult");
const scene=async(c,m=3)=>JSON.parse(await MDTest.call('md_aircraft_scene',['number','number'],[c,m],'string'));
async function ready(){if(!window.MDModule)throw Error('먼저 PLAY IN BROWSER를 누르세요.');boxCanvas.hidden=true;await MDTest.advance(1);const c=document.getElementById('game');c.width=900;c.height=900;await MDTest.call('md_resize',['number','number','number','number','number','number'],[900,900,900,900,0,0]);}
async function compare(){await MDTest.call('md_aircraft_scene',['number','number'],[8,3],'string');}
function run(id,fn){document.getElementById(id).onclick=async()=>{try{await fn();}catch(e){output.textContent=String(e);}};}
function download(blob,name){const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(a.href),5000);}
run('check',async()=>{await ready();output.textContent='검사 실행 중';const initial=await scene(0),before=await MDTest.state();await scene(2);const frozen=await scene(1);await scene(3);await scene(4);let moving;for(let i=0;i<240;i++)moving=await scene(1);await scene(6);let combat;for(let i=0;i<600;i++)combat=await scene(1);for(let i=0;i<1200;i++)await scene(1);const warm=await MDTest.state();for(let i=0;i<240;i++)await scene(1);const end=await MDTest.state();const checks={pause:frozen.time===initial.time&&frozen.actors[1].rotor===initial.actors[1].rotor,movement:moving.actors[1].x!==initial.actors[1].x,actors:moving.actors.length===8,damage:combat.actors[0].hp<100000&&combat.actors[2].hp<100000,sound:combat.sounds>0,noSceneWrites:combat.writes===0,noSaveWrites:end.writes===before.writes,noWarmUploads:end.uploads===warm.uploads};reviewData={checks,initial,moving,combat,before,warm,end,userAgent:navigator.userAgent};output.textContent=JSON.stringify(checks,null,2);});

run('savedata',async()=>download(new Blob([JSON.stringify(reviewData,null,2)],{type:'application/json'}),'machdefense-scale2-combat-20261108.json'));