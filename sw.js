const VERSION='701b376ebb23a762',CACHE='machdefense-assets-'+VERSION;
const BASE=new URL('./',self.location.href);
async function prepareResources(){
 const cache=await caches.open(CACHE),manifestURL=new URL('asset-manifest.json',BASE);
 let response=await cache.match(manifestURL);if(!response)response=await fetch(manifestURL,{cache:'no-store'});
 if(!response.ok)throw Error('Resource list unavailable');const manifest=await response.json();
 let done=0,total=manifest.files.reduce((s,f)=>s+f.size,0);
 for(const file of manifest.files){
  const url=new URL(file.url,BASE);
  if(!await cache.match(url)){
   const result=await fetch(url,{cache:'no-store'});if(!result.ok)throw Error('Download failed: '+file.url);
   const bytes=await result.arrayBuffer();const hash=Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',bytes)),b=>b.toString(16).padStart(2,'0')).join('');
   if(hash!==file.sha256)throw Error('Resource checksum failed: '+file.url);
   await cache.put(url,new Response(bytes,{headers:result.headers}));
  }
  done+=file.size;for(const client of await self.clients.matchAll({includeUncontrolled:true}))client.postMessage({kind:'download',done,total});
 }
 await cache.put(manifestURL,new Response(JSON.stringify(manifest),{headers:{'Content-Type':'application/json'}}));
 if(!await cache.match(new URL('build.json',BASE)))await cache.put(new URL('build.json',BASE),await fetch(new URL('build.json',BASE),{cache:'no-store'}));
}
self.addEventListener('install',event=>event.waitUntil(prepareResources()));
self.addEventListener('message',event=>{if(event.data?.kind==='prepare')event.waitUntil(prepareResources().then(()=>event.ports[0]?.postMessage({ok:true}),error=>event.ports[0]?.postMessage({ok:false,error:error.message})));});
self.addEventListener('activate',event=>event.waitUntil((async()=>{for(const key of await caches.keys())if(key.startsWith('machdefense-assets-')&&key!==CACHE)await caches.delete(key);await self.clients.claim();})()));
self.addEventListener('fetch',event=>{
 const url=new URL(event.request.url);if(url.origin!==BASE.origin||!url.pathname.startsWith(BASE.pathname)||event.request.method!=='GET')return;
 event.respondWith((async()=>{const cache=await caches.open(CACHE);const key=event.request.mode==='navigate'?new URL('index.html',BASE):event.request;return await cache.match(key)||fetch(event.request);})());
});
