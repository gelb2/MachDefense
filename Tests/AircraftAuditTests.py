"""Level-only art is consumed; genuinely unreferenced art remains unused."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from audit_aircraft_assembly_consumption import classify_authored_usage,audit
unused,level_only=classify_authored_usage(['body','upgrade','orphan'],[[{'part':'body'}]],[[{'part':'body'},{'part':'upgrade'}]])
assert unused==['orphan'] and level_only==['upgrade']
actual=audit()
assert actual['unusedAuthoredParts']==[]
assert actual['levelOnlyAuthoredParts']==sorted([f'{prefix}_{lv:02}'for prefix in ('ALightCover','ADuctCover','AHeavyCover','ATransportRoot0','ATransportRoot1') for lv in range(2,11)])
scout=next(a for a in actual['aircraft']if a['model']=='AircraftLight')
assert scout['nodeCount']==5 and scout['distinctLevelImageSequences']==10
assert [x['nodeCount']for x in scout['levels']]==[5]*10
print('AircraftAuditTests: level-only cover references counted across all 44 aircraft; orphan fixture remains unused')

cheyenne=next(a for a in actual["aircraft"]if a["model"]=="AircraftDuct")
assert cheyenne["nodeCount"]==7 and cheyenne["distinctLevelImageSequences"]==10
assert all(x["nodeCount"]==7 for x in cheyenne["levels"])

beluga=next(a for a in actual["aircraft"]if a["model"]=="AircraftTransport")
assert beluga["nodeCount"]==11 and beluga["distinctLevelImageSequences"]==10
assert all(x["nodeCount"]==11 for x in beluga["levels"])

heavy=next(a for a in actual["aircraft"]if a["model"]=="AircraftHeavy")
assert heavy["nodeCount"]==9 and heavy["distinctLevelImageSequences"]==10
assert all(x["nodeCount"]==9 for x in heavy["levels"])
