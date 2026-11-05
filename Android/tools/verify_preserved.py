#!/usr/bin/env python3
"""Check the exact Apple/shared files recorded before Android development."""
import hashlib,json
from pathlib import Path
root=Path(__file__).resolve().parents[2]
manifest=json.loads((root/'Android/protected-baseline.json').read_text())
changed=[]
for item in manifest['files']:
 p=root/item['path']
 if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=item['sha256']:changed.append(item['path'])
if changed:raise SystemExit('Changed protected files:\n'+'\n'.join(changed))
print(f"PASS: {len(manifest['files'])} Apple/shared/resource/test files unchanged from {manifest['base']}")
