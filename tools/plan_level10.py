#!/usr/bin/env python3
"""Calculate a review proposal only. Never writes runtime Resources or Sources."""
import csv, hashlib, json, math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'docs/research/level10'
def table(name):
    lines=[x for x in (ROOT/'Resources/Tables'/f'{name}.tsv').read_text().splitlines() if x.strip() and not x.startswith('//')]
    rows=list(csv.reader(lines,delimiter='\t')); keys=[x.strip() for x in rows[0]]
    return {r[0].strip():dict(zip(keys,[x.strip() for x in r])) for r in rows[1:]}
W=table('WeaponInfo'); B=table('MachBuild_Name'); M=table('MachList'); A=table('SpAttackList'); BASE=table('BaseUpgrade')
items=table('ItemInfo');rows=[]; weapons=[]
configs=[('RifleTurret','Tur_Gun','Turret_gun',lambda l:50*l*l+50*l+500,5,30),('MissileTurret','Tur_Msl','Turret_msl',lambda l:50*l*l+250*l+1000,20,30),('Vulcan','Tur_MG','Turret_mg',lambda l:50*l*l+250*l+1000,5,100),('IonCannon','Tur_Ion','Turret_ion',lambda l:50*l*l+350*l+1300,40,300),('LaserTurret','Tur_Lsr','Turret_lsr',lambda l:500*l+1000,5,0),('PlasmaCannon','Tur_Pla','Turret_pla',lambda l:50*l*l+350*l+1300,30,None)]
def weapon(prefix,l,cap):
    w=W[f'{prefix}{min(l,cap):02d}'].copy();w['Parts']=f'{prefix}{l:02d}';return w
def put(name,l,cap,w,hp,cost,upgrade,asset,extra=''):
    row=dict(object=name,level=l,source='original' if l<=cap else 'proposal',hp=hp,damage=int(w['Dmg']) if w else '',shots=int(w['ShtCnt']) if w else '',shot_delay=float(w['ShtDly']) if w else '',reload=float(w['RldDly']) if w else '',range=int(w['Range']) if w else '',shot_range=int(w['ShtRng']) if w else '',purchase_cost=cost,upgrade_to_next=upgrade,asset_level=asset,extra=extra)
    if w:
        row['ui_attack_estimate']=round(int(w['Dmg'])*int(w['ShtCnt'])/(int(w['ShtCnt'])*float(w['ShtDly'])+float(w['RldDly'])),3)
        weapons.append(dict(object=name,level=l,source=row['source'],fields=w))
    else:row['ui_attack_estimate']=''
    rows.append(row)
for name,mach,prefix,hpf,dd,dc in configs:
    last=B[f'{name}_05'];first_extension=int(B[f'{name}_04']['UpgCost'])*2
    for l in range(1,11):
        n=l-5;w=weapon(prefix+'_',l,5)
        if l>5:
            w['Dmg']=str(int(w['Dmg'])+dd*n)
            if name in ('RifleTurret','LaserTurret'):
                w['ShtCnt']=str(24+2*n);w['ShtDly']=str(round(.14-.005*n,3));w['RldDly']=str(round(1.9-.05*n,3))
            elif name=='MissileTurret':
                w['ShtCnt']=str(5+(n+1)//2);w['ShtDly']=str(round(.35-.01*n,3));w['RldDly']=str(round(2-.05*n,3))
            elif name=='Vulcan':w['ShtCnt']=str(80+10*n)
            elif name=='IonCannon':w['RldDly']=str(round(1.6-.08*n,3))
            elif name=='PlasmaCannon':w['RldDly']=str(round(.8-.04*n,3))
        if l<=5:
            cost=int(B[f'{name}_{l:02d}']['Cost']);hp=2*int(M[f'{mach}_{l:02d}']['HP'])
            assert hp==2*hpf(l)
        else:
            hp=2*hpf(l);cost=int(last['Cost'])+(dc*n if dc is not None else sum(60+10*k for k in range(1,n+1)))
        up=int(B[f'{name}_{l:02d}']['UpgCost']) if l<5 else (0 if l==10 else first_extension*2**(l-5))
        put(name,l,5,w,hp,cost,up,min(l,5))
for category in ('BaseCannon','BaseDefense'):
    for l in range(1,11):
        n=l-7;w=weapon('BaseWpn',l,7) if category=='BaseCannon' else None
        if w and l>7:w.update(Dmg=str(30+5*n),ShtCnt=str(24+2*n),ShtDly=str(round(.1-.005*n,3)),RldDly=str(round(1.9-.05*n,3)))
        hp='' if w else (int(BASE[f'BaseDefense_{l:02d}']['Param']) if l<=7 else 100000+30000*n)
        up=int(BASE[f'{category}_{l:02d}']['UpgCost']) if l<7 else (0 if l==10 else 600*2**(l-1))
        put(category,l,7,w,hp,'',up,min(l,7))
for name,prefix,cap,idbase,dd,dr,dc in [('AirStrike-Bomb','AS_BOMB_',5,2000,100,10,100),('AirStrike-Missile','AS_MISSILE_',5,2010,100,10,150),('AirStrike-Nuclear','AS_NUCLEAR_',5,2020,1000,10,200),('AirStrike-EMP','AS_EMP_',4,2030,50,10,300),('Crossfire-Missile','CF_MISSILE_',8,2100,0,5,300)]:
    last=A[f'{name}_{cap:02d}']
    for l in range(1,11):
        n=l-cap;w=weapon(prefix,l,cap)
        if l>cap:
            assert f'{idbase+l:05d}' not in items
            w['Dmg']=str(int(w['Dmg'])+dd*n);w['Range']=str(int(w['Range'])+dr*n)
            if cap==5 and name!='AirStrike-Nuclear' or name=='Crossfire-Missile':w['ShtCnt']=str(int(w['ShtCnt'])+n)
            if name=='AirStrike-EMP':w['sp1']=str(36+2*n)
        cost=int(A[f'{name}_{l:02d}']['Cost']) if l<=cap else int(last['Cost'])+dc*n
        up=int(A[f'{name}_{l:02d}']['UpgCost']) if l<cap else (0 if l==10 else (32000 if name=='Crossfire-Missile' and l==9 else int(last['UpgCost'])*2**(l-cap)))
        extra=f"EMP持続={w['sp1']}s" if name=='AirStrike-EMP' else ''
        extra=extra.replace('持続','지속')
        put(name,l,cap,w,'',cost,up,min(l,cap),extra)
# Verify existing rows and all proposed primitive values; runtime assets stay read-only.
visual=('Type','Grd','Speed','AimRt','UiTile','BltTile','EffType','EffTile','Shell','FireEff','ExplodeEff','ExplScl','FireSound')
for entry in weapons:
    w=entry['fields'];k=w['Parts'];assert list(w)==list(next(iter(W.values())))
    if entry['source']=='original':assert w==W[k]
    for key in ('Dmg','Range'):assert 0<=int(w[key])<=32767
    assert 0<int(w['ShtCnt'])<=255
    assert 0<int(float(w['ShtRng'])*1.5)<=32767
    assert float(w['ShtDly'])>=0 and float(w['RldDly'])>0
    if int(w['ShtCnt'])>1:assert float(w['ShtDly'])>0
    assert int(w['Dmg'])*int(w['ShtCnt'])<2147483647
    if entry['source']=='proposal':
        same=[x for x in weapons if x['object']==entry['object'] and x['source']=='original'][-1]['fields']
        assert all(w[key]==same[key] for key in visual)
for r in rows:
    for k in ('hp','purchase_cost','upgrade_to_next'):
        if r[k]!='':assert 0<=r[k]<2147483647
OUT.mkdir(parents=True,exist_ok=True)
fields=list(rows[0]);
with (OUT/'levels.csv').open('w') as f:
    out=csv.DictWriter(f,fieldnames=fields);out.writeheader();out.writerows(rows)
(OUT/'proposal.json').write_text(json.dumps({'status':'proposal-not-runtime','rows':rows,'weapons':weapons},ensure_ascii=False,indent=2)+'\n')
manifest={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted((ROOT/'Resources/Tables').glob('*.tsv'))}
summary={'objects':13,'levels':len(rows),'new_levels':sum(r['source']=='proposal' for r in rows),'weapon_rows':len(weapons),'max_damage':max(int(x['fields']['Dmg']) for x in weapons),'max_shots':max(int(x['fields']['ShtCnt']) for x in weapons),'resource_table_sha256':manifest,'checks':['existing weapon rows equal','table column layout equal','new special item IDs unused','damage/range int16 and shots uint8 valid','HP/cost/product int32 valid','new visual/audio fields equal original maximum'],'limitations':['proposal arithmetic only; no new game implementation or device performance test']}
(OUT/'validation.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2)+'\n')
print(json.dumps({k:v for k,v in summary.items() if k!='resource_table_sha256'},ensure_ascii=False))
