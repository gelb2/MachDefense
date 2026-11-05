#!/usr/bin/env python3
"""Package prepared resources; never read the compatibility archive or an IPA."""
import hashlib, json, shutil, sys
from pathlib import Path
root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
manifest = {}
for folder in ('Images', 'Audio', 'RawData', 'Tables', 'Generated'):
    for source in sorted((root / 'Resources' / folder).rglob('*')):
        if not source.is_file() or source.name.startswith('.') or source.suffix.lower() in ('.md', '.py'): continue
        relative = source.relative_to(root / 'Resources')
        data = source.read_bytes()
        if data.startswith(b'version https://git-lfs.github.com/spec/v1'): raise RuntimeError(f'Missing LFS content: {relative}')
        target = out / relative; target.parent.mkdir(parents=True, exist_ok=True)
        if not target.exists() or target.read_bytes() != data: shutil.copy2(source, target)
        manifest[str(relative)] = hashlib.sha256(data).hexdigest()
for target in out.rglob('*'):
    if target.is_file() and str(target.relative_to(out)) not in manifest and target.name != 'resource-manifest.json': target.unlink()
(out / 'resource-manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
print(f'Prepared {len(manifest)} resources')
