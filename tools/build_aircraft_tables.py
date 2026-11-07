#!/usr/bin/env python3
"""Append aircraft rows; preserve every existing original/level10 table byte."""
from pathlib import Path
import math,csv,json
root=Path(__file__).resolve().parents[1]; folder=root/'Resources/Tables'
def read(name):
 lines=(folder/(name+'.tsv')).read_text().splitlines(); keys=[x.strip() for x in lines[0].split('\t')]
 return keys,{r[keys[0]]:r for line in lines[1:] if line.strip() and not line.startswith('//') and (r:=dict(zip(keys,[x.strip() for x in line.split('\t')]))).get(keys[0])}
def append(name, rows):
 p=folder/(name+'.tsv');b=p.read_bytes(); marker=b'// Allied aircraft 2026-11-08';b=b.split(marker)[0];nl=b'\r\n' if b'\r\n' in b else b'\n'
 if not b.endswith(b'\n'):b+=nl
 keys,_=read(name);p.write_bytes(b+marker+nl+b''.join(('\t'.join(str(r.get(k,'-')) for k in keys)).encode()+nl for r in rows))
_,builds=read('MachBuild_Parts');_,items=read('ItemInfo');_,weapons=read('WeaponInfo');_,names=read('MachBuild_Name');_,machines=read('MachList')
# Lowest projectile missiles and original visual/audio definitions remain intact.
bindings=[('ALightGun','E_Turret_Wpn_02',0,1),('ALightMissile','E_Turret17_Wpn_01',0,1),('AHeavyGun','E_Tank_Wpn_13',1,1),('AHeavyMissile','E_Tank28_Wpn_01',1,1),('AHeavyCannon','E_Tank31_Wpn',1,1),('ADuctGun','P_Wpn_HMG_01',2,1),('ADuctMissile','E_Tank28_Wpn_01',2,2),('ATransportGun','E_Tank31_Wpn',3,2),('ATransportMissile','P_Wpn_Msl_206',3,2)]
specs=[('ScoutHeli','AircraftLight','MachineGun',1.5,.70,2.0,8,12),('Gunship','AircraftHeavy','HeavyMissile',2.0,.70,1.0,36,28),('Orca','AircraftDuct','HMG',1.5,.70,2.1,16,14),('OrcaBomber','AircraftTransport','HeavyMissile',1.5,.60,1.9,24,14)]
def ip(code):return items[f'{int(code):05d}']
wr=[];br=[];mr=[];out=[]
for lv in range(1,11):
 reference=builds[f'MachineGun_{lv:02}'];small_range=int(weapons[ip(reference['Weapon'])['strParam']]['ShtRng'])
 # Only production mech/turret weapons, not special attacks/enemy weapons.
 candidates=[]
 for key,b in builds.items():
  if key.endswith(f'_{lv:02}') :candidates.append((int(weapons[ip(b['Weapon'])['strParam']]['ShtRng']),key))
 for key,b in names.items():
  if key.endswith(f'_{lv:02}') and not key.startswith(tuple(s[0] for s in specs)):
   # Named turrets have one matching weapon; table IDs use Wpn prefix.
   mach=b['Turret'];w=weapons.get(mach.replace('Tur_','TurWpn_'))
   if w:candidates.append((int(w['ShtRng']),key))
 # Also explicit turret prefix from actual table naming conventions.
 for key,w in weapons.items():
  if key.startswith('Turret_') and key.endswith(f'_{lv:02}'):candidates.append((int(w['ShtRng']),key))
 longest,maxsource=max(candidates)
 # Same weapon effects, burst count/reload; scale damage to target sustained totals.
 targets=[30+8*(lv-1),260+65*(lv-1),100+25*(lv-1),160+40*(lv-1)]
 ranges=[small_range,longest,round(small_range*1.2),round(small_range*1.35)]
 for k,(name,model,ref,cost_factor,hp_factor,speed,mission,radius) in enumerate(specs):
  b=builds[f'{ref}_{lv:02}']; hp=round(float(ip(b['Armor'])['Param1'])*hp_factor/2)*2
  cost=math.ceil(int(b['Cost'])*cost_factor/5)*5;upgrade=math.ceil(int(b['UpgCost'])*cost_factor/50)*50
  base=sum(int(weapons[src]['Dmg'])*int(weapons[src]['ShtCnt'])/(int(weapons[src]['ShtCnt'])*float(weapons[src]['ShtDly'])+float(weapons[src]['RldDly']))*copies for part,src,kind,copies in bindings if kind==k)
  dps=0;parts=[]
  for part,src,kind,copies in bindings:
   if kind!=k:continue
   w=dict(weapons[src]);w.update(Parts=f'{part}_{lv:02}',Dmg=max(1,round(int(w['Dmg'])*targets[k]/base)),ShtRng=ranges[k],Grd=lv)
   wr.append(w);dps+=int(w['Dmg'])*int(w['ShtCnt'])/(int(w['ShtCnt'])*float(w['ShtDly'])+float(w['RldDly']))*copies
   parts.append(dict(part=part,source=src,damage=w['Dmg'],burst=int(w['ShtCnt']),copies=copies))
  br.append(dict(MachName=f'{name}_{lv:02}',Cost=cost,UpgCost=upgrade,Bld=b['Bld'],Mul=3 if k!=1 else 2,Type='MACH',Bot='-',Turret=f'{model}_{lv:02}'))
  # Gunship cruises at half the Scout Heli speed at the same upgrade level.
  move_speed=round((specs[0][5]+.03*(lv-1))*.5,3) if name=='Gunship' else round(speed+.03*(lv-1),2)
  mr.append(dict(MachName=f'{model}_{lv:02}',HP=hp//2,EXP=0,Spd=move_speed,Type='PLANE',Refer='-',Rad=radius,**{'Coin%':0,'Coin':0,'ItemRegen':'NULL','BaseParts':model}))
  out.append(dict(name=name,level=lv,mission=mission,cells=cost,upgradeCredits=upgrade,hp=hp,speed=mr[-1]['Spd'],range=ranges[k],rangeReference=maxsource if k==1 else ref,dps=round(dps,2),weapons=parts))
append('WeaponInfo',wr);append('MachBuild_Name',br);append('MachList',mr)
append('Description_us',[{'Key':'MN_'+s[0],'Value':{'Gunship':'AH-10','OrcaBomber':'Beluga','Orca':'Cheyenne'}.get(s[0],s[0])} for s in specs]) if read('Description_us')[0][0]=='Key' else None
(root/'Art/Aircraft/runtime/campaign-stats.json').write_text(json.dumps(out,indent=2)+'\n')
print('Appended 40 builds, 40 machines, 90 weapons. Original rows retained.')
print('Heavy maximum range references:', sorted(set((r['rangeReference'],r['range']) for r in out if r['name']=='Gunship')))
