#!/usr/bin/env python3
"""Package the original only into an external local device/simulator build."""
import hashlib,json,os,shutil,sys,zipfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
c=json.loads((root/'.local/compatibility.json').read_text())
build=Path(c['build_root']).resolve();src=Path(c['runtime_source']).resolve();ipa=Path(c['original']).resolve()
for p in [build,ipa]:
 if p==root or root in p.parents:raise SystemExit('Original and packaged build must remain outside Git repository')
if hashlib.sha256(ipa.read_bytes()).hexdigest()!=c['sha256']:raise SystemExit('Original SHA-256 mismatch')
sim=os.environ.get('PLATFORM_NAME')=='iphonesimulator'
target='aarch64-apple-ios-sim' if sim else 'aarch64-apple-ios'
app=(Path(os.environ['TARGET_BUILD_DIR'])/os.environ['WRAPPER_NAME']).resolve()
if root in app.parents:raise SystemExit('Build output inside Git repository is forbidden for the compatibility app')
app.mkdir(parents=True,exist_ok=True)
shutil.copy2(build/'target'/target/'release/touchHLE',app/'MachDefenseCompatibility')
os.utime(app/'MachDefenseCompatibility',None)
shutil.copy2(ipa,app/'MachDefense.ipa')
shutil.copy2(root/'Compatibility/MachDefense_options.txt',app/'MachDefense_options.txt')
shutil.copytree(src/'touchHLE_fonts',app/'touchHLE_fonts',dirs_exist_ok=True)
with zipfile.ZipFile(app/'touchHLE_dylibs.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in (src/'touchHLE_dylibs').iterdir():
  if p.is_file():z.write(p,p.name)
shutil.copy2(src/'touchHLE_default_options.txt',app/'touchHLE_default_options.txt')
print('Packaged local compatibility app:',app)
