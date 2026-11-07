#!/usr/bin/env python3
"""Crop and uniformly pack generated body poses; preserve RGBA. No repainting."""
from pathlib import Path
import subprocess
from prepare_resources import png_pixels
root=Path(__file__).resolve().parents[1]
src=root/'Art/Aircraft/parts/helicopter-bank-v1.png';meta,p=png_pixels(src.read_bytes());w,h=meta['width'],meta['height'];out=root/'Resources/Images'
for row,prefix,maxW,maxH in [(0,'ALight',20,57),(1,'AHeavy',90,87)]:
 boxes=[]
 for col in range(3):
  points=[(x,y) for y in range(row*h//2,(row+1)*h//2) for x in range(col*w//3,(col+1)*w//3) if p[(y*w+x)*4+3]>=16]
  xs,ys=zip(*points);boxes.append((min(xs),min(ys),max(xs)-min(xs)+1,max(ys)-min(ys)+1))
 neutral=boxes[1];scale=min(maxW/neutral[2],maxH/neutral[3])
 for col,suffix in [(0,'BankL'),(1,''),(2,'BankR')]:
  x,y,bw,bh=boxes[col];targetW=round(bw*scale);targetH=round(bh*scale)
  subprocess.run(['magick',str(src),'-crop',f'{bw}x{bh}+{x}+{y}','+repage','-filter','Lanczos','-resize',f'{targetW}x{targetH}','-background','none','-gravity','center','-extent',f'{maxW+4}x{maxH+4}','-rotate','90',f'PNG32:{out/(prefix+"Body"+suffix+".png")}'],check=True)
  print(prefix,suffix or 'neutral',boxes[col],targetW,targetH)
