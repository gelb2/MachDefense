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
  const name=['AH-10','Beluga','Cheyenne'][row],factor=[1.2,.85,.85][row],boxes=[];
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
 boxResult={baseline:'7e7fa8f',worldZoom:2,render:'Web cheat command 8 / mode 4; north; rotor angle 0; shadows excluded',method:'Bounds of actual non-background rendered pixels; PNG alpha 0 contributes no pixels',rasterTolerancePixels:2,rows,userAgent:navigator.userAgent};
 boxCanvas.hidden=false;boxOut.textContent=JSON.stringify(rows.map(r=>({name:r.name,before:r.before.width+' × '+r.before.height,after:r.after.width+' × '+r.after.height,target:r.factor,pass:r.pass})),null,2);
}catch(e){boxOut.textContent=String(e);}};
document.getElementById('exportboxes').onclick=()=>boxCanvas.toBlob(b=>downloadBlob(b,'machdefense-bounding-boxes-20261108.png'));
document.getElementById('exportstats').onclick=()=>downloadBlob(new Blob([JSON.stringify(boxResult,null,2)],{type:'application/json'}),'machdefense-bounding-boxes-20261108.json');
