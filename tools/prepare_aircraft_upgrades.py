#!/usr/bin/env python3
"""Prepare adopted Scout upgrade parts through the existing CgBI PNG path.

RGBA inputs are already game-scale authored parts. Do not resize, repaint or
change alpha here. Definitions and weapon aliases remain in assembly.json.
"""
import argparse,hashlib,json,subprocess,tempfile
from pathlib import Path
from prepare_resources import png_pixels
ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'Art/Aircraft/upgrades-20261109/scout'
def run():
 body_meta,body_pixels=png_pixels((ROOT/'Art/Aircraft/banking-correction-20261109/prepared/ALightBody.png').read_bytes());assert(body_meta['width'],body_meta['height'])==(448,47)
 cover_outside=0
 crush=subprocess.check_output(['xcrun','--find','pngcrush'],text=True).strip();records=[];cover_levels={}
 for path in sorted((ART/'rgba').glob('*.png')):
  name=path.stem;meta,pixels=png_pixels(path.read_bytes())
  assert name.startswith(('ALightCover_','ALightGun_','ALightMissile_')),name
  expected=(112,17) if name.startswith('ALightCover_') else (14,7) if name.startswith('ALightGun_') else (13,8)
  assert (meta['width'],meta['height'])==expected,(name,meta)
  if name.startswith('ALightCover_'):
   cells=[b''.join(pixels[(y*112+i*16)*4:(y*112+(i+1)*16)*4]for y in range(17))for i in range(7)]
   hashes=[hashlib.sha256(c).hexdigest()for c in cells];assert len(set(hashes))==7
   cover_levels[name]=hashes
   outside=sum(bool(cells[i][(y*16+x)*4+3]) and not body_pixels[((y+14)*448+i*64+x+25)*4+3] for i in range(7) for y in range(17) for x in range(16))
   assert outside==0;cover_outside+=outside
  dest=ROOT/'Resources/Images'/path.name
  with tempfile.TemporaryDirectory(prefix='md-aircraft-upgrade-') as td:
   encoded=Path(td)/path.name
   subprocess.run([crush,'-iphone','-f','0',str(path),str(encoded)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   encoded_meta,decoded=png_pixels(encoded.read_bytes());assert encoded_meta['cgbi'] and decoded[3::4]==pixels[3::4]
   assert all(decoded[i+c]==pixels[i+c]for i in range(0,len(pixels),4)if pixels[i+3]==255 for c in range(3))
   dest.write_bytes(encoded.read_bytes())
  records.append({'name':name,'source':str(path.relative_to(ROOT)),'sourceSHA256':hashlib.sha256(path.read_bytes()).hexdigest(),'runtimeSHA256':hashlib.sha256(dest.read_bytes()).hexdigest(),'size':expected,'cgbi':True,'alphaPreserved':True,'potRGBABytes':(1<<(expected[0]-1).bit_length())*(1<<(expected[1]-1).bit_length())*4})
 assert len(cover_levels)==9 and len({tuple(h)for h in cover_levels.values()})==9
 report={'scope':'Scout level 2–10 resources listed below; level1 preserved','coverPoseHashes':cover_levels,'coverRegistration':{'actualBodyCell':[64,47],'coverCell':[16,17],'bodyTopLeft':[25,14],'bodyLocalCenter':[1,1],'socket0':[-1,-1],'outsideApprovedAlphaPixels':cover_outside},'weaponHousingsPrepared':[r['name']for r in records if not r['name'].startswith('ALightCover_')],'records':records,'potRGBABytes':sum(r['potRGBABytes']for r in records),'runtimeVisualVerification':'pending'}
 (ART/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
def prepare_cheyenne():
 art=ROOT/'Art/Aircraft/upgrades-20261109/cheyenne'
 body_meta,body=png_pixels((ROOT/'Art/Aircraft/banking-20261109/prepared/ADuctBody.png').read_bytes())
 assert (body_meta['width'],body_meta['height'])==(770,88)
 original_meta,original=png_pixels((ROOT/'Resources/Images/ADuctMissile.png').read_bytes())
 assert (original_meta['width'],original_meta['height'])==(18,13)
 crush=subprocess.check_output(['xcrun','--find','pngcrush'],text=True).strip()
 records=[];level_hashes=[]
 paths=sorted((art/'rgba').glob('*.png'))
 assert {p.stem for p in paths}=={f'{prefix}_{level:02}' for prefix in ('ADuctCover','ADuctMissile') for level in range(1,11)}
 for path in paths:
  meta,pixels=png_pixels(path.read_bytes());cover=path.stem.startswith('ADuctCover_')
  expected=(196,14) if cover else (18,13)
  assert (meta['width'],meta['height'])==expected,(path.name,meta)
  if cover:
   cells=[b''.join(pixels[(y*196+i*28)*4:(y*196+(i+1)*28)*4] for y in range(14)) for i in range(7)]
   hashes=[hashlib.sha256(cell).hexdigest() for cell in cells]
   assert len(set(hashes))==7,path.name
   assert not any(cells[i][(y*28+x)*4+3] and not body[((y+37)*770+i*110+x+35)*4+3] for i in range(7) for y in range(14) for x in range(28)),path.name
   level_hashes.append(tuple(hashes))
  else:
   assert pixels[3::4]==original[3::4],path.name
  dest=ROOT/'Resources/Images'/path.name
  with tempfile.TemporaryDirectory(prefix='md-cheyenne-upgrade-') as td:
   encoded=Path(td)/path.name
   subprocess.run([crush,'-iphone','-f','0',str(path),str(encoded)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   encoded_meta,decoded=png_pixels(encoded.read_bytes())
   assert encoded_meta['cgbi'] and decoded[3::4]==pixels[3::4]
   assert all(decoded[i+c]==pixels[i+c] for i in range(0,len(pixels),4) if pixels[i+3]==255 for c in range(3))
   if not cover:
    assert all(decoded[(y*18+x)*4:(y*18+x+1)*4]==original[(y*18+x)*4:(y*18+x+1)*4] for y in range(13) for x in range(8,18)),path.name
   dest.write_bytes(encoded.read_bytes())
  records.append({'name':path.stem,'source':str(path.relative_to(ROOT)),'sourceSHA256':hashlib.sha256(path.read_bytes()).hexdigest(),'runtimeSHA256':hashlib.sha256(dest.read_bytes()).hexdigest(),'size':expected,'cgbi':True,'alphaPreserved':True,'potRGBABytes':(1<<(expected[0]-1).bit_length())*(1<<(expected[1]-1).bit_length())*4})
 assert len(set(level_hashes))==10
 report={'scope':'Cheyenne level 1–10 independent armor and missile housings','coverRegistration':{'bodyCell':[110,88],'coverCell':[28,14],'bodyTopLeft':[35,37],'socket0':[6,0],'outsideApprovedAlphaPixels':0},'records':records,'potRGBABytes':sum(r['potRGBABytes'] for r in records),'runtimeVisualVerification':'separate evidence report'}
 (art/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({'family':'cheyenne','parts':len(records),'potRGBABytes':report['potRGBABytes']}))

if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('--family',choices=('scout','cheyenne'),default='scout')
 args=parser.parse_args()
 prepare_cheyenne() if args.family=='cheyenne' else run()
