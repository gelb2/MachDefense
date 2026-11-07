#!/usr/bin/env python3
"""Package product outputs only; original archives and personal saves are never inputs."""
import argparse, pathlib, shutil, hashlib, json, re
p=argparse.ArgumentParser();p.add_argument('dist',type=pathlib.Path);p.add_argument('--cheat',action='store_true');a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[2]
a.dist.mkdir(parents=True,exist_ok=True)
for f in (root/'Distribution/Web').iterdir():
 if f.is_file():shutil.copy2(f,a.dist/f.name)
# Original menu artwork for the pre-game launcher, not a second in-game UI.
shutil.copy2(root/'Resources/Images/Title.jpg',a.dist/'title.jpg')
icons=list((root/'Apple').rglob('AppIcon*.png'))
if not icons: icons=list((root/'Apple').rglob('*1024*.png'))
if icons:shutil.copy2(icons[0],a.dist/'icon.png')
# Split at fixed groups of original asset entries. A changed PNG changes its
# group, not every subsequent byte offset in the entire Emscripten data file.
parts=[]
chunk_dir=a.dist/'chunks'
if chunk_dir.exists(): shutil.rmtree(chunk_dir)
chunk_dir.mkdir()
data=(a.dist/'game.data').read_bytes()
entries=re.findall(r'filename:"[^"\n]+",start:([0-9.e+]+),end:([0-9.e+]+)',(a.dist/'game.js').read_text())
entries=[(int(float(a)),int(float(b))) for a,b in entries]
if not entries or int(entries[0][0])!=0 or int(entries[-1][1])!=len(data):
 raise SystemExit('Unknown Emscripten package layout')
for previous,current in zip(entries,entries[1:]):
 if previous[1]!=current[0]: raise SystemExit('Noncontiguous Emscripten package')
for i in range(0,len(entries),16):
 start=int(entries[i][0]);end=int(entries[min(i+15,len(entries)-1)][1])
 for offset in range(start,end,4*1024*1024):
  piece=data[offset:min(offset+4*1024*1024,end)]
  digest=hashlib.sha256(piece).hexdigest();name='chunks/'+digest+'.bin'
  (a.dist/name).write_bytes(piece)
  parts.append(dict(url=name,size=len(piece),sha256=digest))
files=[]
for f in sorted(a.dist.iterdir()):
 if f.is_file() and f.name not in ['asset-manifest.json','sw.js','build.json']:
  b=f.read_bytes();entry=dict(url=f.name,size=len(b),sha256=hashlib.sha256(b).hexdigest())
  if f.name=='game.data':entry['parts']=parts
  files.append(entry)
version=hashlib.sha256(json.dumps(files,sort_keys=True).encode()+(a.dist/'sw.js').read_bytes()).hexdigest()[:16]
manifest=dict(version=version,cheat=a.cheat,files=files)
(a.dist/'asset-manifest.json').write_text(json.dumps(manifest))
(a.dist/'build.json').write_text(json.dumps(dict(version=version,cheat=a.cheat)))

sw=a.dist/"sw.js"
sw.write_text(sw.read_text().replace("__BUILD_VERSION__",version).replace("__ASSET_MANIFEST__",json.dumps(manifest)))
