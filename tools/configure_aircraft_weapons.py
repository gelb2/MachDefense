#!/usr/bin/env python3
"""Apply selected original weapon behavior to helicopter-only table rows.
All original rows retain their exact bytes. Optional measured rates preserve the
accepted total firepower while distributing it across the requested loadout.
"""
import argparse,csv,math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
E=ROOT/'docs/evidence/aircraft-implementation-20261109'
p=argparse.ArgumentParser();p.add_argument('--rates',type=Path);a=p.parse_args()
def rows(path):
 return list(csv.DictReader(path.open(),delimiter='\t'))
sources={(n,int(lv)):w for n,lv,w in csv.reader((E/'weapon-sources.tsv').open(),delimiter='\t')}
units={(r['unit'],int(r['level'])):r for r in rows(ROOT/'docs/evidence/aircraft-balance-20261108/units.tsv')}
path=ROOT/'Resources/Tables/WeaponInfo.tsv';raw=path.read_bytes().splitlines(keepends=True)
keys=[s.strip() for s in raw[0].decode().split('\t')]
original={r[0].strip():dict(zip(keys,[v.strip() for v in r])) for r in csv.reader(path.open(),delimiter='\t') if len(r)==len(keys)}
# unit, family, original source, count, share of unit's ideal DPS
bindings=[('Scout Heli','ALightGun','MachineGun',1,.6),('Scout Heli','ALightMissile','Missile',1,.4),
 ('Cheyenne','ADuctMissile','Missile',1,4/7),('Cheyenne','ADuctMissileR','Missile',1,3/7),
 ('Beluga','ATransportGun','HMG',2,.6),('Beluga','ATransportMissile','MissileTurret',2,.4),
 ('AH-10','AHeavyGun','Vulcan',1,.3),('AH-10','AHeavyMissile','Missile',1,.15),
 ('AH-10','AHeavyRocket','E_Turret17_Wpn_01',1,.15),('AH-10','AHeavyCannon','E_Tank31_Wpn',1,.25),
 ('AH-10','AHeavyAirStrike','AirStrike-Missile',1,.15)]
rates={r['weapon']:float(r['shotsPerSecond']) for r in rows(a.rates)} if a.rates else {}
output=[];mapping=[]
for unit,family,source,count,share in bindings:
 for lv in range(1,11):
  name=f'{family}_{lv:02d}';src=sources.get((source,lv),source);row=dict(original[src]);row['Parts']=name;row['Grd']=str(lv)
  existing=original.get(name)
  if existing is None and family=='ADuctMissileR':existing=original.get(f'ADuctMissile_{lv:02d}')
  row['ShtRng']=existing['ShtRng'] if existing else units[unit,lv]['rawRange']
  if family.startswith('ADuctMissile'):row.update(ShtCnt='3' if family.endswith('R') else '4',ShtDly='0',RldDly='2.5',AimRt='0')
  if family=='AHeavyCannon':row['Speed']='0.9' # 3x the original enemy cannon, actual projectile not hitscan.
  row['Dmg']=str(max(1,math.floor(float(units[unit,lv]['targetIdealDps'])*share/(rates[name]*count)+.5))) if rates else '1'
  output.append(('\t'.join(row[k] for k in keys)+'\n').encode())
  mapping.append([unit,lv,name,src,count,share,row['Dmg'],row['ShtRng'],row['FireSound'],row['BltTile'],row['EffTile']])
prefixes=(b'ALightGun',b'ALightMissile',b'AHeavyGun',b'AHeavyMissile',b'AHeavyCannon',b'AHeavyRocket',b'AHeavyAirStrike',b'ADuctGun',b'ADuctMissile',b'ATransportGun',b'ATransportMissile')
retained=[line for line in raw if not line.split(b'\t')[0].startswith(prefixes)]
path.write_bytes(b''.join(retained+output))
with (E/'weapon-bindings.tsv').open('w') as f:
 writer=csv.writer(f,delimiter='\t');writer.writerow(['unit','level','weapon','source','count','dpsShare','damage','rawRange','sound','bullet','effect']);writer.writerows(mapping)
print(f'{len(output)} helicopter weapon rows; original rows retained')
