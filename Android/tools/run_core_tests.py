#!/usr/bin/env python3
"""Run the unchanged core test executables on one explicit Android device."""
import argparse, json, re, subprocess, time, tarfile
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--serial',required=True);args=p.parse_args()
root=Path(__file__).resolve().parents[2];sdk=Path.home()/'Library/Android/sdk';adb=str(sdk/'platform-tools/adb')
local=root/'.local/ndk-tests';remote='/data/local/tmp/machdefense-tests-20261106'
out=root/'.local'/('core-'+args.serial);out.mkdir(exist_ok=True)
def call(*cmd,check=True):return subprocess.run([adb,'-s',args.serial,*cmd],capture_output=True,text=True,check=check)
tests=[]
for line in (local/'CTestTestfile.cmake').read_text().splitlines():
    m=re.match(r'add_test\(\[=\[(.*?)\]=\] (.*)\)$',line)
    if m:tests.append((m[1],re.findall(r'"([^"]*)"',m[2])))
archive=root/'.local/core-test-payload.tar'
with tarfile.open(archive,'w') as tar:
    for name,params in tests:tar.add(params[0],arcname='bin/'+Path(params[0]).name)
    for folder in ('Tables','RawData','Images','Generated'):tar.add(root/'Resources'/folder,arcname='Resources/'+folder)
call('shell','mkdir','-p',remote);call('push',str(archive),remote+'/payload.tar')
call('shell','tar','xf',remote+'/payload.tar','-C',remote)
results=[]
for name,params in tests:
    command=[remote+'/bin/'+Path(params[0]).name]+[s.replace(str(root/'Resources'),remote+'/Resources') for s in params[1:]]
    start=time.monotonic();r=call('shell',*command,check=False);elapsed=time.monotonic()-start
    (out/(name+'.log')).write_text(r.stdout+r.stderr)
    row={'name':name,'exit':r.returncode,'seconds':round(elapsed,3)};results.append(row)
    print(json.dumps(row),flush=True)
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
raise SystemExit(any(r['exit'] for r in results))
