#!/usr/bin/env python3
"""Prepare adopted Scout upgrade parts through the existing CgBI PNG path.

RGBA inputs are already game-scale authored parts. Do not resize, repaint or
change alpha here. Definitions and weapon aliases remain in assembly.json.
"""
import hashlib,json,subprocess,tempfile
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
if __name__=='__main__':run()
