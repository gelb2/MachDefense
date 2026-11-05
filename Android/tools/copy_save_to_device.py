#!/usr/bin/env python3
"""Copy an explicit save into a stopped Android app, backing up its old save first."""
import argparse,hashlib,json,subprocess,time
from pathlib import Path
from device import ADB
p=argparse.ArgumentParser();p.add_argument('--serial',required=True);p.add_argument('--package',choices=['com.gelb2.machdefense.restored','com.gelb2.machdefense.validation'],required=True);p.add_argument('--save',type=Path,required=True);p.add_argument('--backup-dir',type=Path,required=True);a=p.parse_args()
def adb(*args,input=None,check=True):return subprocess.run([ADB,'-s',a.serial,*args],input=input,capture_output=True,check=check)
inspector=Path(__file__).resolve().parents[2]/'.local/save-inspect'
if not inspector.is_file():raise RuntimeError('Build Android/tools/save_inspect.cpp with Sources/Core/Session/SaveData.cpp first; see Android/README.md')
subprocess.run([str(inspector),str(a.save)],capture_output=True,check=True)
data=a.save.read_bytes()
if len(data)!=42496:raise ValueError('Expected MachDefense save: 42496 bytes')
a.backup_dir.mkdir(parents=True,exist_ok=True);stamp=str(time.time_ns())
adb('shell','am','force-stop',a.package)
old=adb('exec-out','run-as',a.package,'cat','files/MachDefense.sav',check=False)
backup=None
if old.returncode==0:
 backup=a.backup_dir/(stamp+'-previous.sav');backup.write_bytes(old.stdout);backup.chmod(0o600)
elif b'No such file' not in old.stderr+old.stdout:raise RuntimeError((old.stderr+old.stdout).decode())
adb('shell','run-as',a.package,'mkdir','-p','files')
adb('shell','run-as',a.package,'sh','-c',"'cat > files/MachDefense.transfer.tmp'",input=data)
adb('shell','run-as',a.package,'mv','files/MachDefense.transfer.tmp','files/MachDefense.sav')
received=adb('exec-out','run-as',a.package,'cat','files/MachDefense.sav').stdout
assert received==data,'Readback mismatch'
report={'package':a.package,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'previousBackup':str(backup) if backup else None,'exactReadback':True}
(a.backup_dir/(stamp+'-transfer.json')).write_text(json.dumps(report,indent=2));print(json.dumps(report))
