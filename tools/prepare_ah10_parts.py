#!/usr/bin/env python3
"""Partition adopted AH-10 pixels into its two fixed embedded weapon parts.

No repaint/rescale/backfill: WorldCombat already keeps both parts body-aligned
with zero recoil. Their full seven-cell lattice preserves texture/shadow sampling.
"""
import hashlib,json,subprocess,tempfile
from pathlib import Path
from prepare_resources import png_pixels,png_encode
ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'Art/Aircraft/banking-20261109/prepared/AHeavyBody.png'
OUT=ROOT/'Art/Aircraft/parts-separation-20261109/ah10'
NAMES=('AHeavyBody','AHeavyRocket','AHeavyAirStrike')
# Polygon boundaries follow the capsule pod and the lower forward narrow pod.
# These are ownership seams; every approved pixel survives exactly once.
ROCKET=[
 [(14,21),(19,18),(36,18),(47,21),(52,25),(52,30),(44,34),(17,34),(13,31),(13,24)],
 [(12,22),(18,19),(36,20),(48,22),(53,26),(53,31),(43,35),(15,35),(11,32),(11,25)],
 [(11,23),(18,20),(35,21),(48,23),(54,27),(54,33),(43,37),(14,37),(10,33),(10,26)],
 [(11,24),(18,21),(35,22),(48,24),(54,28),(54,34),(43,38),(14,38),(10,34),(10,27)],
 [(12,22),(18,19),(36,19),(48,21),(53,25),(53,30),(43,34),(15,34),(11,31),(11,25)],
 [(11,21),(18,18),(36,18),(48,20),(54,24),(54,29),(43,33),(14,33),(10,30),(10,24)],
 [(10,21),(17,18),(36,18),(48,20),(54,24),(54,28),(43,32),(13,32),(9,29),(9,24)],
]
AIRSTRIKE=[(42,68,69,80),(42,68,69,80),(42,69,69,81),(42,70,69,82),(41,67,69,79),(41,65,69,78),(40,64,69,77)]
def inside(x,y,poly):
 result=False;j=len(poly)-1
 for i,(xi,yi) in enumerate(poly):
  xj,yj=poly[j]
  if (yi>y)!=(yj>y) and x<(xj-xi)*(y-yi)/(yj-yi)+xi:result=not result
  j=i
 return result

def run():
 OUT.mkdir(parents=True,exist_ok=True)
 raw=SOURCE.read_bytes();meta,pixels=png_pixels(raw);w,h=meta['width'],meta['height'];assert(w,h)==(532,112)
 layers=[bytearray(len(pixels)) for _ in NAMES];counts=[[0]*3 for _ in range(7)]
 for y in range(h):
  for x in range(w):
   f,lx=divmod(x,76);a,b,c,d=AIRSTRIKE[f]
   owner=1 if inside(lx+.5,y+.5,ROCKET[f]) else 2 if a<=lx<c and b<=y<d else 0
   i=(y*w+x)*4;layers[owner][i:i+4]=pixels[i:i+4]
   if pixels[i+3]:counts[f][owner]+=1
 assert all(all(n>0 for n in row) for row in counts)
 assert bytes(sum(layer[i] for layer in layers) for i in range(len(pixels)))==pixels
 crush=subprocess.check_output(['xcrun','--find','pngcrush'],text=True).strip();records=[];decoded=[];gaps={}
 for name,layer in zip(NAMES,layers):
  path=OUT/(name+'.png');path.write_bytes(png_encode(w,h,bytes(layer)))
  dest=ROOT/'Resources/Images'/(name+'.png')
  with tempfile.TemporaryDirectory(prefix='md-ah10-part-') as td:
   encoded=Path(td)/(name+'.png')
   subprocess.run([crush,'-iphone','-f','0',str(path),str(encoded)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   m,p=png_pixels(encoded.read_bytes());assert m['cgbi'] and p[3::4]==layer[3::4]
   decoded.append(p)
   dest.write_bytes(encoded.read_bytes())
  records.append({'name':name,'preparedImage':str(path.relative_to(ROOT)),'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'runtimeSHA256':hashlib.sha256(dest.read_bytes()).hexdigest(),'cellSize':[76,112],'tileCount':7})
 with tempfile.TemporaryDirectory(prefix='md-ah10-source-') as td:
  encoded=Path(td)/'source.png'
  subprocess.run([crush,'-iphone','-f','0',str(SOURCE),str(encoded)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  _,before=png_pixels(encoded.read_bytes())
  recomposed=bytes(sum(layer[i] for layer in decoded) for i in range(len(before)))
  runtime_diff=sum(a!=b for a,b in zip(before,recomposed));assert runtime_diff==0
 for name,layer,muzzle in zip(NAMES[1:],layers[1:],[(55,22),(74,74)]):
  points=[(x,y) for y in range(h) for x in range(76) if layer[(y*w+x)*4+3]]
  row=max(x for x,y in points if y==muzzle[1])
  gaps[name]={'oldMuzzlePixel':muzzle,'forwardAlphaEdge':max(x for x,y in points),'forwardAlphaOnMuzzleRow':row,'nearestAlphaPixel':min(points,key=lambda p:(p[0]-muzzle[0])**2+(p[1]-muzzle[1])**2),'gapOnMuzzleRow':muzzle[0]-row}
 report={'decodedRuntimeDifferentChannels':runtime_diff,'existingMuzzleArtGaps':gaps,'source':str(SOURCE.relative_to(ROOT)),'sourceSHA256':hashlib.sha256(raw).hexdigest(),'preparedRecompositionByteIdentical':True,'overlappingNonzeroAlphaPixels':0,'counts':counts,'rocketPolygons':ROCKET,'airstrikeBoxes':AIRSTRIKE,'records':records,'mountBehavior':'Existing WorldCombat embedded body angle and recoil zero retained; no independently rotating weapon or support backfill added.','muzzlePixelsNeutral':{'AHeavyRocket':[55,22],'AHeavyAirStrike':[74,74]},'runtimeVisualVerification':'pending'}
 (OUT/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps(report))
if __name__=='__main__':run()
