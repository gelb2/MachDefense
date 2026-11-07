#!/usr/bin/env python3
"""Bundle the existing web product, with a desktop-only storage adapter."""
from pathlib import Path
import hashlib,json,shutil,subprocess
root=Path(__file__).resolve().parents[3];desktop=root/'Desktop/Tauri'
source=root/'build/web-normal/dist';out=desktop/'build/frontend'
manifest=json.loads((source/'asset-manifest.json').read_text())
if manifest['cheat']:raise SystemExit('Normal desktop bundle requires normal web product')
if out.exists():shutil.rmtree(out)
out.mkdir(parents=True)
for entry in manifest['files']:
 p=source/entry['url'];b=p.read_bytes()
 if hashlib.sha256(b).hexdigest()!=entry['sha256'] or len(b)!=entry['size']:raise SystemExit('Invalid web file: '+entry['url'])
 shutil.copy2(p,out/p.name)
shutil.copy2(source/'build.json',out/'build.json')
shutil.copy2(desktop/'frontend/desktop-host.js',out/'desktop-host.js')
p=out/'index.html';p.write_text(p.read_text().replace('<script src="app.js">','<script src="desktop-host.js"></script><script src="app.js">'))
(out/'desktop-source.json').write_text(json.dumps({'web_version':manifest['version'],'files':[{k:f[k] for k in ('url','size','sha256')} for f in manifest['files']]},indent=2))
print('Desktop web product:',manifest['version'])
notices=desktop/'build/notices'
if notices.exists():shutil.rmtree(notices)
notices.mkdir(parents=True)
for name in ('LICENSE','LICENSE-SCOPE.md','ASSET-LICENSES.md','THIRD_PARTY_NOTICES.md'):
 shutil.copy2(root/name,notices/name)
shutil.copytree(root/'LICENSES',notices/'LICENSES')
metadata=json.loads(subprocess.check_output(['cargo','metadata','--locked','--format-version','1','--manifest-path',str(desktop/'src-tauri/Cargo.toml')]))
packages=[]
for p in metadata['packages']:
 if not p.get('source'):continue
 base=Path(p['manifest_path']).parent;target=notices/'rust'/(p['name']+'-'+p['version']);files=[]
 for item in base.iterdir():
  if item.is_file() and item.name.upper().startswith(('LICENSE','LICENCE','COPYING','NOTICE','COPYRIGHT')):
   target.mkdir(parents=True,exist_ok=True);shutil.copy2(item,target/item.name);files.append(item.name)
 packages.append({'name':p['name'],'version':p['version'],'license':p.get('license'),'repository':p.get('repository'),'files':files})
(notices/'rust-packages.json').write_text(json.dumps(packages,indent=2)+'\n')
