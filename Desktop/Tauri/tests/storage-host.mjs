import vm from 'node:vm';
import fs from 'node:fs';
import assert from 'node:assert/strict';
const src=fs.readFileSync(new URL('../frontend/desktop-host.js',import.meta.url),'utf8');
let failures=0,revision=0,closed=0,active=0;
const errors=[];let ctx;
const invoke=async(name,args)=>{
 if(name==='save_read')return null;
 if(name==='finish_close'){closed++;return;}
 assert.equal(name,'save_write');if(failures){failures--;throw Error('disk failure');}
 assert.equal(args.expected,revision);return {bytes:args.bytes,revision:++revision};
};
ctx=vm.createContext({Uint8Array,Array,Promise,Error,__TAURI__:{core:{invoke}},MDError:e=>errors.push(e),MDTest:{async activity(v){assert.equal(v,false);active++;}}});ctx.window=ctx;vm.runInContext(src,ctx);
await ctx.MDStore.open(false);
const a=new Uint8Array(42496).fill(1),b=new Uint8Array(42496).fill(2);
failures=1;assert.equal(await ctx.MDStore.write(a),false);
assert.equal(await ctx.MDStore.write(b),true);assert.equal(ctx.MDStore.current[0],2);
failures=1;assert.equal(await ctx.MDStore.write(a),false);
await ctx.MDDesktop.close();assert.equal(closed,1);assert.equal(ctx.MDStore.current[0],1);assert.equal(active,1);
ctx.MDDesktop.closing=false;failures=2;await ctx.MDStore.write(b);await ctx.MDDesktop.close();assert.equal(closed,1);assert.equal(ctx.MDDesktop.closing,false);
await ctx.MDDesktop.close();assert.equal(closed,2);assert.equal(ctx.MDStore.current[0],2);
console.log('PASS: failed save recovery, pending-write flush, failed-close retry, revision order');
