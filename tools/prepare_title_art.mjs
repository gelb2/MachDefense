#!/usr/bin/env node
// Bake the approved report composition once. Runtime only reads static textures.
import {chromium} from '../Web/tests/node_modules/playwright/index.mjs';
import fs from 'node:fs/promises';
import path from 'node:path';
import crypto from 'node:crypto';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..');
const art=path.join(root,'Art/Title/20261110'),output=path.join(root,'Resources/Generated/Title');
const source=await fs.readFile(path.join(root,'docs/research/title-joins-20261110/composite.js'),'utf8');
const drawing=source.slice(0,source.indexOf('function render()'));
const inputs={original:'Resources/Images/Title.jpg',wide:'Art/Title/20261110/inputs/wide-natural.webp',tall:'Art/Title/20261110/inputs/tall.webp'};
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const urls={},hashes={};for(const [key,file] of Object.entries(inputs)){const b=await fs.readFile(path.join(root,file));urls[key]=`data:image/${key==='original'?'jpeg':'webp'};base64,${b.toString('base64')}`;hashes[file]=sha(b)}
if(hashes[inputs.original]!=='50ce101b40757018130437f411e64dd8709234ebf649f7f8e9d555bb73cbf696')throw Error('Original title changed');
await fs.mkdir(output,{recursive:true});await fs.mkdir(path.join(art,'prepared-source'),{recursive:true});
const browser=await chromium.launch({headless:true,executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'});
const rows=[];
try{
 const page=await browser.newPage();await page.setContent('<html><body></body></html>');
 await page.addScriptTag({content:drawing+'\nwindow.bakeTitle=async(urls,w,h)=>{for(const [k,url] of Object.entries(urls)){const im=new Image;im.src=url;await im.decode();assets[k]=im}const c=document.createElement("canvas");c.width=w===3200?1600:w;c.height=h;draw(c);if(w!==3200)return c.toDataURL("image/png").split(",")[1];const wide=document.createElement("canvas");wide.width=w;wide.height=h;const ctx=wide.getContext("2d");ctx.imageSmoothingQuality="high";cover(ctx,assets.wide,0,0,assets.wide.width,assets.wide.height,0,0,w,h);matchScenery(ctx,c,800,1600,w,h);ctx.drawImage(c,800,0);const a=c.getContext("2d").getImageData(0,0,1600,900).data,b=ctx.getImageData(800,0,1600,900).data;for(let i=0;i<a.length;i++)if(a[i]!==b[i])throw Error("Adopted wide region changed");return wide.toDataURL("image/png").split(",")[1]};'});
 for(const [name,w,h] of [['Wide',3200,900],['Tall',390,844]]){
  const b64=await page.evaluate(({urls,w,h})=>window.bakeTitle(urls,w,h),{urls,w,h});
  const file=path.join(art,'prepared-source',name+'.png');await fs.writeFile(file,Buffer.from(b64,'base64'));
  const prepared=path.join(output,name+'.png');
  execFileSync('xcrun',['pngcrush','-iphone','-f','0',file,prepared],{stdio:'pipe'});
  const b=await fs.readFile(prepared);if(b.subarray(12,16).toString()!=='CgBI')throw Error('Apple PNG preparation missing');
  execFileSync('python3',['-c','import sys,pathlib;sys.path.insert(0,str(pathlib.Path(sys.argv[1])/"tools"));from prepare_resources import png_pixels;assert png_pixels(pathlib.Path(sys.argv[2]).read_bytes())[1]==png_pixels(pathlib.Path(sys.argv[3]).read_bytes())[1]',root,file,prepared]);
  rows.push({name,width:w,height:h,source:path.relative(root,file),sourceSHA256:sha(await fs.readFile(file)),runtime:path.relative(root,prepared),runtimeSHA256:sha(b),cgbi:true,preparedRGBAUnchanged:true});
 }
}finally{await browser.close()}
const manifest={acceptedReport:'docs/research/title-joins-20261110.md',drawing:'docs/research/title-joins-20261110/composite.js',drawingSHA256:sha(Buffer.from(source)),inputs:hashes,adoptedWideRegion:[800,0,1600,900],adoptedWidePixelsUnchanged:true,preparation:'Approved browser Canvas composition, then Apple pngcrush -iphone -f 0. Additional scenery beyond the adopted wide region supports 32:9 without vertical recropping. Only outside rectangles are consumed by the game; Title.jpg remains separate.',textures:rows};
await fs.writeFile(path.join(art,'runtime-art.json'),JSON.stringify(manifest,null,2)+'\n');console.log(JSON.stringify(rows));
