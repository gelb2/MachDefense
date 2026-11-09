#!/usr/bin/env python3
"""Uniform crop/packing only. Central hull width excludes rotors/pods/weapons.
Generated source sheets remain intact; RGBA stays multi-tone/partial-alpha.
"""
from pathlib import Path
import subprocess,json,hashlib,math
from prepare_resources import png_pixels
root=Path(__file__).resolve().parents[1];out=root/'Resources/Images'
# Fuselage rectangles are explicit visual annotations of neutral source body.
# These are NOT the full alpha box, which includes tailplane/side engines.
specs=[
 ('ALight','helicopter-bank-v2.png',0,2,[698,142,140,174],23),
 ('AHeavy','gunship-bank-v3.png',0,1,[844,116,484,481],104),
 ('ADuct','orca-bank-v2.png',0,1,[970,165,231,326],23),
 ('ATransport','orca-bomber-bank-v2.png',0,1,[970,87,231,346],23),
]
records=[]
for prefix,file,row,rows,hull,target in specs:
 src=root/'Art/Aircraft/parts'/file;m,p=png_pixels(src.read_bytes());W,H=m['width'],m['height'];boxes=[]
 for col in range(3):
  pts=[(x,y) for y in range(row*H//rows,(row+1)*H//rows) for x in range(col*W//3,(col+1)*W//3) if p[(y*W+x)*4+3]>=16]
  xs,ys=zip(*pts);boxes.append((min(xs),min(ys),max(xs)-min(xs)+1,max(ys)-min(ys)+1))
 scale=target/hull[2]
 dims=[(round(w*scale),round(h*scale)) for x,y,w,h in boxes]
 cw=max(w for w,h in dims)+4;ch=max(h for w,h in dims)+4
 for col,suffix in [(0,'BankL'),(1,''),(2,'BankR')]:
  x,y,w,h=boxes[col];dw,dh=dims[col];dest=out/(prefix+'Body'+suffix+'.png')
  subprocess.run(['magick',str(src),'-crop',f'{w}x{h}+{x}+{y}','+repage','-filter','Lanczos','-resize',f'{dw}x{dh}','-background','none','-gravity','center','-extent',f'{cw}x{ch}','-rotate','90',f'PNG32:{dest}'],check=True)
 records.append(dict(prefix=prefix,source=str(src.relative_to(root)),source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),source_neutral_hull_xywh=hull,source_alpha_boxes=boxes,uniform_scale=scale,target_hull_width=target,packed_alpha_dimensions_north=dims,canvas_north=[cw,ch],note='Manual neutral-hull annotation; excludes outboard nacelles, weapons, rotor and tailplane.'))
path=root/'Art/Aircraft/runtime/body-revision-v2.json';path.write_text(json.dumps(records,indent=2)+'\n');print(path.read_text())
