const VERSION='__BUILD_VERSION__',CACHE='machdefense-assets-'+VERSION;
const MANIFEST=__ASSET_MANIFEST__;
const BASE=new URL('./',self.location.href),BLOBS='machdefense-chunks-v1';
const urlFor=name=>new URL(name,BASE).href;
// The first published worker rewrites every cached file during preparation.
// Pin its manifest so this worker can serve its verified cache without writing it.
const LEGACY_CACHE='machdefense-assets-f9f26e94f1d4e3d6';
const LEGACY_MANIFEST_SHA256='60f4870d3efe8d8b6de31643f2d08ce845172bca6daf79f556415da947e0744a';
const release=releaseIntegrity(BASE.href,'machdefense-assets-',CACHE,async key=>
 Boolean(await (await caches.open(key)).match(urlFor('asset-manifest.json'))));
const scopedClients=async()=> (await self.clients.matchAll({includeUncontrolled:true,type:'window'})).filter(client=>client.url.startsWith(BASE.href));
const digest=async bytes=>Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',bytes)),b=>b.toString(16).padStart(2,'0')).join('');
let preparation=null, promotion=null, fallback;
async function checked(response,file){
 if(!response?.ok)return null;
 const bytes=await response.arrayBuffer();
 return bytes.byteLength===file.size&&await digest(bytes)===file.sha256?bytes:null;
}
async function download(file){
 const response=await fetch(urlFor(file.url),{cache:'no-store'}),bytes=await checked(response,file);
 if(!bytes)throw Error('Resource checksum failed: '+file.url);
 return bytes;
}
function responseFor(bytes,name){
 const type=name.endsWith('.js')?'text/javascript':name.endsWith('.html')?'text/html':name.endsWith('.css')?'text/css':name.endsWith('.wasm')?'application/wasm':name.endsWith('.json')?'application/json':name.endsWith('.webmanifest')?'application/manifest+json':'application/octet-stream';
 return new Response(bytes,{headers:{'Content-Type':type,'X-MachDefense-Version':VERSION}});
}
async function prepareResources(){
 if(preparation)return preparation;
 preparation=prepare().finally(()=>{preparation=null;});return preparation;
}
async function prepare(){
 if(MANIFEST.version!==VERSION)throw Error('Mixed resource versions');
 const cache=await caches.open(CACHE),blobs=await caches.open(BLOBS);
 const old=[];
 for(const key of await release.olderCaches())old.push(await caches.open(key));
 let done=0,downloaded=0,reused=0;const total=MANIFEST.files.reduce((n,f)=>n+f.size,0);
 for(const file of MANIFEST.files){
  let bytes=null;
  if(file.parts){
   const output=new Uint8Array(file.size);let offset=0;
   for(const part of file.parts){
    let piece=await checked(await blobs.match(urlFor(part.url)),part);
    if(piece)reused+=piece.byteLength;
    else{piece=await download(part);downloaded+=piece.byteLength;await blobs.put(urlFor(part.url),responseFor(piece,part.url));}
    output.set(new Uint8Array(piece),offset);offset+=piece.byteLength;
    for(const client of await scopedClients())client.postMessage({kind:'download',version:VERSION,done:done+offset,total,downloaded,reused});
   }
   if(offset!==file.size||await digest(output)!==file.sha256)throw Error('Resource assembly failed');
   bytes=output;
   // Store only the chunks. A second full data copy doubles the offline quota.
   await cache.delete(urlFor(file.url));
  }else{
   const local=await checked(await cache.match(urlFor(file.url)),file);
   bytes=local;
   if(!bytes)for(const source of old){bytes=await checked(await source.match(urlFor(file.url)),file);if(bytes)break;}
   if(bytes)reused+=file.size;
   else{bytes=await download(file);downloaded+=bytes.byteLength;}
   if(!local)await cache.put(urlFor(file.url),responseFor(bytes,file.url));
  }
  done+=file.size;
  for(const client of await scopedClients())client.postMessage({kind:'download',version:VERSION,done,total,downloaded,reused});
 }
 // Only complete, verified releases receive this marker. Progress lives in
 // IndexedDB and is never touched by this worker.
 const metadata={'asset-manifest.json':MANIFEST,'build.json':{version:VERSION,cheat:MANIFEST.cheat}};
 for(const [name,value] of Object.entries(metadata)){
  const text=JSON.stringify(value);
  if(await (await cache.match(urlFor(name)))?.text()!==text)await cache.put(urlFor(name),responseFor(text,name));
 }
 return {version:VERSION,downloaded,reused};
}
async function assemble(file){
 const blobs=await caches.open(BLOBS),output=new Uint8Array(file.size);let offset=0;
 for(const part of file.parts){
  const bytes=await checked(await blobs.match(urlFor(part.url)),part);
  if(!bytes||offset+bytes.byteLength>output.byteLength)return null;
  output.set(new Uint8Array(bytes),offset);
  offset+=bytes.byteLength;
 }
 return offset===file.size&&await digest(output)===file.sha256?output:null;
}
async function legacyManifest(){
 if(MANIFEST.cheat||!await caches.has(LEGACY_CACHE))return null;
 const cache=await caches.open(LEGACY_CACHE);
 const response=await cache.match(urlFor('asset-manifest.json'));
 if(!response)return null;
 const bytes=await response.arrayBuffer();
 if(await digest(bytes)!==LEGACY_MANIFEST_SHA256)return null;
 return JSON.parse(new TextDecoder().decode(bytes));
}
async function verifyLegacy(manifest){
 const cache=await caches.open(LEGACY_CACHE);
 for(const file of manifest.files){
  const bytes=file.parts?await assemble(file):await checked(await cache.match(urlFor(file.url)),file);
  if(!bytes)throw Error('Cached release file failed verification: '+file.url);
 }
}
async function usingFallback(){
 if(fallback!==undefined)return fallback;
 const published=await caches.has(CACHE)&&
  await (await caches.open(CACHE)).match(urlFor('.client-editions'));
 fallback=!published&&Boolean(await legacyManifest());
 return fallback;
}
function promoteRelease(){
 if(promotion)return promotion;
 promotion=(async()=>{
  const older=await release.olderCaches();
  try{
   const result=await prepareResources();
   await release.stage(await caches.open(CACHE),older);
   await retainChunks();
   fallback=false;
   return result;
  }catch(error){
   await caches.delete(CACHE);
   await retainChunks().catch(()=>{});
   if(older.at(-1)!==LEGACY_CACHE)throw error;
   const manifest=await legacyManifest();
   if(!manifest)throw error;
   await verifyLegacy(manifest);
   fallback=true;
   return {version:manifest.version,downloaded:0,reused:manifest.files.reduce((n,file)=>n+file.size,0)};
  }
 })().finally(()=>{promotion=null;});
 return promotion;
}
self.addEventListener('install',event=>event.waitUntil((async()=>{
 await promoteRelease();
 await self.skipWaiting();
})()));
self.addEventListener('message',event=>{
 const reply=value=>event.ports[0]?.postMessage(value);
 if(event.data?.kind==='prepare')event.waitUntil((async()=>{
  if(await usingFallback()){reply({ok:true,...await promoteRelease()});return;}
  const edition=await release.edition(event.source?.id);
  if(edition===CACHE){reply({ok:true,...await prepareResources()});return;}
  const previous=await (await (await caches.open(edition)).match(urlFor('build.json')))?.json();
  if(!previous)throw Error('Reopen the app to load a verified release.');
  reply({ok:true,version:previous.version});
 })().catch(error=>reply({ok:false,error:error.message})));
 if(event.data?.kind==='activate')event.waitUntil((async()=>{
  const result=await usingFallback()?await promoteRelease():await prepareResources();
  reply({ok:true,version:result.version});await self.skipWaiting();
 })().catch(error=>reply({ok:false,error:error.message})));
});
async function retainChunks(){
 const versions=await release.olderCaches();
 if((await caches.keys()).includes(CACHE))versions.push(CACHE);
 const needed=new Set();
 for(const key of versions){
  const cache=await caches.open(key),response=await cache.match(urlFor('asset-manifest.json'));
  if(!response)continue;const manifest=await response.json();
  for(const file of manifest.files)for(const part of file.parts||[])needed.add(urlFor(part.url));
 }
 const blobs=await caches.open(BLOBS);for(const request of await blobs.keys())if(request.url.startsWith(urlFor('chunks/'))&&!needed.has(request.url))await blobs.delete(request);
}
// A reopened Safari app can start without a controller while an older cache
// remains. Take control only after install has staged the verified release.
self.addEventListener('activate',event=>event.waitUntil(self.clients.claim()));
self.addEventListener('fetch',event=>{
 const url=new URL(event.request.url);if(url.origin!==BASE.origin||!url.pathname.startsWith(BASE.pathname)||event.request.method!=='GET')return;
 // SW updates must always reach the server. Never cache the worker script.
 if(url.pathname===new URL('sw.js',BASE).pathname)return;
 const navigation=event.request.mode==='navigate';
 const name=navigation?'index.html':decodeURIComponent(url.pathname.slice(BASE.pathname.length));
 event.respondWith((async()=>{
  const edition=await usingFallback()?LEGACY_CACHE:navigation?CACHE:await release.edition(event.clientId);
  const cache=await caches.open(edition);
  const manifest=edition===CACHE?MANIFEST:edition===LEGACY_CACHE?await legacyManifest():
   await (await cache.match(urlFor('asset-manifest.json')))?.json();
  if(edition===LEGACY_CACHE&&!manifest)throw Error('Cached release manifest failed verification');
  let response=await cache.match(urlFor(name));
  const file=manifest?.files.find(file=>file.url===name);
  if(!manifest&&response)return response;
  if(!file&&!['build.json','asset-manifest.json'].includes(name))throw Error('Unlisted release file');

  if(file?.parts){
   let bytes=await assemble(file);if(!bytes&&edition===CACHE){await prepareResources();bytes=await assemble(file);}
   if(!bytes)throw Error('Resource assembly unavailable');return responseFor(bytes,name);
  }
  if(file){const bytes=await checked(response,file);if(bytes)return responseFor(bytes,name);}
  else if(manifest)return responseFor(JSON.stringify(name==='build.json'?{version:manifest.version,cheat:manifest.cheat}:manifest),name);
  if(edition!==CACHE)throw Error('Reopen the app to load a verified release.');
  await prepareResources();response=await cache.match(urlFor(name));
  if(!response)throw Error('Resource unavailable: '+name);return response;
 })().catch(async error=>{const message=name+': '+String(error);for(const client of await self.clients.matchAll())client.postMessage({kind:'resource-error',message});return new Response(message,{status:503});}));
});
