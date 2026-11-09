#!/usr/bin/env python3
"""Measure actual alpha silhouettes in C++ assembly probe frames (no shadows)."""
import sys, json, math
from pathlib import Path
from prepare_resources import png_pixels
root=Path(__file__).resolve().parents[1]
frames=Path(sys.argv[1]);out=Path(sys.argv[2]);cache={};result=[]
for aircraft in json.loads(frames.read_text())['aircraft']:
 sizes=[]
 for frame in aircraft['frames']:
  points=[]
  for sprite in frame:
   if sprite['shadow']:continue
   name=sprite['image']
   if name not in cache:
    meta,pixels=png_pixels((root/'Resources/Images'/name).read_bytes());W,H=meta['width'],meta['height'];edge=[]
    for y in range(H):
     xs=[x for x in range(W) if pixels[(y*W+x)*4+3]>=16]
     if xs:
      for x in (min(xs),max(xs)+1):edge.extend([(x,y),(x,y+1)])
    cache[name]=(W,H,edge)
   W,H,edge=cache[name];x,y,w,h=sprite['destination'];angle=math.radians(sprite['rotation']);c,t=math.cos(angle),math.sin(angle)
   for px,py in edge:
    u,v=(px/W-.5)*w,(py/H-.5)*h;points.append((x+w/2+u*c-v*t,y+h/2+u*t+v*c))
  sizes.append([max(x for x,y in points)-min(x for x,y in points),max(y for x,y in points)-min(y for x,y in points)])
 result.append(dict(name=aircraft['name'],orientation='north',alpha_threshold=16,rotor_phases=len(sizes),max_width=max(s[0] for s in sizes),max_height=max(s[1] for s in sizes)))
out.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
