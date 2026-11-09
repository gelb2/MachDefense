#!/usr/bin/env python3
"""Split adopted Beluga bank pixels into fixed parts without repainting/resizing.

Preparation only: every input pixel belongs to exactly one part. The authored
body is immutable; source pixels reassemble byte-identically. Runtime consumption
requires the accompanying assembly patch and bank-tile selection for fixed parts.
"""
import hashlib, json, subprocess, tempfile
from pathlib import Path
from prepare_resources import png_pixels, png_encode
ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'Art/Aircraft/parts-separation-20261109/beluga'
SOURCE=ROOT/'Art/Aircraft/banking-correction-20261109/prepared/ATransportBody.png'
NAMES=('ATransportBody','ATransportWingL','ATransportWingR','ATransportEngine')
# Per-pose seam bands measured on the adopted sheet, nose right. The two
# nacelle regions are one fixed engine assembly; wing left is screen-top.
SEAMS=[
 {'wingX':[64,104],'topEnd':51,'bottomStart':82,'engines':[[49,49,75,61],[49,72,75,85]]},
 {'wingX':[65,104],'topEnd':49,'bottomStart':79,'engines':[[49,47,74,59],[49,71,74,83]]},
 {'wingX':[65,105],'topEnd':48,'bottomStart':77,'engines':[[49,46,74,58],[49,69,74,81]]},
 {'wingX':[65,106],'topEnd':47,'bottomStart':76,'engines':[[49,45,74,57],[49,68,74,80]]},
 {'wingX':[62,103],'topEnd':51,'bottomStart':82,'engines':[[49,49,75,61],[49,73,75,85]]},
 {'wingX':[60,103],'topEnd':53,'bottomStart':84,'engines':[[49,51,75,63],[49,75,75,87]]},
 {'wingX':[58,103],'topEnd':55,'bottomStart':86,'engines':[[49,53,75,65],[49,77,75,89]]},
]
def run():
 ART.mkdir(parents=True,exist_ok=True)
 original=SOURCE.read_bytes();meta,pixels=png_pixels(original)
 w,h=meta['width'],meta['height'];assert (w,h)==(980,131)
 layers=[bytearray(len(pixels)) for _ in NAMES];counts=[[0]*4 for _ in range(7)]
 for y in range(h):
  for x in range(w):
   frame,local=x//140,x%140;s=SEAMS[frame];part=0
   if any(a<=local<c and b<=y<d for a,b,c,d in s['engines']):part=3
   elif s['wingX'][0]<=local<s['wingX'][1]:
    if y<s['topEnd']:part=1
    elif y>=s['bottomStart']:part=2
   i=(y*w+x)*4;layers[part][i:i+4]=pixels[i:i+4]
   if pixels[i+3]:counts[frame][part]+=1
 recomposed=bytes(sum(layer[i] for layer in layers) for i in range(len(pixels)))
 assert recomposed==pixels
 assert all(sum(bool(layer[i+3]) for layer in layers)<=1 for i in range(0,len(pixels),4))
 assert all(all(n>0 for n in row) for row in counts)
 crush=subprocess.check_output(['xcrun','--find','pngcrush'],text=True).strip()
 records=[];offsets={};restored=[]
 for name,layer in zip(NAMES,layers):
  box=[0,0,140,131]
  if name!='ATransportBody':
   points=[(x%140,y) for y in range(h) for x in range(w) if any(layer[(y*w+x)*4:(y*w+x)*4+4])]
   x0=min(x for x,y in points);x1=max(x for x,y in points)+1
   y0=min(y for x,y in points);y1=max(y for x,y in points)+1
   # Match original cell parity so parent socket offsets are whole pixels.
   # PartDefinition::socket truncates fractional coordinates.
   if (x1-x0)%2: x1+=1
   if (y1-y0)%2==0: y1+=1
   assert x1<=140 and y1<=131
   # Preserve longitudinal UV lattice: full-width bank frames avoid nearest
   # shadow sampling changes observed after X cropping on Chrome/WebKit.
   x0,x1=0,140
   box=[x0,y0,x1,y1]
  x0,y0,x1,y1=box;cw,ch=x1-x0,y1-y0;packed=bytearray(cw*7*ch*4);unpacked=bytearray(len(layer))
  for frame in range(7):
   for y in range(ch):
    start=((y+y0)*w+frame*140+x0)*4;target=(y*cw*7+frame*cw)*4
    packed[target:target+cw*4]=layer[start:start+cw*4]
    unpacked[start:start+cw*4]=packed[target:target+cw*4]
  assert unpacked==layer
  restored.append(unpacked)
  offset=[x0+cw/2-70,65.5-(y0+ch/2)]
  assert all(v==int(v) for v in offset)
  offsets[name]=offset
  prepared=ART/(name+'.png');prepared.write_bytes(png_encode(cw*7,ch,bytes(packed)))
  dest=ROOT/'Resources/Images'/(name+'.png')
  with tempfile.TemporaryDirectory(prefix='md-beluga-part-') as td:
   tmp=Path(td)/(name+'.png')
   subprocess.run([crush,'-iphone','-f','0',str(prepared),str(tmp)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   om,decoded=png_pixels(tmp.read_bytes());assert om['cgbi'] and decoded[3::4]==packed[3::4]
   assert all(decoded[i+c]==packed[i+c] for i in range(0,len(packed),4) if packed[i+3]==255 for c in range(3))
   dest.write_bytes(tmp.read_bytes())
  records.append({'name':name,'preparedImage':str(prepared.relative_to(ROOT)),'sha256':hashlib.sha256(prepared.read_bytes()).hexdigest(),'runtimeSHA256':hashlib.sha256(dest.read_bytes()).hexdigest(),'tileCount':7,'cellSize':[cw,ch],'commonCrop':box,'parentSocket':offset,'pivot':[0,0],'potRGBABytes':(1<<(cw*7-1).bit_length())*(1<<(ch-1).bit_length())*4})
 assert bytes(sum(layer[i] for layer in restored) for i in range(len(pixels)))==pixels
 manifest={'source':str(SOURCE.relative_to(ROOT)),'sourceSHA256':hashlib.sha256(original).hexdigest(),'method':'Disjoint pixel ownership at measured part seam bands; no repaint, resize, recolor or alpha changes','seams':SEAMS,'nonzeroAlphaPixelsByFrame':counts,'preparedRecompositionByteIdentical':True,'overlappingNonzeroAlphaPixels':0,'records':records,'runtimeVisualVerification':'pending','limitations':['Fixed parts preserve full source X span and crop only common Y bounds; runtime pixel comparison is required.','Engine split contains both existing nacelles as one fixed part.']}
 (ART/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
 patch={'parts':[{'name':r['name'],'tileCount':7,'preparedImage':r['preparedImage']} for r in records],
        'machine':'AircraftTransport','appendChildren':[{'name':n,'parentSocket':socket,'zOrder':z,'children':[]} for n,socket,z in [('ATransportWingL',2,1),('ATransportWingR',3,1),('ATransportEngine',4,2)]],
        'rootSocketsRequired':{str(i+2):offsets[n] for i,n in enumerate(NAMES[1:])},'partSocketsRequired':{n:[[0,0]] for n in NAMES[1:]},'bankTileNames':list(NAMES)}
 out=ROOT/'.local/beluga-assembly-patch.json';out.write_text(json.dumps(patch,indent=2)+'\n')
 print(json.dumps({'preparedRecompositionByteIdentical':True,'nonzeroAlphaOverlap':0,'files':4,'assemblyPatch':str(out)}))
if __name__=='__main__':run()
