/* Progress uses a separate database from replaceable PWA resource caches. */
window.MDStore={current:null,revision:0,db:null,conflict:false,writes:0,
 async open(cheat){
  this.db=await new Promise((resolve,reject)=>{const q=indexedDB.open(cheat?'machdefense-cheat-v1':'machdefense-v1',1);q.onupgradeneeded=()=>q.result.createObjectStore('save');q.onsuccess=()=>resolve(q.result);q.onerror=()=>reject(q.error);q.onblocked=()=>reject(Error('Close other MachDefense windows and retry.'));});
  const row=await this.get('current');this.current=row?.bytes?new Uint8Array(row.bytes):null;this.revision=row?.revision||0;
  if(this.current&&this.current.length!==42496)throw Error('The saved game is damaged. Your data has been kept.');
 },
 get(key){return new Promise((resolve,reject)=>{const q=this.db.transaction('save').objectStore('save').get(key);q.onsuccess=()=>resolve(q.result);q.onerror=()=>reject(q.error);});},
 async write(bytes,restoring=false){
  if(bytes.length!==42496||this.conflict)return false;
  const expected=this.revision;const snapshot=new Uint8Array(bytes);
  try{const revision=await new Promise((resolve,reject)=>{const tx=this.db.transaction('save','readwrite');const store=tx.objectStore('save');const q=store.get('current');let next;
   q.onsuccess=()=>{const old=q.result;if((old?.revision||0)!==expected){this.conflict=true;tx.abort();return;}next=expected+1;if(old){store.put(old,'backup');if(restoring)store.put(old,'restoreBackup');}store.put({bytes:snapshot,revision:next},'current');};
   tx.oncomplete=()=>resolve(next);tx.onabort=()=>reject(tx.error||Error('Another window has saved newer progress. Reload this window.'));tx.onerror=()=>{};
  });this.current=snapshot;this.revision=revision;++this.writes;return true;
  }catch(e){window.MDError?.(e.message||'Save failed. Your previous save has been kept.');return false;}
 }
};
