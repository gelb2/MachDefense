#!/usr/bin/env python3
"""Verify Android text adapters and shared Menu restore using a private fixture.
Run with SERIAL CODE_FILE SAVE_FILE. Existing validation save is backed up.
The regular app and its save are untouched. Results stay under .local/.
"""
import hashlib,json,subprocess,sys,time
from pathlib import Path
from device import Device,PACKAGE,ADB

d=Device(sys.argv[1]);code=Path(sys.argv[2]).read_text();expected=Path(sys.argv[3]).read_bytes()
checks=[]
def record(name):
    checks.append(name)
    (d.output/'save-transfer-checks.json').write_text(json.dumps({'checks':checks,'fixtureBytes':len(expected),'fixtureSha256':hashlib.sha256(expected).hexdigest()},indent=2))
def read_save():return d.adb('exec-out','run-as',PACKAGE,'cat','files/MachDefense.sav',raw=True)
def put_text(value):
    subprocess.run([ADB,'-s',d.serial,'shell',f"run-as {PACKAGE} sh -c 'cat > files/transfer-input.txt'"],input=value.encode(),check=True)
    d.command('transfer-input-file');return d.wait(lambda s:s['transferLength']==(len(value.encode()) if len(value.encode())<=120000 else 0))
def action(index):
    s=d.snapshot();buttons=[x for x in s['sprites'] if x['image']=='SysMenu_Btn.png' and x['w']==130]
    b=buttons[index];d.tap(b['x']+b['w']/2,b['y']+b['h']/2,s);d.snapshot()
def open_field(export=False):
    s=d.snapshot();d.press('SysMenu_Btn.png',s,index=0 if export else 1)
    return d.wait(lambda s:s['transferVisible'] and s['transferReadOnly']==export)
def profiles():
    s=d.snapshot()
    if any(x['image']=='Title.jpg' for x in s['sprites']):d.tap(s['width']/2,s['height']/2,s);d.command('ticks',180)
    return d.wait(lambda s:any(x['image']=='Common_Btn_NEW.png' for x in s['sprites']) and not any(x['image'].startswith('Loading_') for x in s['sprites']),timeout=45)

# Wait for APK resources/audio to load before asking the validation receiver.
for attempt in range(20):
    try:d.snapshot();break
    except RuntimeError:
        if attempt==19:raise
        time.sleep(.5)
# Back up before any mutating operation. Never assume an installed app is empty.
before=read_save();backup=d.output/('save-before-transfer-'+str(time.time_ns())+'.sav');backup.write_bytes(before);backup.chmod(0o600)
profiles();open_field();put_text(code);put_text('가'*60000);action(0);d.wait(lambda s:not s['transferVisible']);assert read_save()==before;d.press('Common_Btn_OK.png');record('oversized-utf8-clears-previous-valid-code')
open_field();put_text('MD1-invalid');action(0);d.wait(lambda s:not s['transferVisible']);assert read_save()==before;d.press('Common_Btn_OK.png');record('invalid-code-preserves-save')
open_field();s=put_text(code);generation=s['transferGeneration'];d.command('transfer-stale',generation+100);assert d.snapshot()['transferLength']==len(code);record('stale-text-callback-rejected')
d.command('recreate-activity');d.wait(lambda s:s['transferVisible'] and s['transferLength']==len(code));record('activity-recreation-preserves-input')
d.adb('shell','input','keyevent','KEYCODE_HOME');d.adb('shell','am','start','-n',PACKAGE+'/com.gelb2.machdefense.GameActivity');d.wait(lambda s:s['active'] and s['transferVisible']);assert d.snapshot()['transferLength']==len(code);record('background-preserves-input')
action(0);d.wait(lambda s:not s['transferVisible']);d.press('Common_Btn_NO.png');assert read_save()==before;record('cancel-confirmation-preserves-save')
open_field();put_text(code);action(0);d.wait(lambda s:not s['transferVisible']);d.press('Common_Btn_YES.png');d.wait(lambda s:any(x['image']=='Common_Btn_OK.png' for x in s['sprites']));assert read_save()==expected;record('restore-all-save-bytes')
d.press('Common_Btn_OK.png');open_field(True);action(0);d.command('transfer-copy-probe');assert d.adb('exec-out','run-as',PACKAGE,'cat','files/transfer-copy.txt')==code;record('export-copy-roundtrip')
action(1);d.wait(lambda s:not s['transferVisible']);d.adb('shell','am','force-stop',PACKAGE);d.adb('shell','am','start','-n',PACKAGE+'/com.gelb2.machdefense.GameActivity');time.sleep(2);assert read_save()==expected;profiles();record('process-restart-preserves-save')
s=d.snapshot('save-transfer-restored');d.screenshot('save-transfer-restored');assert not s['transferVisible'] and not s['audioError']
d.adb('shell','run-as',PACKAGE,'rm','-f','files/transfer-input.txt','files/transfer-copy.txt');d.adb('shell','am','force-stop',PACKAGE);d.adb('shell','input','keyevent','KEYCODE_HOME')
print(json.dumps({'passed':checks,'backup':str(backup)}))
