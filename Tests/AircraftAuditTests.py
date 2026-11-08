"""Level-only art is consumed; genuinely unreferenced art remains unused."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from audit_aircraft_assembly_consumption import classify_authored_usage,audit
unused,level_only=classify_authored_usage(['body','upgrade','orphan'],[[{'part':'body'}]],[[{'part':'body'},{'part':'upgrade'}]])
assert unused==['orphan'] and level_only==['upgrade']
actual=audit()
assert actual['unusedAuthoredParts']==[]
assert actual['levelOnlyAuthoredParts']==[f'ALightCover_{lv:02}'for lv in range(2,11)]
scout=next(a for a in actual['aircraft']if a['model']=='AircraftLight')
assert scout['nodeCount']==4 and scout['distinctLevelImageSequences']==10
assert [x['nodeCount']for x in scout['levels']]==[4]+[5]*9
print('AircraftAuditTests: level-only cover references counted across all 44 aircraft; orphan fixture remains unused')
