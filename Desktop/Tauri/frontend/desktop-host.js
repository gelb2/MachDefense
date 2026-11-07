/* Desktop adapter only. Game buttons and the MD1 codec remain in C++. */
window.MDDesktop={closing:false,async checkUpdate(){
 try{
  const version=await __TAURI__.core.invoke('update_check');if(!version||document.querySelector('#launcher').hidden)return;
  const button=document.createElement('button');button.textContent='UPDATE TO '+version;button.id='desktop-update';
  document.querySelector('#play').before(button);
  button.onclick=async()=>{if(document.querySelector('#launcher').hidden)return;button.disabled=true;document.querySelector('#play').disabled=true;button.textContent='DOWNLOADING UPDATE…';
   try{await MDStore.flush();await __TAURI__.core.invoke('update_install',{version});}
   catch(e){MDError('Could not update: '+String(e));button.disabled=false;document.querySelector('#play').disabled=false;button.textContent='RETRY UPDATE';}
  };
 }catch(e){console.info('Update check unavailable; using the bundled game.');}
},async close(){
 if(this.closing)return;this.closing=true;
 try{if(window.MDTest)await MDTest.activity(false);await MDStore.flush();await __TAURI__.core.invoke('finish_close');}
 catch(e){this.closing=false;MDError('Could not finish saving: '+String(e));}
}};
window.MDStore={current:null,revision:0,writes:0,pending:Promise.resolve(),unsaved:null,
 async open(cheat){this.cheat=cheat;const s=await __TAURI__.core.invoke('save_read',{cheat});this.current=s?new Uint8Array(s.bytes):null;this.revision=s?.revision||0;},
 async write(bytes,restoring=false){
  const snapshot=Array.from(bytes);const operation=this.pending.then(async()=>{
   this.unsaved={bytes:snapshot,restoring};
   const s=await __TAURI__.core.invoke('save_write',{cheat:this.cheat,bytes:snapshot,expected:this.revision,restoring});
   this.current=new Uint8Array(s.bytes);this.revision=s.revision;this.unsaved=null;++this.writes;return true;
  });this.pending=operation.catch(()=>{});
  try{return await operation;}catch(e){MDError('Could not save: '+String(e));return false;}
 },
 async flush(){await this.pending;if(this.unsaved){const s=this.unsaved;if(!await this.write(s.bytes,s.restoring))throw Error('Progress has not been saved. Try closing again.');}}
};
