import struct,json,math,sys
from pathlib import Path
p=Path(sys.argv[1]);a=(p/'frame-native.bin').open('rb');b=(p/'frame-wasm.bin').open('rb');frames=values=changed=different_frames=0;maximum=0;first=None;nonfinite=0
while True:
 ha=a.read(12);hb=b.read(12)
 if not ha:assert not hb;break
 assert ha==hb,('non-float metadata differs',frames)
 n,_=struct.unpack('<IQ',ha);x=a.read(n*4);y=b.read(n*4);assert len(x)==len(y)==n*4
 if x!=y:
  different_frames+=1
  for idx,(u,v) in enumerate(zip(struct.unpack('<%df'%n,x),struct.unpack('<%df'%n,y))):
   if u!=v:
    changed+=1
    if not math.isfinite(u) or not math.isfinite(v):nonfinite+=1;continue
    d=abs(u-v)
    if d>maximum:maximum=d;first={'frame':frames,'condition':frames//900,'tick':frames%900,'field':idx,'native':u,'wasm':v}
 frames+=1;values+=n
r={'frames':frames,'floatValues':values,'changedValues':changed,'differentFrames':different_frames,'nonFloatMetadataEqual':True,'maxAbsoluteDifference':maximum,'maxDifferenceAt':first,'nonFiniteDifferences':nonfinite};print(json.dumps(r,indent=2));(p/'frame-comparison.json').write_text(json.dumps(r,indent=2))
assert frames==144000 and nonfinite==0 and maximum<.001
