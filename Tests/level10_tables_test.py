#!/usr/bin/env python3
"""Check extension against the unchanged original prepared table blobs."""
import csv, hashlib, importlib.util, json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('prepare',ROOT/'tools/prepare_resources.py');prepare=importlib.util.module_from_spec(spec);spec.loader.exec_module(prepare)
def parse(text):
 lines=[x for x in text.splitlines() if x.strip() and not x.startswith('//')]
 rows=list(csv.reader(lines,delimiter='\t'));keys=[x.strip() for x in rows[0]]
 return keys,{r[0].strip():dict(zip(keys,[x.strip() for x in r])) for r in rows[1:]}
changed=[];added={}
for name in ['WeaponInfo','MachBuild_Name','MachList','BaseUpgrade','SpAttackList','ItemInfo','PartsList','TileList']:
 oldkeys,old=parse(prepare.decode_table((ROOT/f'Resources/RawData/{name}.tble').read_bytes()).decode('utf-8-sig'))
 keys,now=parse((ROOT/f'Resources/Tables/{name}.tsv').read_text())
 assert oldkeys==keys,(name,'schema')
 for key,row in old.items():
  assert key in now
  for col,value in row.items():
   if now[key][col]!=value:
    assert name in ['MachBuild_Name','BaseUpgrade'] and col=='UpgCost' and value=='0',(name,key,col)
    changed.append((name,key,col))
 added[name]=len(now)-len(old)
assert len(changed)==8,changed
for name in ['PartsInfo','MachInfo']:
 source=json.loads((ROOT/f'Resources/Data/{name}.json').read_text())
 assert hashlib.sha256((ROOT/f'Resources/RawData/{name}.mcb').read_bytes()).hexdigest()==source['source_sha256']
proposal=json.loads((ROOT/'docs/research/level10/proposal.json').read_text())
_,weapons=parse((ROOT/'Resources/Tables/WeaponInfo.tsv').read_text())
for w in proposal['weapons']:assert weapons[w['fields']['Parts']]==w['fields']
print(json.dumps({'original_rows_changed_only_max_upgrade_price':changed,'added_rows':added,'original_mcb_sha':'unchanged','proposal_weapon_rows_equal':len(proposal['weapons'])}))
