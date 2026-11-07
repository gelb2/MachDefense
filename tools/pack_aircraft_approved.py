#!/usr/bin/env python3
"""Pack preserved approved artwork. No repainting, stretching or new silhouettes."""
from pathlib import Path
import hashlib, json, subprocess
from prepare_resources import png_pixels

root = Path(__file__).resolve().parents[1]
out = root / 'Resources/Images'
records = []
# Central fuselage measurements exclude tailplane, engines and weapon pods.
# Heavy uses the complete body incl. fixed rotor mounts, excluding rotor blades.
specs = [
 ('ALightBody', 'parts/light-modular-v3.png', (227,34,355,1155), 250, 34*2/3),
 ('AHeavyBody', 'parts/heavy-body-v4.png', (51,668,534,491), 534, 104),
 ('ADuctBody', 'concepts/orca-gray-parts-v2.png', (50,120,530,580), 150, 34*2/3),
 ('ATransportBody', 'concepts/orca-bomber-parts-v1.png', (40,40,550,690), 129, 34*2/3),
]
for name, source, box, measured, target in specs:
    src = root/'Art/Aircraft'/source
    x,y,w,h = box
    scale = target/measured
    dest = out/(name+'.png')
    subprocess.run(['magick',str(src),'-crop',f'{w}x{h}+{x}+{y}','+repage',
        '-filter','Lanczos','-resize',f'{round(w*scale)}x{round(h*scale)}',
        '-background','none','-rotate','90',f'PNG32:{dest}'],check=True)
    meta,pixels = png_pixels(dest.read_bytes()); W,H=meta['width'],meta['height']
    points=[(x,y) for y in range(H) for x in range(W) if pixels[(y*W+x)*4+3]>=16]
    xs,ys=zip(*points)
    records.append(dict(name=name,source=str(src.relative_to(root)),
        source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),crop_xywh=box,
        source_measured_width=measured,target_width=target,scale=scale,
        alpha_width_north=max(ys)-min(ys)+1,alpha_length_north=max(xs)-min(xs)+1,
        sha256=hashlib.sha256(dest.read_bytes()).hexdigest()))
# Colour-matched Orca weapons use the same recoloured sheet.
for name,box,height in [('ADuctGun','180x340+220+830',10),('ADuctMissile','180x320+850+860',10)]:
    subprocess.run(['magick',str(root/'Art/Aircraft/concepts/orca-gray-parts-v2.png'),
        '-crop',box,'+repage','-trim','+repage','-filter','Lanczos','-resize',f'x{height}',
        '-background','none','-rotate','90',f'PNG32:{out/(name+".png")}'],check=True)
subprocess.run(['magick',str(root/'Art/Aircraft/concepts/orca-bomber-parts-v1.png'),
    '-crop','250x250+125+960','+repage','-trim','+repage','-filter','Lanczos',
    '-resize','35x35','-background','none','-rotate','90',f'PNG32:{out/"ATransportFan.png"}'],check=True)
(root/'Art/Aircraft/runtime/approved-restoration.json').write_text(json.dumps(records,indent=2)+'\n')
print(json.dumps(records,indent=2))
