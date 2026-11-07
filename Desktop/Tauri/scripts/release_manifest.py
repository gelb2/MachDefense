#!/usr/bin/env python3
"""Prepare public desktop artifacts after a signed Tauri release build."""
from pathlib import Path
import base64,hashlib,json,re,shutil,subprocess
root=Path(__file__).resolve().parents[3];desktop=root/'Desktop/Tauri'
version=json.loads((desktop/'src-tauri/tauri.conf.json').read_text())['version']
tag='v'+'.'.join(version.split('.')[:2]);out=root/'build/release'/tag;out.mkdir(parents=True,exist_ok=True)
bundle=desktop/'src-tauri/target/release/bundle/macos';source=bundle/'MachDefense Desktop.app.tar.gz'
signature=(bundle/(source.name+'.sig')).read_text().strip()
signed=base64.b64decode(signature,validate=True).decode()
assert re.search(r'\tversion:'+re.escape(version)+r'(?:\s|$)',signed),'Signature version differs from app version'
name=f'MachDefense-Desktop-{tag}-arm64.app.tar.gz';shutil.copy2(source,out/name)
(out/(name+'.sig')).write_text(signature+'\n')
zipname=f'MachDefense-Desktop-{tag}-arm64.zip'
subprocess.run(['ditto','-c','-k','--keepParent',str(bundle/'MachDefense Desktop.app'),str(out/zipname)],check=True)
manifest={'version':version,'notes':'MachDefense desktop update. Progress remains in the app data folder.','platforms':{'darwin-aarch64':{'signature':signature,'url':f'https://github.com/gelb2/MachDefense/releases/download/{tag}/{name}'}}}
(out/'tauri-update.json').write_text(json.dumps(manifest,indent=2)+'\n')
files=[out/name,out/(name+'.sig'),out/zipname,out/'tauri-update.json']
(out/'desktop-SHA256SUMS.txt').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n' for p in files))
print(out)
