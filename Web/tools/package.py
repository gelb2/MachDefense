#!/usr/bin/env python3
"""Package product outputs only; original archives and personal saves are never inputs."""
import argparse, pathlib, shutil, hashlib, json
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
files=[]
for f in sorted(a.dist.iterdir()):
 if f.is_file() and f.name not in ['asset-manifest.json','sw.js','build.json']:
  b=f.read_bytes();files.append(dict(url=f.name,size=len(b),sha256=hashlib.sha256(b).hexdigest()))
version=hashlib.sha256(json.dumps(files,sort_keys=True).encode()+(a.dist/'sw.js').read_bytes()).hexdigest()[:16]
(a.dist/'asset-manifest.json').write_text(json.dumps(dict(version=version,files=files)))
(a.dist/'build.json').write_text(json.dumps(dict(version=version,cheat=a.cheat)))

sw=a.dist/"sw.js"
sw.write_text(sw.read_text().replace("__BUILD_VERSION__",version))
