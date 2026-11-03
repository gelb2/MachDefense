#!/usr/bin/env python3
import argparse, hashlib, json, re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('action',choices=['check','plan'])
p.add_argument('--task',default='restoration')
a=p.parse_args()
m=json.loads((root/'docs/document-map.json').read_text())
if a.action=='plan':
    if a.task not in m['domains']:raise SystemExit('Tasks: '+', '.join(m['domains']))
    print('\n'.join(dict.fromkeys(m['base_read']+m['domains'][a.task])))
else:
    errors=[]
    for name in m['base_read']+[x for values in m['domains'].values() for x in values]:
        if not (root/name).is_file():errors.append('Missing route: '+name)
    for item in json.loads((root/'docs/skill-source-manifest.json').read_text())['files']:
        path=root/item['destination']
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest()!=item['sha256']:errors.append('Source copy mismatch: '+str(path))
    # Historical copies keep their original paths; check maintained entry documents only.
    for path in [root/'README.md',root/'AGENTS.md',root/'HANDOVER-CURRENT.md',*list((root/'docs').glob('*.md'))]:
        for link in re.findall(r'\]\(([^)]+)\)',path.read_text()):
            target=link.split('#')[0]
            if target and '://' not in target and not (path.parent/target).exists():errors.append('Broken link: '+str(path.relative_to(root))+': '+link)
    if errors:raise SystemExit('\n'.join(errors))
    print('PASS: document routes, maintained links and source-copy hashes')
