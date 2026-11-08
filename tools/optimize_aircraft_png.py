#!/usr/bin/env python3
"""Use Apple's PNG preparation on helicopter-only files; never edit original assets."""
import hashlib,json,subprocess,tempfile
from pathlib import Path
from prepare_resources import png_pixels
ROOT=Path(__file__).resolve().parents[1]
def run():
 source=ROOT/'Art/Aircraft/runtime/png-source';dest=ROOT/'Resources/Images'
 pngcrush=subprocess.check_output(['xcrun','--find','pngcrush'],text=True).strip();rows=[]
 for p in sorted(source.glob('*.png')):
  assert p.name.startswith(('ALight','AHeavy','ADuct','ATransport'))
  meta,before=png_pixels(p.read_bytes());assert not meta['cgbi']
  output=dest/p.name
  with tempfile.TemporaryDirectory(prefix='md-heli-png-') as td:
   temp=Path(td)/p.name
   subprocess.run([pngcrush,'-iphone','-f','0',str(p),str(temp)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   afterMeta,after=png_pixels(temp.read_bytes());assert afterMeta['cgbi'];assert (meta['width'],meta['height'])==(afterMeta['width'],afterMeta['height'])
   assert before[3::4]==after[3::4]
   assert all(before[i+c]==after[i+c] for i in range(0,len(before),4) if before[i+3]==255 for c in range(3))
   output.write_bytes(temp.read_bytes())
  rows.append({'image':p.name,'sourceSHA256':hashlib.sha256(p.read_bytes()).hexdigest(),'outputSHA256':hashlib.sha256(output.read_bytes()).hexdigest(),'size':[meta['width'],meta['height']],'inputBytes':p.stat().st_size,'outputBytes':output.stat().st_size,'cgbi':True,'alphaUnchanged':True,'opaqueRGBUnchanged':True})
 report={'tool':'Apple pngcrush -iphone -f 0','scope':'Helicopter-only build preparation; original images and renderer unchanged','textures':rows}
 (ROOT/'Art/Aircraft/runtime/png-optimization.json').write_text(json.dumps(report,indent=2)+'\n');print('Optimized',len(rows),'helicopter textures with Apple pngcrush')
if __name__=='__main__':run()
