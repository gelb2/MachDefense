#!/usr/bin/env python3
"""Render each existing mission through the cheat APK without changing normal saves."""
import json,sys,time
from device import Device,PACKAGE
d=Device(sys.argv[1]);d.adb('shell','am','force-stop',PACKAGE);d.adb('shell','am','start','-n',PACKAGE+'/com.gelb2.machdefense.GameActivity');time.sleep(2)
rows=[]
for stage in range(80):
 d.command('validation',6);d.command('ticks',200);s=d.snapshot()
 if any('iPon_HelpScr' in x['image'] for x in s['sprites']):
  h=d.sprite('iPon_HelpScr',s);d.tap(160,h['y']+450,s)
 d.command('ticks',40);s=d.snapshot();assert s['world']['stage']==stage and s['world']['state']==0,(stage,s.get('world'))
 rows.append({'stage':stage+1,'frames':s['frames'],'uploads':s['uploads'],'sprites':len(s['sprites'])})
 if stage in [0,31,79]:d.screenshot('campaign-'+str(stage+1))
 if stage<79:d.command('validation',5)
 (d.output/'campaign-render.json').write_text(json.dumps(rows,indent=2))
print('Rendered',len(rows),'missions without native/render errors')
# Verify real Activity destruction/recreation retains the same game and pause menu.
s=d.snapshot();context=s['contexts'];stage=s['world']['stage'];writes=s['writes'];d.command('recreate-activity');time.sleep(1.5);s=d.snapshot('activity-recreated');d.screenshot('activity-recreated')
assert s['contexts']>context and s['world']['stage']==stage and s['world']['paused'] and s['writes']==writes
print('Activity recreation retained world, opened pause menu, wrote no save')
d.adb('shell','am','force-stop',PACKAGE);d.adb('shell','input','keyevent','KEYCODE_HOME')
