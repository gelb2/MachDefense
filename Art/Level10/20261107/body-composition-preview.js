(async()=>{
const root='md-body-concepts-20261107/';
const manifest=await(await fetch(root+'composition.json')).json();
const load=src=>new Promise((resolve,reject)=>{const i=new Image;i.onload=()=>resolve(i);i.onerror=()=>reject(Error(src));i.src=src});
const canvas=(w,h)=>Object.assign(document.createElement('canvas'),{width:w,height:h});
function bounds(im,x=0,y=0,w=im.width,h=im.height){const c=canvas(w,h),g=c.getContext('2d',{willReadFrequently:true});g.drawImage(im,x,y,w,h,0,0,w,h);const a=g.getImageData(0,0,w,h).data;let l=w,t=h,r=-1,b=-1;for(let yy=0;yy<h;yy++)for(let xx=0;xx<w;xx++)if(a[4*(yy*w+xx)+3]>32){l=Math.min(l,xx);t=Math.min(t,yy);r=Math.max(r,xx);b=Math.max(b,yy)}return[x+l,y+t,r-l+1,b-t+1]}
const calibration=[];
function fit(im,box,pivot,w,h,target,limit,key){
 const referenceLength=limit[2]-limit[0],scale=referenceLength/box[2];
 if(!Number.isFinite(scale)||scale<=0)throw Error('invalid scale '+key);
 const c=canvas(w,h),g=c.getContext('2d');g.imageSmoothingEnabled=false;
 const dest=[target[0]+(box[0]-pivot[0])*scale,target[1]+(box[1]-pivot[1])*scale,box[2]*scale,box[3]*scale];
 if(dest[0]<0||dest[1]<0||dest[0]+dest[2]>w||dest[1]+dest[3]>h)throw Error('clipped part '+key);
 g.drawImage(im,...box,...dest);
 calibration.push({key,scale,dest,limit,pivot,target,referenceSize:[referenceLength,limit[3]-limit[1]],aspectPreserved:true});return c
}
function cell(im,index){const x=Math.round(index*im.width/5),end=Math.round((index+1)*im.width/5);return bounds(im,x,0,end-x,im.height)}
function landmark(box,fraction){return[box[0]+box[2]*fraction[0],box[1]+box[3]*fraction[1]]}
const originalBody=await load('md-turret-scale-20261107/Turret_b_05.png');const bodies={5:originalBody};
const bodySheet=await load(root+manifest.body_sheet);
for(let level=6;level<=10;level++){const box=cell(bodySheet,level-6);bodies[level]=fit(bodySheet,box,landmark(box,manifest.body_ring_fraction),64,64,manifest.body_target_anchor,manifest.body_visible_limit,'body-'+level)}
const families={};
for(const f of manifest.families){const p=f.original,im=await load('md-turret-scale-20261107/'+p.name+'.png'),sheet=await load(root+f.sheet);const anchor=[p.png_size[0]/2+Math.trunc(p.sockets[0][0]),p.png_size[1]/2-Math.trunc(p.sockets[0][1])];const weapons={5:im};for(let l=6;l<=10;l++){const box=cell(sheet,l-6);weapons[l]=fit(sheet,box,landmark(box,f.source_ring_fraction),...p.png_size,anchor,p.visible_bbox,f.id+'-'+l)}families[f.id]={...f,anchor,weapons}}
function drawPart(g,im,anchor,angle,shadow=false){g.save();if(shadow){g.globalAlpha=.6;g.translate(4,4)}g.rotate(-angle);g.drawImage(im,-anchor[0],-anchor[1]);g.restore()}
const shadows=new WeakMap();function shadowOf(im){if(shadows.has(im))return shadows.get(im);const s=canvas(im.width,im.height),g=s.getContext('2d');g.drawImage(im,0,0);g.globalCompositeOperation='source-in';g.fillStyle='#000';g.fillRect(0,0,s.width,s.height);shadows.set(im,s);return s}
function draw(target,family,level,mode,delta){const g=target.getContext('2d');g.clearRect(0,0,target.width,target.height);g.imageSmoothingEnabled=false;g.save();g.translate(80,82);const body=bodies[level],f=families[family],weapon=f.weapons[level===5||mode==='original'?5:level],bodyAngle=Math.PI/2,gunAngle=bodyAngle+delta*Math.PI/180;drawPart(g,shadowOf(body),manifest.body_target_anchor,bodyAngle,true);if(mode!=='body')drawPart(g,shadowOf(weapon),f.anchor,gunAngle,true);drawPart(g,body,manifest.body_target_anchor,bodyAngle);if(mode!=='body')drawPart(g,weapon,f.anchor,gunAngle);g.restore()}
const bodyContainer=document.querySelector('#md-body-only');for(let l=5;l<=10;l++){const fig=document.createElement('figure'),link=document.createElement('a'),small=canvas(64,64),large=canvas(64,64),cap=document.createElement('figcaption');for(const c of [small,large])c.getContext('2d').drawImage(bodies[l],0,0);small.style='width:64px;height:64px;display:block';large.className='md-body-detail';link.href=l===5?'md-turret-scale-20261107/Turret_b_05.png':root+manifest.body_sheet;link.append(small,large);cap.textContent=(l===5?'원작 ':'시안 ')+'Lv.'+l;const dims=l===5?[53,60]:calibration.find(v=>v.key==='body-'+l).dest.slice(2);cap.textContent+=' · '+dims.map(n=>n.toFixed(1)).join(' × ');fig.append(link,cap);bodyContainer.append(fig)}
const grid=document.querySelector('#md-compositions'),cards=[];for(const f of manifest.families){const title=document.createElement('h4');title.textContent=f.name;const row=document.createElement('div');row.className='md-compose-grid';grid.append(title,row);for(let l=5;l<=10;l++){const fig=document.createElement('figure'),c=canvas(160,160),cap=document.createElement('figcaption');cap.className='md-compose-caption';cap.textContent=(l===5?'원작 ':'시안 ')+'Lv.'+l;const dims=l===5?f.original.visible_size:calibration.find(v=>v.key===f.id+'-'+l).dest.slice(2);cap.textContent+=' · '+dims.map(n=>n.toFixed(1)).join(' × ');fig.append(c,cap);row.append(fig);cards.push({c,family:f.id,level:l})}const opt=document.createElement('option');opt.value=f.id;opt.textContent=f.name;document.querySelector('#md-focus-family').append(opt)}
const mode=document.querySelector('#md-compose-mode'),angle=document.querySelector('#md-compose-angle'),family=document.querySelector('#md-focus-family'),level=document.querySelector('#md-focus-level');family.value='msl';
function render(){for(const card of cards)draw(card.c,card.family,card.level,card.level===5?'original':mode.value,+angle.value);draw(document.querySelector('#md-compose-focus'),family.value,+level.value,mode.value,+angle.value);document.querySelector('#md-compose-angle-value').textContent=angle.value+'°'}
for(const input of [mode,angle,family,level])input.addEventListener('input',render);document.querySelector('#md-compose-reset').onclick=()=>{mode.value='new';angle.value=0;family.value='msl';level.value=10;render()};render();window.mdBodyCalibration=calibration;document.documentElement.dataset.bodyConceptsReady='true';
})().catch(e=>{console.error(e);document.documentElement.dataset.bodyConceptsError=e.message});
