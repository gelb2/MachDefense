#!/usr/bin/env python3
"""Compare delivered sprites, without changing any runtime image."""
from PIL import Image,ImageDraw,ImageFont
from pathlib import Path
import numpy as np,json,hashlib
from prepare_resources import png_pixels
root=Path(__file__).resolve().parents[1]
images=root/'Resources/Images';out=root/'docs/research/turret-art-review-20261107';out.mkdir(exist_ok=True)
rows=[('body','Turret_b'),('rifle','Turret_gun'),('missile','Turret_msl'),('vulcan','Turret_mg'),('ion','Turret_ion'),('laser','Turret_lsr'),('plasma','Turret_pla')]
levels=[1,3,5,6,8,10];records=[]
font=ImageFont.truetype('/System/Library/Fonts/Menlo.ttc',16)
cell_width=max(Image.open(p).width for p in images.glob('Turret_*.png'))*4+24
board=Image.new('RGB',(cell_width*6,2400),(65,70,73));d=ImageDraw.Draw(board)
d.text((15,10),'ORIGINAL L1 / L3 / L5                     ADDED L6 / L8 / L10 — 4x nearest-neighbor',font=font,fill='white')
for y,(family,prefix) in enumerate(rows):
 for level in range(1,11):
  path=images/f'{prefix}_{level:02}.png';im=Image.open(path).convert('RGBA');a=np.array(im);mask=a[:,:,3]>=224;rgb=a[:,:,:3][mask].astype(float);v=rgb@np.array([.2126,.7152,.0722]);cmax=rgb.max(1);cmin=rgb.min(1);sat=np.divide(cmax-cmin,cmax,out=np.zeros_like(cmax),where=cmax!=0)
  original=root/'Resources/OriginalImages'/path.name
  source_match=None
  if original.exists():
   meta,pixels=png_pixels(original.read_bytes());straight=bytearray(pixels)
   if meta['cgbi']:
    for i in range(0,len(straight),4):
     alpha=straight[i+3]
     for c in range(3):straight[i+c]=min(255,(straight[i+c]*255+alpha//2)//alpha) if alpha else 0
   source_match=bytes(straight)==im.tobytes()
  records.append(dict(source_decoded_pixels_identical=source_match,family=family,level=level,file=str(path.relative_to(root)),sha256=hashlib.sha256(path.read_bytes()).hexdigest(),original_identical=original.exists() and original.read_bytes()==path.read_bytes(),canvas=im.size,bounds=im.getbbox(),opaque_pixels=int(mask.sum()),mean_luma=round(float(v.mean()),2),median_luma=round(float(np.median(v)),2),dark_pct=round(float((v<64).mean()*100),2),bright_pct=round(float((v>=160).mean()*100),2),saturation=round(float(sat.mean()),3),partial_alpha_pct=round(float(((a[:,:,3]>0)&(a[:,:,3]<224)).sum()/max(1,(a[:,:,3]>0).sum())*100),2)))
  if level in levels:
   x=levels.index(level);left=x*cell_width;top=40+y*335;d.text((left+8,top),f'{family} L{level}',font=font,fill=(245,245,245));scale=4
   scaled=im.resize((im.width*scale,im.height*scale),Image.Resampling.NEAREST);board.paste(scaled,(left+(cell_width-scaled.width)//2,top+40+(280-scaled.height)//2),scaled)
board.save(out/'original-vs-added.png');(out/'measurements.json').write_text(json.dumps(records,indent=2)+'\n')
for family,prefix in rows:
 old=next(r for r in records if r['family']==family and r['level']==5);new=[r for r in records if r['family']==family and r['level']>5]
 print(family,'L5 luma',old['mean_luma'],'new',round(sum(r['mean_luma'] for r in new)/5,1),'dark%',old['dark_pct'],round(sum(r['dark_pct'] for r in new)/5,1),'bright%',old['bright_pct'],round(sum(r['bright_pct'] for r in new)/5,1))
