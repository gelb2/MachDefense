#!/usr/bin/env python3
"""Generate a review proposal. Never writes Resources, Sources, or player saves."""
import csv, hashlib, json, math
from collections import defaultdict
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'docs/evidence/aircraft-balance-20261108'
def read(p):
    with p.open() as f: return list(csv.DictReader(f, delimiter='\t'))
def number(v):
    try:
        n=float(v)
        return int(n) if n.is_integer() else n
    except ValueError: return v
old=[{k:number(v) for k,v in r.items()} for r in read(ROOT/'docs/evidence/scout-width-20261108/specs.tsv')]
refs=[{k:number(v) for k,v in r.items()} for r in read(OUT/'references.tsv')]
rates=[{k:number(v) for k,v in r.items()} for r in read(OUT/'fire-rates.tsv')]
R={(r['reference'],r['level']):r for r in refs}
rate={r['weapon']:r['shotsPerSecond'] for r in rates}
reference_dps=defaultdict(float)
for r in rates: reference_dps[r['unit'],r['level']]+=r['idealDps']
with (ROOT/'Resources/Tables/WeaponInfo.tsv').open() as f:
    raw=list(csv.reader(f,delimiter='\t'))
keys=[s.strip() for s in raw[0]]
raw_weapons={r[0].strip():dict(zip(keys,[number(s.strip()) for s in r])) for r in raw[1:] if len(r)==len(keys)}
NAMES={'Scout Heli':'스카웃','Cheyenne':'샤이엔','Beluga':'벨루가','AH-10':'AH-10'}
# Actual catalog labels are retained in keys, Korean names are only report labels.
assert set(NAMES)=={r['unit'] for r in old}, {r['unit'] for r in old}
def rnd(x): return math.floor(x+.5)
def lerp(a,b,t): return a+(b-a)*t
def growth(v,p): return None if p is None else round((v/p-1)*100,3)
units=[];weapons=[]
for index,(name,label) in enumerate(NAMES.items()):
    t=index/3
    cumulative=0;prev=None;prev_weapons={}
    for lv in range(1,11):
        rows=[r for r in old if r['unit']==name and r['level']==lv]
        base=rows[0];mg=R['MachineGun',lv];vu=R['Vulcan',lv]
        scout=next(r for r in old if r['unit']=='Scout Heli' and r['level']==lv)
        raw_range=rnd(lerp(mg['range']/1.5,vu['range']/1.5,t))
        target=lerp(reference_dps['MachineGun',lv],reference_dps['Vulcan',lv]*.9,t)
        previous_dps=sum(r['damage']*r['count']*rate[r['weapon']] for r in rows)
        factor=target/previous_dps
        proposal_weapons=[]
        for row in rows:
            w=dict(row)
            w['damage']=max(1,rnd(row['damage']*factor))
            w['rawRange']=raw_range;w['range']=int(raw_range*1.5);w['phoneRange']=int(w['range']*.5)
            w['hudAttackPoint']=rnd(w['damage']*w['burst']/(w['burst']*w['shotDelay']+w['reloadDelay']))
            w['shotsPerSecond']=rate[w['weapon']]
            w['idealDps']=w['damage']*w['count']*w['shotsPerSecond']
            key=w['weapon'].rsplit('_',1)[0]
            w['damageGrowthPct']=growth(w['damage'],prev_weapons.get(key))
            prev_weapons[key]=w['damage']
            w['sourceDamage']=row['damage']
            # Entire resource row retained to identify every projectile/effect parameter.
            w['proposedWeaponInfo']=dict(raw_weapons[w['weapon']], Dmg=w['damage'], ShtRng=raw_range)
            # Remove obsolete unit-wide source numbers; final unit rows carry these.
            for k in ['mission','hp','cells','upgradeCredits','buildSeconds','queueLimit','speedPerTick','radius','height','noseDegreesPerSecond','weaponDegreesPerSecond']: w.pop(k)
            proposal_weapons.append(w)
        u={k:base[k] for k in ['unit','level','mission','buildSeconds','queueLimit','speedPerTick','radius','height','noseDegreesPerSecond','weaponDegreesPerSecond']}
        u.update(label=label,maxLevel=10,cells=rnd(lerp(mg['cells']*1.2,vu['cells']*1.2,t)),
            upgradeCredits=rnd(lerp(mg['upgradeCredits']*1.2,vu['upgradeCredits']*1.2,t)),
            cumulativeUpgradeCredits=cumulative,
            hp=rnd(lerp(scout['hp'],vu['hp']*.9,t)/2)*2,
            rawRange=raw_range,range=int(raw_range*1.5),phoneRange=int(int(raw_range*1.5)*.5),
            targetIdealDps=target,idealDps=sum(w['idealDps'] for w in proposal_weapons),
            hudAttack=sum(w['hudAttackPoint']*w['count'] for w in proposal_weapons),
            sourceHP=base['hp'],sourceCells=base['cells'],sourceRange=base['range'],sourceIdealDps=previous_dps)
        u['tableHP']=u['hp']//2
        u['targetDeviationPct']=(u['idealDps']/target-1)*100
        for k in ['hp','cells','range','idealDps','hudAttack','speedPerTick','buildSeconds','upgradeCredits']:
            u[k+'GrowthPct']=growth(u[k],prev[k] if prev else None)
        # Level 10 has no further upgrade, not a -100% price discount.
        if lv==10:u['upgradeCreditsGrowthPct']=None
        cumulative+=u['upgradeCredits'];prev=u
        units.append(u);weapons.extend(proposal_weapons)

def tsv(path,rows):
    with path.open('w') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]),delimiter='\t');w.writeheader();w.writerows(rows)
tsv(OUT/'WeaponInfo.proposal.tsv',[w['proposedWeaponInfo'] for w in weapons])
tsv(OUT/'units.tsv',units)
tsv(OUT/'weapons.tsv',[{k:v for k,v in w.items() if k!='proposedWeaponInfo'} for w in weapons])
checks=[]
def check(name,truth):
    assert truth,name
    checks.append(name)
for lv in range(1,11):
    s,c,b,a=[next(u for u in units if u['unit']==name and u['level']==lv) for name in NAMES]
    mg,vu=R['MachineGun',lv],R['Vulcan',lv]
    check(f'L{lv} reference order',mg['range']<vu['range'] and mg['cells']<vu['cells'])
    check(f'L{lv} Scout range',s['range']==mg['range'] and s['phoneRange']==mg['phoneRange'])
    check(f'L{lv} AH range',a['range']==vu['range'] and a['phoneRange']==vu['phoneRange'])
    check(f'L{lv} Scout price',s['cells']*5==mg['cells']*6)
    check(f'L{lv} AH price',a['cells']*5==vu['cells']*6)
    check(f'L{lv} AH HP',a['hp']*10==vu['hp']*9)
    for k in ['range','phoneRange','cells','hp','idealDps']:
        check(f'L{lv} ascending {k}',s[k]<c[k]<b[k]<a[k])
for u in units:
    check(f"{u['unit']} L{u['level']} integer limits", all(isinstance(u[k],int) and 0<u[k]<32768 for k in ['hp','tableHP','range','phoneRange','cells']))
    check(f"{u['unit']} L{u['level']} upgrade ceiling",(u['upgradeCredits']==0)==(u['level']==10))
    check(f"{u['unit']} L{u['level']} target within 3%",abs(u['targetDeviationPct'])<3)
    if u['level']>1:
        check(f"{u['unit']} L{u['level']} growth",all(u[k+'GrowthPct']>0 for k in ['hp','cells','range','idealDps']))
for w in weapons:
    check(w['weapon']+' damage limits',0<w['damage']<32768 and isinstance(w['damage'],int))
    if w['level']>1:check(w['weapon']+' damage growth',w['damageGrowthPct']>0)
check('40 unit levels',len(units)==40)
check('90 weapon rows',len(weapons)==90)
manifest={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [ROOT/'Resources/Tables/WeaponInfo.tsv',ROOT/'Resources/Tables/MachList.tsv',ROOT/'Resources/Tables/MachBuild_Name.tsv',ROOT/'Resources/Tables/ItemInfo.tsv',ROOT/'docs/evidence/scout-width-20261108/specs.tsv']}
fixed={'dodgeSpeedMultiplier':2.5,'dodgeMaxInvulnerabilitySeconds':.45,'dodgeCooldownAfterEndSeconds':[.25,.50], 'sharedProductionJobsLimit':7,'targetRefreshSeconds':1,'maximumLevel':10,'lockOn':False,'orbitSectorCount':12,'orbitRetargetSeconds':[2.4,4.0],'rotorRevolutionsPerSecond':7.3,'maxBankRadians':.4,'bankLerp':.18,'bankStep':.035,'orbitWidthFractions':{'Scout Heli':.50,'Cheyenne':.425,'Beluga':.25,'AH-10':.425},'dodgeThreatSearchCruiseMultiplier':21,'dodgeDirectionCandidates':12}
data={'status':'proposal-not-applied','baselineCommit':'db6e0a8','sourceHashes':manifest,'formula':'same-level interpolation t=0,1/3,2/3,1; source damage ratios preserved; integer rounding half up','fireRateMethod':{'stepSeconds':float.fromhex('0x1.0ff972p-6'),'warmupSeconds':60,'sampleSeconds':600,'mode':'manual fire, always ready, emissions only; no hit/splash simulation'},'fixedProposal':fixed,'references':refs,'units':units,'weapons':weapons}
(OUT/'proposal.json').write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')
validation={'status':'passed','checks':len(checks),'unitLevels':len(units),'weaponRows':len(weapons),'maxTargetDpsDeviationPct':max(abs(u['targetDeviationPct']) for u in units),'checksPassed':checks,'sourceHashes':manifest}
(OUT/'validation.json').write_text(json.dumps(validation,ensure_ascii=False,indent=2)+'\n')

def f(v,d=2):
    if v is None:return '—'
    if isinstance(v,str):return v
    return str(v) if isinstance(v,int) else f'{v:.{d}f}'
def table(headers,rows):
    return '\n'.join(['| '+' | '.join(headers)+' |','|'+'|'.join(['---']*len(headers))+'|']+['| '+' | '.join(f(x) for x in row)+' |' for row in rows])+'\n\n'
md=['''# 헬기 4종 Lv.1–10 수치안 — 2026-11-08

사용자 규칙은 같은 레벨 기준으로 모두 성립한다. 추가 지시 없이 수치안을 작성할 수 있다. 이 문서는 **정식 도입을 위한 제안**이며 게임에 적용하지 않았다. 크기·디자인·색조는 현재 승인 상태를 유지한다.

## 사용자 지정값과 제안 기준

- **사용자 지정:** 스카웃 사거리=머신건 메크, AH-10 사거리=발칸포탑. 스카웃 생산 셀=머신건 메크×1.20, AH-10 생산 셀=발칸포탑×1.20. AH-10 HP=발칸포탑×0.90. 전 기체 최대 Lv.10.
- **사용자 지정 범위:** 샤이엔·벨루가 생산 셀은 두 기체 사이. 사거리는 사이이거나 근소한 이탈을 허용했다. 이 안에서는 이탈 없이 샤이엔 1/3, 벨루가 2/3 지점에 둔다.
- **가격 해석:** 20% 할증은 생산 셀에 적용한다. 강화 크레딧은 별도 재화다. 강화 비용에도 같은 기준 유닛의 1.20배를 사용하는 것은 이번 **제안**이다. 중간 두 기체의 강화 비용은 같은 1/3·2/3 보간을 쓴다.
- **HP 제안:** 스카웃은 현재 HP를 유지한다. 샤이엔·벨루가는 스카웃과 AH-10 HP의 1/3·2/3 지점으로 정한다. 기체 테이블 HP가 실행 시 2배가 되므로 가장 가까운 짝수로 반올림한다. AH-10 지정 비율은 정확히 맞는다.
- **화력 제안:** 스카웃의 전탄 명중 가정 발사 DPS 목표는 같은 레벨 머신건 메크의 100%, AH-10은 발칸포탑의 90%. 샤이엔·벨루가는 그 사이 1/3·2/3 지점이다. 기동·회피와 범위 피해를 감안해 AH-10에 10%의 기본 화력 차이를 두는 안이다. 목표값에 맞춰 각 레벨의 기존 무장별 피해 비율을 동일 배수로 조정하고 발당 피해를 정수로 반올림한다.
- **유지 제안:** 무장 수·타입·연사·재장전·탄속·폭발 반경·명중 분산, 생산 시간·해금·이동·선회·회피는 현재 값. 기체별 역할은 스카웃=저비용·빠른 생산·빠른 선회, 샤이엔=빠른 직선 기동, 벨루가=다수 범위 무장, AH-10=긴 사거리·높은 HP다.

사거리는 deviceScale=1 월드 좌표다. 미터나 화면 픽셀이 아니다. 게임 표의 ShtRng를 정수로 정한 뒤 `월드=int(ShtRng×1.5)`, `휴대폰=int(월드×0.5)`를 적용했다. 중간 사거리는 이 반올림 때문에 정확한 삼등분과 최대 1 단위 차이가 난다. 각 기체의 모든 무장 사거리는 같다.

증가율은 직전 레벨 대비 `(현재/직전−1)×100`이다. 고정 증가율을 강제하지 않고 원작 기준 유닛의 성장 곡선을 따른다. 아래 표는 반올림 후 실제 채택할 수치를 표시한다.

## Lv.1과 Lv.10 비교

''']
md.append(table(['기체','레벨','HP','생산 셀','월드/휴대폰 사거리','발사 DPS','HUD 공격','Lv.10까지 강화 크레딧'],[[u['label'],u['level'],u['hp'],u['cells'],f"{u['range']} / {u['phoneRange']}",u['idealDps'],u['hudAttack'],next(x['cumulativeUpgradeCredits'] for x in units if x['unit']==u['unit'] and x['level']==10)] for u in units if u['level'] in (1,10)]))
md.append('''AH-10 HP는 사용자 비율에 따라 **2,340→15,300**이다. 현행 **700→2,100**보다 각각 약 3.34배·7.29배다. 발칸포탑 HP를 실제 게임 로더로 읽어 계산한 결과다. 규칙의 충돌은 없지만 회피 무적까지 더해지므로 실전 생존력은 별도 검증해야 한다.

발사 DPS는 실제 WeaponController를 `.0166f` 간격으로 실행해 60초 예열 후 600초 동안 센 발사 수에 피해와 장착 수를 곱한 값이다. 모든 탄이 한 표적에 명중한다고 가정하며 폭발의 다중 피해는 곱하지 않는다. 이동·조준 손실·빗나감·피격을 포함한 전투 실측 DPS는 아니다. HUD 공격은 게임의 기존 계산식이며 DPS와 다르다.

## 기체별 전체 레벨

강화 비용은 **해당 행 레벨→다음 레벨** 비용이다. 누적은 Lv.1에서 해당 레벨까지 지출한 크레딧이다. Lv.10의 ‘—’는 강화 종료이며 무료 Lv.11을 뜻하지 않는다. 생산 시간은 기지 생산 강화 전이다.

''')
weaponlabels={'ALightGun':'기관총','ALightMissile':'로켓','ADuctGun':'기관총','ADuctMissile':'로켓','ATransportGun':'포','ATransportMissile':'미사일','AHeavyGun':'기관총','AHeavyMissile':'로켓','AHeavyCannon':'포'}
for name,label in NAMES.items():
    us=[u for u in units if u['unit']==name]
    md.append('### '+label+'\n\n')
    md.append(table(['Lv.','HP','생산 셀','다음 강화 크레딧','누적 크레딧','사거리 월드/폰','DPS','HUD 공격','생산 초','속도/틱'],[[u['level'],u['hp'],u['cells'],u['upgradeCredits'] if u['level']<10 else '—',u['cumulativeUpgradeCredits'],f"{u['range']} / {u['phoneRange']}",u['idealDps'],u['hudAttack'],u['buildSeconds'],f(u['speedPerTick'],3)] for u in us]))
    md.append(table(['Lv.','HP 증가%','생산 셀 증가%','사거리 증가%','DPS 증가%','HUD 증가%','속도 증가%','생산 시간 증가%','다음 강화비 증가%'],[[u['level']]+[u[k+'GrowthPct'] for k in ['hp','cells','range','idealDps','hudAttack','speedPerTick','buildSeconds','upgradeCredits']] for u in us]))
    ws=[w for w in weapons if w['unit']==name];ids=list(dict.fromkeys(w['weapon'].rsplit('_',1)[0] for w in ws))
    md.append(table(['Lv.']+[weaponlabels[key]+' 피해/발 (증가%)' for key in ids],[[lv]+[f"{w['damage']} ({f(w['damageGrowthPct'])}%)" if lv>1 else str(w['damage']) for key in ids for w in ws if w['weapon']==key+f'_{lv:02}'] for lv in range(1,11)]))
md.append('## 무장 고정값\n\n피해·사거리만 위 표에 따라 바꾸는 제안이다. 아래 연사·탄속·명중 분산은 10레벨 공통이다. 발당 피해는 장착 수를 곱하기 전이다.\n\n')
md.append(table(['기체·무장','수','타입','연사 수','간격 초','재장전 초','탄속 설정','폭발 반경','AimRt'],[[NAMES[w['unit']]+' '+weaponlabels[w['weapon'].rsplit('_',1)[0]],w['count'],w['type'],w['burst'],w['shotDelay'],w['reloadDelay'],w['bulletSpeed'],w['splashRadius'],w['accuracyParameter']] for w in weapons if w['level']==1]))
md.append('''Gun은 즉시 피해, Bullet·Missile은 탄속이 있는 발사체다. 명칭이 로켓인 스카웃·샤이엔·AH-10 무장은 Bullet형이다. 벨루가 미사일은 Missile형이지만 현재 락온은 꺼져 있어 움직이는 표적을 계속 추적하지 않는다. 탄속 설정은 엔진 내부 값이며 초당 월드 이동거리로 읽지 않는다. Gun의 탄속 값은 즉시 피해의 이동 속도를 뜻하지 않는다. AimRt는 명중 확률이 아닌 분산 계산 계수다. 휴대폰 좌표에서 폭발 반경·탄속 설정은 절반이다.

전탄 연속 발사의 이론 주기는 `(연사 수−1)×발사 간격+max(재장전, 발사 간격)`에 가깝다. 실제 코드의 틱 반올림을 포함하기 위해 본문 DPS에는 발사 횟수 조회 결과를 사용했다. HUD 공격은 무장별 `round(피해×연사 수/(연사 수×발사 간격+재장전))×장착 수`의 합계다.

## 이동·생산·회피 고정값

''')
md.append(table(['기체','해금: 완료 미션 수','동일 항목 생산 작업 상한','피격 반경','높이','기수 °/초','무장 °/초','궤도 폭 계수'],[[u['label'],u['mission'],u['queueLimit'],u['radius'],u['height'],u['noseDegreesPerSecond'],u['weaponDegreesPerSecond'],fixed['orbitWidthFractions'][u['unit']]] for u in units if u['level']==1]))
md.append('''- 전체 생산 작업은 최대 7개. 기체별 3/3/3/2는 동시 생산 작업 제한이며 살아 있는 유닛 수 제한이 아니다. 위 해금 미션은 현행 조건을 유지하는 제안이다.
- 회피 속도 2.5배, 무적 최대 0.45초, 종료 후 재사용 대기 0.25~0.50초. 목적지에 먼저 도착하면 조기 종료한다. 히트스캔은 회피를 직접 발동시키지 않지만 회피 중 무적은 히트스캔 피해도 막는다.
- 평상시 가속/감속은 순항 속도의 1/30·1/24씩, 회피 중에는 2.5배로 높인 회피 속도의 1/6·1/4씩 적용한다. 회피 위협 검색 거리는 순항 속도×21, 위험 판정 반경은 max(폭발 반경, 피격 반경)+순항 속도×6이다. 회피 거리는 max(폭발 반경, 피격 반경)×1.3을 순항 속도×16~45 범위로 제한한다. 목적지 후보는 12방향이다. 기체 피격 반경과 그림 외곽 사각형은 별도다.
- 궤도 반경은 `min(맵 폭×궤도 폭 계수×0.5, 사거리×0.72)`. 12구간을 쓰며 목적지는 2.4~4.0초마다 갱신한다. 목표 검색 간격은 1초다. 한 기체의 무장은 같은 표적을 공유하며 무장별 사거리 조건을 검사한다.
- 기울기 상한 ±0.4rad, 보간 0.18·단계 0.035, 기수 보간 1/8, 로터 7.3회전/초를 유지한다. 기체별 선회 상한은 위 표를 따른다.
- 새 방어력·관통·치명타·상태 이상 보너스는 추가하지 않는 안이다. 현재 무장 부가값·탄종·효과와 피격 처리는 유지한다. 전체 무장 표의 부가 파라미터는 JSON의 `proposedWeaponInfo`에 포함했다.

## 현행 수치와 제안 차이

''')
md.append(table(['기체·레벨','HP 현재→제안','생산 셀 현재→제안','사거리 현재→제안','발사 DPS 현재→제안'],[[u['label']+' '+str(u['level']),f"{u['sourceHP']} → {u['hp']}",f"{u['sourceCells']} → {u['cells']}",f"{u['sourceRange']} → {u['range']}",f"{u['sourceIdealDps']:.2f} → {u['idealDps']:.2f}"] for u in units if u['level'] in (1,10)]))
md.append('''스카웃의 Lv.10 화력은 원작 머신건 메크의 성장에 맞춰 현행보다 크게 올라간다. 샤이엔·벨루가 HP도 새 AH-10 HP를 기준으로 보간하므로 증가 폭이 크다. 따라서 화력/셀·HP/셀만으로 최종 균형을 확정하지 않는다. 기존 회피 시험 결과는 이 수치안의 검증 결과로 재사용하지 않는다.

## 검사와 적용 시 남은 검증

''')
md.append(f"수치 조건 **{len(checks)}개**를 검사했다. 40개 레벨·90개 무장 행에서 사용자 지정 비율, 중간 범위, 최대 레벨, 정수 범위와 성장을 확인했다. 피해 반올림 뒤 목표 DPS와 최대 차이는 **{validation['maxTargetDpsDeviationPct']:.2f}%**다. 기준 유닛·발사 주기는 실제 C++ 로더와 WeaponController로 조회했다. 별도 제안 무장 표를 C++ WeaponCatalog로 다시 읽어 피해·월드/휴대폰 사거리·HUD 공격·연사 수·폭발 반경 540항목을 대조했다.\n\n")
md.append('''정식 적용 시에는 ① 전체 10레벨 생산·강화·저장 복원, ② 같은 셀 예산의 머신건 메크·발칸포탑과 처치 시간/생존율 비교, ③ 히트스캔·비유도탄·범위 공격 상대의 회피와 누적 피해, ④ 여러 대의 대기열·기지 생산 강화·해금 경계, ⑤ PC/휴대폰 사거리 판정을 확인한다. 이 수치안의 캠페인 플레이 검증은 아직 하지 않았다.

추가 지시가 반드시 필요한 모순은 없다. 사용자가 정하지 않은 화력 기준, 중간 HP, 강화 크레딧, 기존 이동·회피 유지 여부는 이 문서의 제안값이다. 이 값들을 채택하면 구현 가능한 1~10레벨 표가 된다.

## 자료와 재계산

[기체 40행 TSV](../evidence/aircraft-balance-20261108/units.tsv) · [무장 90행 TSV](../evidence/aircraft-balance-20261108/weapons.tsv) · [전체 제안 JSON](../evidence/aircraft-balance-20261108/proposal.json) · [기준 유닛](../evidence/aircraft-balance-20261108/references.tsv) · [발사 주기 조회](../evidence/aircraft-balance-20261108/fire-rates.tsv) · [수치 검사](../evidence/aircraft-balance-20261108/validation.json) · [C++ 로더 대조](../evidence/aircraft-balance-20261108/loader-validation.json) · [제안 무장 표](../evidence/aircraft-balance-20261108/WeaponInfo.proposal.tsv)

기준 커밋 db6e0a8. 입력 파일 SHA-256은 JSON에 보존했다. 생성 도구는 Resources·Sources·저장 파일에 쓰지 않는다.

```
c++ -std=c++20 -O2 -I Sources tools/aircraft_balance_reference_probe.cpp build/save-transfer/libMachDefenseCore.a -o /tmp/md-aircraft-balance-probe
/tmp/md-aircraft-balance-probe Resources docs/evidence/aircraft-balance-20261108/fire-rates.tsv > docs/evidence/aircraft-balance-20261108/references.tsv
python3 tools/propose_aircraft_balance.py
c++ -std=c++20 -O2 -I Sources tools/aircraft_balance_validate.cpp build/save-transfer/libMachDefenseCore.a -o /tmp/md-aircraft-balance-validate
/tmp/md-aircraft-balance-validate docs/evidence/aircraft-balance-20261108/WeaponInfo.proposal.tsv docs/evidence/aircraft-balance-20261108/weapons.tsv
```
''')
(ROOT/'docs/research/aircraft-balance-20261108.md').write_text(''.join(md))
print(json.dumps({k:validation[k] for k in ['status','checks','unitLevels','weaponRows','maxTargetDpsDeviationPct']}))
