#!/usr/bin/env python3
"""Private remote and protected branch checks. Called before Git transfers objects."""
import json, subprocess, sys
from original_guard import ROOT, git


def main():
    expected = ('git@github.com:gelb2/MachDefense.git', 'https://github.com/gelb2/MachDefense.git')
    if len(sys.argv) != 3 or sys.argv[2] not in expected:
        raise SystemExit('BLOCKED: unexpected push destination')
    info = json.loads(subprocess.check_output(['gh','repo','view','gelb2/MachDefense','--json','visibility'],cwd=ROOT))
    if info['visibility'] != 'PRIVATE':
        raise SystemExit('BLOCKED: repository must remain private')
    for line in sys.stdin:
        _, sha, remote_ref, _ = line.split()
        if set(sha) == {'0'}:
            continue
        subprocess.run([sys.executable,str(ROOT/'tools/original_guard.py'),'--history','--revision',sha],cwd=ROOT,check=True)
        if remote_ref == 'refs/heads/main':
            parents = git('rev-list','--parents','-n','1',sha).decode().split()[1:]
            dev = git('rev-parse','develop').decode().strip()
            if len(parents) != 2 or parents[1] != dev:
                raise SystemExit('BLOCKED: main must merge current develop')
    print('PASS: private destination and branch flow')


if __name__ == '__main__':
    main()
