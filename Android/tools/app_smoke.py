#!/usr/bin/env python3
"""ADB touches use the rendered shared-game sprites; cheats only prepare state."""
import sys,time,json
from device import Device,PACKAGE

d=Device(sys.argv[1]);checks=[]
def record(name):
 s=d.snapshot(name);d.screenshot(name);checks.append({'name':name,**{k:v for k,v in s.items() if k!='sprites'}});(d.output/'app-checks.json').write_text(json.dumps(checks,indent=2));return s
def has(s,n):return any(n in x['image'] for x in s['sprites'])
def resume():
 s=d.snapshot()
 if has(s,'iPon_HelpScr'):
  h=d.sprite('iPon_HelpScr',s);d.tap(160,h['y']+450,s)
 s=d.snapshot()
 if s.get('world',{}).get('paused'):
  rows=[x for x in s['sprites'] if x['image']=='SysMenu_Btn.png' and x['w']>100]
  if rows:d.tap(130,rows[0]['y']+16,s)
 return d.snapshot()

s=resume();assert 'world' in s and not s['world']['paused']
d.command('validation',12);d.press('Btn_BuildMachBG',index=0);d.command('ticks',240)
s=record('production-confirmed');assert s['world']['produced']>=1
# Real menu touch and pause time, followed by real resume.
d.press('iPon_QuickBtn',index=0);s=d.snapshot();assert s['world']['paused'];t=s['world']['time'];d.command('ticks',50);assert d.snapshot()['world']['time']==t
resume();s=record('pause-resume');assert not s['world']['paused']
# Base and equipment dialogs, from ordinary controls.
d.press('iPon_QuickBtn',index=1);time.sleep(1);s=record('base-upgrade');assert has(s,'Btn_BaseUpgrade_BaseCannon') and s['world']['paused']
d.press('Btn_UpgradeBase_Close');time.sleep(1);assert not d.snapshot()['world']['paused']
d.press('Btn_UpgradeEquip');s=record('unit-upgrade');assert has(s,'CommonUI_Upgrade') and s['world']['paused']
d.press('Btn_UpgradeEquip');assert not d.snapshot()['world']['paused']
# Pause/restart sound backend, then force the EGL context to be replaced.
for i in range(5):
 d.command('focus',0);s=d.snapshot();t=s['world']['time'];d.command('ticks',3);assert d.snapshot()['world']['time']==t
 d.command('focus',1);d.command('audio');probe=d.snapshot();assert probe['probeSeconds']>0,probe['probeSeconds'];resume()
record('audio-focus-five-cycles')
s=d.snapshot();c=s['contexts'];w=s['writes'];d.command('context-cycle');s=record('egl-context-recreated');assert s['contexts']==c+1 and s['writes']==w and 'world' in s
# Android background/foreground recreates Surface and keeps the menu paused.
for i in range(3):
 d.adb('shell','input','keyevent','KEYCODE_HOME');time.sleep(.3);s=d.snapshot();assert not s['active'];t=s['world']['time']
 d.adb('shell','am','start','-n',PACKAGE+'/com.gelb2.machdefense.GameActivity');time.sleep(.6);s=d.snapshot();assert s['world']['paused'] and s['world']['time']==t
 d.command('audio');assert d.snapshot()['probeSeconds']>0;resume()
record('background-three-cycles')
# Real resupply control after its simulation-time intervals.
d.command('ticks',3800);s=resume();assert s['world']['state']==0
# Bottom-left game controls use the same bottom offset as the equipment button.
s=d.snapshot();quick=d.sprite('iPon_QuickBtn',s,index=0);bottom=quick['y']-430
d.tap(208.5,443+bottom,s);s=record('resupply');assert has(s,'Common_Btn_CLOSE')
rows=[x for x in s['sprites'] if x['image']=='SysMenu_Btn.png' and x['w']>100]
for row in rows[:3]:d.tap(row['x']+row['w']/2,row['y']+row['h']/2)
d.press('Common_Btn_CLOSE');record('resupply-claimed')
# Force result through the existing cheat, press the ordinary Next/Retry controls.
d.command('validation',409);time.sleep(.2);s=record('victory');assert s['world']['state']!=0 and has(s,'SummaryBtn_NextStage')
stage=s['world']['stage'];d.press('SummaryBtn_NextStage');d.command('ticks',250);s=d.snapshot();assert s['world']['stage']==stage+1
resume();record('next-mission');d.command('validation',410);time.sleep(.2);s=record('defeat');assert has(s,'SummaryBtn_Retry')
d.press('SummaryBtn_Retry');d.command('ticks',250);s=resume();assert s['world']['state']==0 and s['world']['stage']==stage+1
record('retry');print(json.dumps({'device':d.serial,'checks':len(checks),'passed':[c['name'] for c in checks]}))
