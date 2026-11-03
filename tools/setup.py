#!/usr/bin/env python3
import shutil, subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
for cmd in ('git','gh','gitleaks'):
    if not shutil.which(cmd):raise SystemExit('Missing required command: '+cmd)
subprocess.run(['git','config','--local','core.hooksPath','.githooks'],cwd=root,check=True)
for p in (root/'.githooks').iterdir():p.chmod(0o755)
print('Installed local hooks. Originals remain outside the repository.')
