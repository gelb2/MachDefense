(async()=>{
 const results={started:new Date().toISOString(),userAgent:navigator.userAgent,steps:[]};
 const wait=async(fn,label)=>{for(let i=0;i<600;i++){if(await fn())return;await new Promise(r=>setTimeout(r,100));}throw Error('Timeout: '+label);};
 const assert=(v,m)=>{if(!v)throw Error(m);results.steps.push(m);};
 try{
 await wait(()=>window.MDStore&&document.querySelector('#play')&&!document.querySelector('#play').disabled,'launcher');
 window.MD_TEST_PAUSE=true;
 const saved=MDStore.current;results.preexisting=!!saved;
 window.MDText.input=async()=> 'DESKTOP';
 await document.querySelector('#play').onclick();
 await wait(()=>window.MDModule,'module');
 assert((await MDTest.state()).ready,'WebGL game ready');
 await MDTest.tap(.5,.5);await MDTest.advance(180);
 const tapImage=async(image)=>{const sprites=await MDTest.sprites(),state=await MDTest.state();const s=sprites.find(x=>x.image===image);if(!s)throw Error('Missing '+image);await MDTest.tap((s.x+s.w/2)/state.width,(s.y+s.h/2)/state.height);};
 if(window.MDValidationRecovery){
  let s=await MDTest.state();await MDTest.tap((s.width/2+75)/s.width,((s.height-480)/2+72)/s.height);
  const field=document.querySelector('#transferInput');assert(!field.hidden,'Recovery text field open');field.value=MDValidationRecovery.trim();
  await MDTest.tap((s.width/2-75)/s.width,71/s.height);await tapImage('Common_Btn_YES.png');await tapImage('Common_Btn_OK.png');
  s=await MDTest.state();await MDTest.tap((s.width/2-75)/s.width,((s.height-480)/2+72)/s.height);
  assert(field.value===MDValidationRecovery.trim(),'Recovery fixture imported and exported unchanged');
  await MDTest.tap((s.width/2+75)/s.width,71/s.height);results.recoveredSlots=await MDTest.save();
 }
 if(!saved&&!window.MDValidationRecovery){await tapImage('Common_Btn_NEW.png');await MDTest.advance(2);assert(MDStore.current?.length===42496,'Profile saved to desktop file');}
 assert((await MDTest.save())[0].active,'Profile present after open');
 await tapImage('iPon_SelSlot_Sel@2x.png');await MDTest.advance(180);
 await tapImage('Common_Btn_OK.png');await MDTest.advance(180);
 assert((await MDTest.sprites()).some(x=>x.image==='RaderMark.png'),'Mission rendered');
 results.state=await MDTest.state();results.save=await MDTest.save();results.revision=MDStore.revision;
 await MDTest.activity(false);await MDTest.activity(true);
 assert((await MDTest.state()).ready,'Resume lifecycle completed');
 results.pass=true;
 }catch(e){results.pass=false;results.error=String(e);results.screenError=document.querySelector('#error')?.textContent;}
 await __TAURI__.core.invoke('diagnostics_write',{report:JSON.stringify(results,null,2)});
 await MDDesktop.close();
})()
