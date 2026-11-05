#!/usr/bin/env python3
"""Check source ownership and preserved third-party bytes before platform builds."""
from pathlib import Path
import hashlib,json,re,sys
root=Path(__file__).resolve().parents[1];errors=[]
manifest=(root/'Modules/core-sources.cmake').read_text()
listed=re.findall(r'\$\{CMAKE_CURRENT_LIST_DIR\}/\.\./(Sources/Core/[^\s)]+\.cpp)',manifest)
actual={str(p.relative_to(root)) for p in (root/'Sources/Core').rglob('*.cpp')}
if set(listed)!=actual:errors.append(f'Core list mismatch: missing={actual-set(listed)}, stale={set(listed)-actual}')
if len(listed)!=len(set(listed)):errors.append('Duplicate core compilation unit')
for p in (root/'Sources/Core').rglob('*'):
 if p.suffix not in ('.cpp','.hpp'):continue
 for dependency in re.findall(r'#(?:include|import)\s*[<"]([^>"]+)',p.read_text()):
  if any(dependency.startswith(x) for x in ['UIKit/','Foundation/','AVFoundation/','OpenAL/','AL/','OpenGLES/','AudioToolbox/','jni.h','emscripten/']):errors.append(f'Platform dependency in {p.relative_to(root)}: {dependency}')
  if dependency.startswith('Core/') and not (root/'Sources'/dependency).is_file():errors.append(f'Missing include: {dependency}')
lock=json.loads((root/'Modules/ThirdParty/openal-soft-source.json').read_text())
for f in lock['files']:
 p=root/'Modules/ThirdParty/openal-soft'/f['path']
 if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=f['sha256']:errors.append('OpenAL source changed: '+f['path'])
for file in ['Apple/iOS/project.yml','Apple/iOS/MachDefense.xcodeproj/project.pbxproj']:
 if 'OpenAL.framework' in (root/file).read_text():errors.append('Apple OpenAL still linked: '+file)
if errors:sys.exit('\n'.join(errors))
print(f'PASS: {len(actual)} shared core units, no platform headers, {len(lock["files"])} preserved OpenAL source files, no Apple OpenAL link')
