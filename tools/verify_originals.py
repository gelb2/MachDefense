#!/usr/bin/env python3
import argparse, hashlib, json
from pathlib import Path
repo=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--root',type=Path)
a=p.parse_args()
root=(a.root or Path(json.loads((repo/'.local/originals.json').read_text())['root'])).resolve()
if root==repo or repo in root.parents:raise SystemExit('Original root must be outside the repository')
manifest=json.loads((repo/'docs/originals-manifest.json').read_text())
for f in manifest['files']:
    path=(root/f['name']).resolve()
    if root not in path.parents:raise SystemExit('Original path escapes root')
    with path.open('rb') as stream:digest=hashlib.file_digest(stream,'sha256').hexdigest()
    if path.stat().st_size!=f['bytes'] or digest!=f['sha256']:raise SystemExit('Original mismatch: '+f['name'])
print('PASS:',len(manifest['files']),'external originals match SHA-256 and size; read only')
