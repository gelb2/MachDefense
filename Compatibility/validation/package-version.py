#!/usr/bin/env python3
"""Create a separate simulator comparison app from an existing local wrapper.

All original packages, executables and resulting apps stay outside the repository.
The existing ordinary/cheat app and its save container are not changed.
"""
import argparse
import hashlib
import plistlib
import shutil
import subprocess
from pathlib import Path

VARIANTS = {
    'indie105': ('Game IPAs/Mach Defense/Mach_Defense v1.0.5 - (kr.indieapps.MachDefense).ipa',
                 '2c3e9aba0d249827da95c557f905762e3e2df08f79602f5404f93aa8f4c3d052'),
    'wemade103': ('Game IPAs/Heavy MACH: Defense/MachDefense v1.0.3 - (com.wemadeent.HeavyMachDefence).ipa',
                  '882fe8efa9a0c3d51c23da190ee3d30376a1b6766e16cc6b87daf09b98dc92b1'),
}
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--variant', choices=VARIANTS, required=True)
p.add_argument('--originals-root', type=Path, required=True)
p.add_argument('--wrapper', type=Path, required=True)
p.add_argument('--runtime', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
original, expected = VARIANTS[a.variant]
ipa = (a.originals_root/original).resolve()
app = a.output.resolve()
for path in (ipa, app, a.runtime.resolve(), a.wrapper.resolve()):
    if path == root or root in path.parents:
        p.error('Originals, runtime and app bundles must stay outside the repository')
if app.exists(): p.error('Output exists; preserve it and select a new output')
if not a.runtime.is_file(): p.error('Runtime executable does not exist')
if not (a.wrapper/'Info.plist').is_file(): p.error('Wrapper Info.plist does not exist')
if hashlib.sha256(ipa.read_bytes()).hexdigest() != expected: p.error('Original hash mismatch')
info = plistlib.loads((a.wrapper/'Info.plist').read_bytes())
if info.get('DTPlatformName') != 'iphonesimulator': p.error('Requires a simulator wrapper')
info['CFBundleIdentifier'] = 'com.gelb2.machdefense.audit.' + a.variant
info['CFBundleDisplayName'] = 'MD Audit ' + a.variant
shutil.copytree(a.wrapper, app)
shutil.copy2(a.runtime, app/info['CFBundleExecutable'])
shutil.copy2(ipa, app/'MachDefense.ipa')
(app/'Info.plist').write_bytes(plistlib.dumps(info))
(app/'MachDefense_options.txt').write_text('--device-family=iphone\n--preferred-languages=en\n')
subprocess.run(['codesign', '--force', '--sign', '-', str(app)], check=True)
print(info['CFBundleIdentifier'])
print(app)
