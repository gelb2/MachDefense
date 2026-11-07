const $=id=>document.getElementById(id),canvas=$('game');let module=null,busy=false,chain=Promise.resolve(),running=false,last=0,installPrompt=null,build=null,preparing=true;
window.MDError=message=>{$('error').textContent=message;$('error').hidden=false;};
window.MDText={input(title,initial,max){return new Promise(resolve=>{$('textLabel').textContent=title;$('textInput').value=initial;$('textInput').maxLength=max;$('textDialog').showModal();$('textInput').focus();$('textDialog').addEventListener('close',()=>resolve($('textDialog').returnValue==='ok'?$('textInput').value:null),{once:true});});}};
// The game draws and handles every recovery action. This field supplies text only.
window.MDTransfer={bounds:null,viewport:{w:320,h:480},base:null,
 open(text,readOnly,bounds){this.base={w:innerWidth,h:innerHeight};this.bounds=bounds;const field=$('transferInput');field.value=text;field.readOnly=readOnly;field.hidden=false;this.layout();if(!readOnly)field.focus({preventScroll:true});},
 text(){return $('transferInput').value;},
 layout(){const field=$('transferInput');if(field.hidden||!this.bounds)return;const r=canvas.getBoundingClientRect(),b=this.bounds,v=this.viewport;Object.assign(field.style,{left:(r.left+b.x*r.width/v.w)+'px',top:(r.top+b.y*r.height/v.h)+'px',width:(b.w*r.width/v.w)+'px',height:(b.h*r.height/v.h)+'px'});},
 close(){const field=$('transferInput');field.blur();field.hidden=true;field.value='';this.bounds=null;this.base=null;canvas.style.height='';if(module)resize();},
 async copy(text){try{if(navigator.clipboard?.writeText){await navigator.clipboard.writeText(text);return;}const field=$('transferInput');field.focus();field.select();if(!document.execCommand('copy'))throw Error('Select and copy the recovery code.');}catch(e){MDError('Could not copy. Select the code and copy it.');}}
};
function call(name,types=[],args=[],result=null){chain=chain.then(async()=>{busy=true;try{return await module.ccall(name,result,types,args,{async:true});}finally{busy=false;}});chain=chain.catch(e=>{MDError(String(e));throw e;});return chain;}
function resize(){const w=innerWidth,base=MDTransfer.base,h=base&&base.w===w?Math.max(innerHeight,base.h):innerHeight,dpr=Math.min(devicePixelRatio||1,2);canvas.style.height=h+'px';canvas.width=Math.round(w*dpr);canvas.height=Math.round(h*dpr);const probe=document.createElement('div');probe.style='position:fixed;padding-top:env(safe-area-inset-top);padding-bottom:env(safe-area-inset-bottom)';document.body.append(probe);const style=getComputedStyle(probe),top=parseFloat(style.paddingTop)||0,bottom=parseFloat(style.paddingBottom)||0;probe.remove();return call('md_resize',['number','number','number','number','number','number'],[w,h,canvas.width,canvas.height,top,bottom]);}
async function activity(active){if(!module)return;await call('md_active',['number'],[+active]);await MDAudio.setActive(active);last=0;}
function frame(now){requestAnimationFrame(frame);if(!running||busy||document.hidden||window.MD_TEST_PAUSE)return;if(now+.2<last)return;last=Math.max(last+1000/60,now);call('md_tick');}
for(const [event,phase] of [['pointerdown',0],['pointermove',1],['pointerup',2],['pointercancel',3]])canvas.addEventListener(event,e=>{if(!running)return;if(phase===0){canvas.setPointerCapture(e.pointerId);MDAudio.context?.resume();}if(phase===1&&!canvas.hasPointerCapture(e.pointerId))return;const r=canvas.getBoundingClientRect();call('md_touch',['number','number','number','number'],[e.pointerId,phase,(e.clientX-r.left)/r.width,(e.clientY-r.top)/r.height]);e.preventDefault();});
canvas.addEventListener('wheel',e=>{if(!running)return;const r=canvas.getBoundingClientRect();const unit=e.deltaMode===1?16:e.deltaMode===2?r.height:1;call('md_scroll',['number','number','number'],[(e.clientX-r.left)/r.width,(e.clientY-r.top)/r.height,e.deltaY*unit/r.height]);e.preventDefault();},{passive:false});
canvas.addEventListener('contextmenu',e=>e.preventDefault());canvas.addEventListener('webglcontextlost',e=>{e.preventDefault();running=false;call('md_context',['number'],[0]);MDAudio.setActive(false);});canvas.addEventListener('webglcontextrestored',async()=>{await call('md_context',['number'],[1]);await resize();await activity(true);running=true;});
addEventListener('resize',()=>{if(module)resize();});document.addEventListener('visibilitychange',()=>activity(!document.hidden));addEventListener('pagehide',()=>activity(false));addEventListener('pageshow',()=>{if(running)activity(true);});
const standalone=matchMedia('(display-mode: standalone)');let installed=false;
function installHelp(){
 const ua=navigator.userAgent,ios=/iPhone|iPad|iPod/.test(ua)||(/Macintosh/.test(ua)&&navigator.maxTouchPoints>1);
 if(ios)return 'In Safari, tap Share → Add to Home Screen. Open MachDefense from its new icon.';
 if(/Macintosh/.test(ua)&&/Safari/.test(ua)&&!/Chrome|Chromium|Edg/.test(ua))return 'In Safari, choose File → Add to Dock. Open MachDefense from the Dock.';
 if(/Android/.test(ua))return 'Open the browser menu → Add to Home screen or Install app. If already installed, open the MachDefense icon.';
 if(/Chrome|Chromium/.test(ua)&&!/Edg/.test(ua))return 'In Chrome, choose ⋮ → Cast, save, and share → Install page as app. If already installed, open MachDefense from your apps.';
 if(/Edg/.test(ua))return 'In Edge, open the menu → Apps → Install this site as an app.';
 return 'Open the browser menu to install this site, or choose PLAY IN BROWSER below.';
}
function updateInstall(){
 const app=standalone.matches||navigator.standalone===true;
 $('install').hidden=preparing||app||installed;
 $('install').textContent=installPrompt?'INSTALL GAME':'HOW TO INSTALL';
 $('installHelp').textContent=app?'Ready to play offline.':installed?'Installed. Open MachDefense from your apps, or play in this browser.':installPrompt?'Install MachDefense to open it from your apps and play offline.':installHelp();
 $('play').textContent=app?'PLAY':'PLAY IN BROWSER';
}
addEventListener('beforeinstallprompt',e=>{e.preventDefault();installPrompt=e;updateInstall();});
addEventListener('appinstalled',()=>{installed=true;installPrompt=null;updateInstall();});
standalone.addEventListener('change',updateInstall);
$('install').onclick=async()=>{
 if(!installPrompt){updateInstall();$('installHelp').focus();return;}
 const prompt=installPrompt;installPrompt=null;$('install').disabled=true;
 try{await prompt.prompt();const choice=await prompt.userChoice;if(choice.outcome==='accepted')installed=true;}
 catch(e){$('installHelp').textContent=installHelp();}
 finally{$('install').disabled=false;updateInstall();}
};
updateInstall();
function workerRequest(worker,kind){return new Promise((resolve,reject)=>{
 const channel=new MessageChannel(),timer=setTimeout(()=>{channel.port1.close();reject(Error('Resource check timed out.'));},180000);
 channel.port1.onmessage=e=>{clearTimeout(timer);channel.port1.close();e.data.ok||e.data.waiting?resolve(e.data):reject(Error(e.data.error));};
 worker.postMessage({kind},[channel.port2]);
});}
function waitInstallation(worker){return new Promise((resolve,reject)=>{
 const check=()=>{if(['installed','activated'].includes(worker.state)){cleanup();resolve();}else if(worker.state==='redundant'){cleanup();reject(Error('Resource download failed.'));}};
 const timer=setTimeout(()=>{cleanup();reject(Error('Resource download timed out.'));},180000);
 function cleanup(){clearTimeout(timer);worker.removeEventListener('statechange',check);}
 worker.addEventListener('statechange',check);check();
});}
async function prepare(){try{
 build=await(await fetch('build.json',{cache:'no-store'})).json();
 if(!('serviceWorker'in navigator))throw Error('Open this game over HTTPS to enable offline play.');
 navigator.serviceWorker.addEventListener('message',e=>{if(preparing&&e.data.kind==='download'){$('progress').value=100*e.data.done/e.data.total;$('status').textContent='Checking game resources… '+Math.round(e.data.done/1048576)+' / '+Math.round(e.data.total/1048576)+' MB';}});
 const reg=await navigator.serviceWorker.register('sw.js',{updateViaCache:'none'});
 try{await reg.update();if(reg.installing)await waitInstallation(reg.installing);}
 catch(error){if(!reg.active)throw error;/* Keep a complete older release when the update fails. */}
 if(!reg.active&&reg.installing)await waitInstallation(reg.installing);
 if(reg.waiting){
  const waiting=reg.waiting,result=await workerRequest(waiting,'activate');
  if(result.ok){
   await new Promise(resolve=>{if(navigator.serviceWorker.controller===waiting)resolve();else navigator.serviceWorker.addEventListener('controllerchange',resolve,{once:true});});
   // Reload the launcher's JS/CSS as well, before any C++ game code starts.
   location.reload();return;
  }
  window.MDUpdateWaiting=true;
 }
 if(!navigator.serviceWorker.controller)await new Promise(resolve=>navigator.serviceWorker.addEventListener('controllerchange',resolve,{once:true}));
 const ready=await workerRequest(navigator.serviceWorker.controller,'prepare');
 if(ready.version&&ready.version!==build.version){location.reload();return;}
 window.MDResourceCheck=ready;await MDStore.open(build.cheat);
 preparing=false;$('progress').value=100;$('status').textContent=window.MDUpdateWaiting?'Ready. Close other game windows to apply the update.':'Ready for offline play';$('play').disabled=false;updateInstall();
}catch(e){MDError('Cannot prepare game: '+e.message);$('status').textContent='Reload to retry.';}}
$('play').onclick=async()=>{try{$('play').disabled=true;$('status').textContent='Loading game…';await navigator.storage?.persist?.();await new Promise((resolve,reject)=>{const s=document.createElement('script');s.src='game.js';s.onload=resolve;s.onerror=reject;document.body.append(s);});module=await createMachDefense({canvas,printErr:console.error});window.MDModule=module;await MDAudio.prepare(module);await MDAudio.context.resume();canvas.hidden=false;const ok=await call('md_start',[],[],'number');if(!ok)throw Error('Cannot start game renderer or audio');await resize();$('launcher').hidden=true;running=true;requestAnimationFrame(frame);canvas.focus();}catch(e){MDError(String(e));}};
window.MDTest={call,async advance(n){window.MD_TEST_PAUSE=true;for(let i=0;i<n;i++)await call('md_tick');},async tap(x,y){await call('md_touch',['number','number','number','number'],[1,0,x,y]);await call('md_touch',['number','number','number','number'],[1,2,x,y]);},async state(){return JSON.parse(await call('md_debug',[],[],'string'));},async sprites(){return JSON.parse(await call('md_ui',[],[],'string'));},async save(){return JSON.parse(await call('md_save_info',[],[],'string'));},activity};
prepare();
