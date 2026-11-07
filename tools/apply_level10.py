#!/usr/bin/env python3
"""Apply the reviewed level rows; preserve original rows and original MCB files.

Run after prepare_resources.py when rebuilding prepared resources. Input is the
committed review proposal, not an IPA. Repeated execution is deterministic.
"""
from pathlib import Path
import copy, hashlib, json, struct
ROOT=Path(__file__).resolve().parents[1]
proposal=json.loads((ROOT/'docs/research/level10/proposal.json').read_text())
class Table:
 def __init__(self,name):
  self.path=ROOT/'Resources/Tables'/f'{name}.tsv'
  self.raw=self.path.read_bytes().decode();self.lines=self.raw.splitlines(keepends=True)
  self.keys=[x.strip() for x in self.lines[0].strip().split('\t')]
  self.rows={}
  for i,line in enumerate(self.lines[1:],1):
   if not line.strip() or line.startswith('//'):continue
   values=[x.strip() for x in line.rstrip('\r\n').split('\t')]
   self.rows[values[0]]=(i,dict(zip(self.keys,values)))
 def get(self,key):return self.rows[key][1].copy()
 def put(self,row):
  key=row[self.keys[0]]
  line='\t'.join(str(row[k]) for k in self.keys)+'\n'
  if key in self.rows:
   i,old=self.rows[key]
   if old==row:return
   self.lines[i]=line;self.rows[key]=(i,row)
  else:
   if self.lines and not self.lines[-1].endswith('\n'):self.lines[-1]+='\n'
   self.rows[key]=(len(self.lines),row);self.lines.append(line)
 def save(self):self.path.write_bytes(''.join(self.lines).encode())
t={n:Table(n) for n in ['WeaponInfo','MachBuild_Name','MachList','BaseUpgrade','SpAttackList','ItemInfo','PartsList','TileList']}
configs={'RifleTurret':('Tur_Gun','Turret_gun'),'MissileTurret':('Tur_Msl','Turret_msl'),'Vulcan':('Tur_MG','Turret_mg'),'IonCannon':('Tur_Ion','Turret_ion'),'LaserTurret':('Tur_Lsr','Turret_lsr'),'PlasmaCannon':('Tur_Pla','Turret_pla')}
special={'AirStrike-Bomb':(5,2000),'AirStrike-Missile':(5,2010),'AirStrike-Nuclear':(5,2020),'AirStrike-EMP':(4,2030),'Crossfire-Missile':(8,2100)}
for entry in proposal['weapons']:
 if entry['source']=='proposal':t['WeaponInfo'].put(entry['fields'])
for r in proposal['rows']:
 n,l=r['object'],r['level'];key=f'{n}_{l:02d}'
 if n in configs:
  cap=5;mach,_=configs[n]
  if l<cap and n!='LaserTurret':continue
  row=t['MachBuild_Name'].get(f'{n}_{min(l,cap):02d}');row.update(MachName=key,UpgCost=str(r['upgrade_to_next']))
  if n=='LaserTurret':row['Cost']=str(r['purchase_cost'])
  if l>cap:
   row.update(Cost=str(r['purchase_cost']),Turret=f'{mach}_{l:02d}')
   m=t['MachList'].get(f'{mach}_{cap:02d}');m.update(MachName=f'{mach}_{l:02d}',HP=str(r['hp']//2));t['MachList'].put(m)
  t['MachBuild_Name'].put(row)
 elif n.startswith('Base'):
  if l<7:continue
  row=t['BaseUpgrade'].get(f'{n}_{min(l,7):02d}');row.update(UpgradeName=key,UpgCost=str(r['upgrade_to_next']))
  if n=='BaseDefense':row['Param']=str(r['hp'])
  t['BaseUpgrade'].put(row)
 else:
  cap,idbase=special[n]
  if l<=cap:continue
  row=t['SpAttackList'].get(f'{n}_{cap:02d}');row.update(AttackName=key,Cost=str(r['purchase_cost']),UpgCost=str(r['upgrade_to_next']),ItemId=f'{idbase+l:05d}');t['SpAttackList'].put(row)
  item=t['ItemInfo'].get(f'{idbase+cap:05d}');item.update(Code=f'{idbase+l:05d}',Param1=str(l));t['ItemInfo'].put(item)
def original(name):
 root=json.loads((ROOT/f'Resources/Data/{name}.json').read_text())['root']
 return {bytes.fromhex(x['data_hex'])[:24].split(b'\0')[0].decode():x for x in root['children']}
parts=original('PartsInfo');machines=original('MachInfo');newparts=[];newmachines=[]
def rename(node,mapping):
 node=copy.deepcopy(node);b=bytes.fromhex(node['data_hex']);old=b[:24].split(b'\0')[0].decode()
 if old in mapping:
  name=mapping[old].encode();assert len(name)<24;b=name.ljust(24,b'\0')+b[24:];node['data_hex']=b.hex()
 node['children']=[rename(c,mapping) for c in node['children']];return node
def alias(old,new):
 newparts.append(rename(parts[old],{old:new}))
 assert (ROOT/f'Resources/Images/{new}.png').is_file(), f'Pack approved art first: {new}'
 row=t['PartsList'].get(old);row.update(PartsName=new,No='0',TileName=new);t['PartsList'].put(row)
 # Copied parts retain their original socket/type data; only the image lookup
 # uses the existing table alias path. These max-level images are single tiles.
 assert bytes.fromhex(parts[old]['data_hex'])[25]==1
 t['TileList'].put({'TileName':new+'.png','X':'1','Y':'1'})
for l in range(6,11):alias('Turret_b_05',f'Turret_b_{l:02d}')
for name,(mach,weapon) in configs.items():
 for l in range(6,11):
  old,new=f'{weapon}_05',f'{weapon}_{l:02d}';alias(old,new)
  newmachines.append(rename(machines[f'{mach}_05'],{f'{mach}_05':f'{mach}_{l:02d}',old:new,'Turret_b_05':f'Turret_b_{l:02d}'}))
for l in range(8,11):
 for old,new in [('Base07',f'Base{l:02d}'),('BaseWpn07',f'BaseWpn{l:02d}'),('Base07_Rader',f'Base{l:02d}_Rader')]:alias(old,new)
 newmachines.append(rename(machines['Base07'],{'Base07':f'Base{l:02d}'}))
def encoded(n):
 data=bytes.fromhex(n['data_hex']);children=b''.join(encoded(c) for c in n['children']);size=16+len(data)+len(children)
 return struct.pack('<III',size,n['type'],len(data))+data+struct.pack('<I',len(n['children']))+children
for name,kind,nodes in [('Level10Parts',0,newparts),('Level10Machines',2,newmachines)]:
 (ROOT/f'Resources/RawData/{name}.mcb').write_bytes(encoded({'type':kind,'data_hex':'','children':nodes}))
for table in t.values():table.save()
files=[table.path for table in t.values()]+[ROOT/'Resources/RawData/Level10Parts.mcb',ROOT/'Resources/RawData/Level10Machines.mcb']
manifest={'source_proposal':'docs/research/level10/proposal.json','source_proposal_sha256':hashlib.sha256((ROOT/'docs/research/level10/proposal.json').read_bytes()).hexdigest(),'generator':'tools/apply_level10.py','original_mcb_preserved':True,'files':[{'path':str(p.relative_to(ROOT/'Resources')),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size} for p in files]}
(ROOT/'Resources/level10-extension.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps({'new_levels':59,'part_definitions':len(newparts),'machine_definitions':len(newmachines),'tables':list(t)}))
