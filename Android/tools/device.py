#!/usr/bin/env python3
import json, subprocess, time
from pathlib import Path
ADB=str(Path.home()/'Library/Android/sdk/platform-tools/adb')
PACKAGE='com.gelb2.machdefense.validation'
class Device:
    def __init__(self,serial):
        self.serial=serial;self.root=Path(__file__).resolve().parents[2];self.output=self.root/'.local'/('app-'+serial);self.output.mkdir(exist_ok=True)
    def adb(self,*args,check=True,raw=False):
        r=subprocess.run([ADB,'-s',self.serial,*map(str,args)],capture_output=True,text=not raw,check=check)
        return r.stdout
    def command(self,name,a=0,b=0):
        self.adb('shell','am','broadcast','-n',PACKAGE+'/com.gelb2.machdefense.TestReceiver','--es','command',name,'--ei','a',a,'--ei','b',b)
    def snapshot(self,label=None):
        self.adb('shell','run-as',PACKAGE,'rm','-f','files/validation.json')
        self.command('snapshot')
        for _ in range(20):
            text=self.adb('exec-out','run-as',PACKAGE,'cat','files/validation.json',check=False)
            try:
                state=json.loads(text)
                assert not state['error'],state['error']
                assert not state['audioError'],state['audioError']
                if label:(self.output/(label+'.json')).write_text(json.dumps(state,indent=2))
                return state
            except json.JSONDecodeError:time.sleep(.05)
        raise RuntimeError(text)
    def screenshot(self,label):
        (self.output/(label+'.png')).write_bytes(self.adb('exec-out','screencap','-p',raw=True))
    def tap(self,x,y,state=None):
        s=state or self.snapshot();self.adb('shell','input','tap',round(x*s['pixelWidth']/s['width']),round(y*s['pixelHeight']/s['height']))
    def sprite(self,needle,s=None,index=-1):
        s=s or self.snapshot();rows=[x for x in s['sprites'] if needle in x['image']]
        if not rows:raise RuntimeError('Missing sprite '+needle)
        return rows[index]
    def press(self,needle,s=None,index=-1):
        s=s or self.snapshot();x=self.sprite(needle,s,index);self.tap(x['x']+x['w']/2,x['y']+x['h']/2,s)
    def wait(self,predicate,timeout=15):
        end=time.monotonic()+timeout
        while time.monotonic()<end:
            s=self.snapshot()
            if predicate(s):return s
            time.sleep(.15)
        raise RuntimeError('Timed out: '+str({k:v for k,v in s.items() if k!='sprites'}))
if __name__=='__main__':
    import sys
    d=Device(sys.argv[1]);s=d.snapshot('snapshot');print(json.dumps({k:v for k,v in s.items() if k!='sprites'},indent=2))
    print('\n'.join(f"{x['image']}: {x['x']},{x['y']} {x['w']}x{x['h']}" for x in s['sprites'] if not x['image'].startswith('Generated/')))
