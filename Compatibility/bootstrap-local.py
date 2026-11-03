#!/usr/bin/env python3
"""Prepare an isolated local runtime from the verified HeavyMach compatibility source."""
import argparse,hashlib,json,shutil,subprocess
from pathlib import Path
repo=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--heavymach-compatibility',type=Path,required=True)
p.add_argument('--build-root',type=Path,required=True)
p.add_argument('--originals-root',type=Path,required=True)
a=p.parse_args();hm=a.heavymach_compatibility.resolve();build=a.build_root.resolve();originals=a.originals_root.resolve()
for d in (build,originals):
 if d==repo or repo in d.parents:raise SystemExit('Build/original paths must be outside this repository')
up=json.loads((repo/'Compatibility/upstream.json').read_text())
checkout=hm/'compat-source'
head=subprocess.check_output(['git','-C',str(checkout),'rev-parse','HEAD'],text=True).strip()
if head!=up['commit']:raise SystemExit('Reference runtime commit mismatch')
subprocess.run(['git','-C',str(checkout),'apply','--reverse','--check',str(repo/'Compatibility/patches/heavymach.patch')],check=True)
reference=checkout/up['source']
sdl=reference/'vendor/rust-sdl2'
if subprocess.check_output(['git','-C',str(sdl),'rev-parse','HEAD'],text=True).strip()!=up['sdl_commit']:raise SystemExit('SDL commit mismatch')
subprocess.run(['git','-C',str(sdl),'apply','--reverse','--check',str(repo/'Compatibility/patches/rust-sdl2.patch')],check=True)
ipa=originals/'Game IPAs/Mach Defense/Mach_Defense v1.0.5 - (kr.indieapps.MachDefense).ipa'
expected=up['original']['sha256']
with ipa.open('rb') as f:
 if hashlib.file_digest(f,'sha256').hexdigest()!=expected:raise SystemExit('Original SHA mismatch')
source=build/'touchHLE'
if source.exists():raise SystemExit('Existing runtime preserved; select a new build root or inspect it')
build.mkdir(parents=True,exist_ok=True)
shutil.copytree(reference,source,ignore=shutil.ignore_patterns('target','.git','touchHLE_apps'),symlinks=False)
(source/'touchHLE_apps').mkdir()
shutil.copy2(reference/'touchHLE_apps/README.txt',source/'touchHLE_apps/README.txt')
subprocess.run(['git','apply',str(repo/'Compatibility/patches/machdefense.patch')],cwd=source,check=True)
config={'build_root':str(build),'runtime_source':str(source),'cargo_home':str((hm/'toolchain/cargo').resolve()),'rustup_home':str((hm/'toolchain/rustup').resolve()),'original':str(ipa),'sha256':expected}
(repo/'.local').mkdir(exist_ok=True)
(repo/'.local/compatibility.json').write_text(json.dumps(config,indent=2)+'\n')
print('Prepared local runtime; original remains outside the repository')
