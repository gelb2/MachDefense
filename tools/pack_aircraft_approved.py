#!/usr/bin/env python3
"""Pack preserved approved artwork. No repainting, stretching or new silhouettes."""
from pathlib import Path
import hashlib, json, subprocess
from prepare_resources import png_pixels

root = Path(__file__).resolve().parents[1]
out = root / 'Resources/Images'
records = []
# Central fuselage measurements exclude tailplane, engines and weapon pods.
# Heavy central body excludes the fixed lateral rotor mounts (user 2026-11-08).
specs = [
 ('ALightBody', 'parts/light-modular-v3.png', (227,34,355,1155), 250, 41*2/3),
 ('AHeavyBody', 'parts/heavy-body-v4.png', (51,668,534,491), 250, 104*3/4),
 ('ADuctBody', 'concepts/orca-gray-parts-v2.png', (50,120,530,580), 150, 41*2/3),
 ('ATransportBody', 'concepts/orca-bomber-parts-v1.png', (40,40,550,690), 129, 41*2/3),
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
# Preserve the selected weapon drawings at the body's revised scale.
for name,box,size in [('ALightGun','182x330+834+211','9x16'),('ALightMissile','182x284+834+750','9x14')]:
    subprocess.run(['magick',str(root/'Art/Aircraft/parts/light-modular-v3.png'),
        '-crop',box,'+repage','-filter','Lanczos','-resize',size,
        '-background','none','-rotate','90',f'PNG32:{out/(name+".png")}'],check=True)
# Gunship central body width is 3/4 of E_Tank32_Body; mounts and weapons
# grow with it, preserving the approved design's proportions.
for name,box,size in [('AHeavyCannon','147x342+1002+105','16x35'),('AHeavyMissile','123x208+1014+530','16x27'),('AHeavyGun','167x251+994+808','19x29')]:
    subprocess.run(['magick',str(root/'Art/Aircraft/parts/heavy-modular-v3.png'),
        '-crop',box,'+repage','-filter','Lanczos','-resize',size,
        '-background','none','-rotate','90',f'PNG32:{out/(name+".png")}'],check=True)
subprocess.run(['magick',str(root/'Art/Aircraft/parts/heavy-body-v4.png'),
    '-crop','480x480+710+685','+repage','-trim','+repage','-filter','Lanczos',
    '-resize','66x66','-background','none','-rotate','90',f'PNG32:{out/"AHeavyRotor.png"}'],check=True)
# Colour-matched Orca weapons use the same recoloured sheet.
for name,box,height in [('ADuctGun','180x340+220+830',12),('ADuctMissile','180x320+850+860',12)]:
    subprocess.run(['magick',str(root/'Art/Aircraft/concepts/orca-gray-parts-v2.png'),
        '-crop',box,'+repage','-trim','+repage','-filter','Lanczos','-resize',f'x{height}',
        '-background','none','-rotate','90',f'PNG32:{out/(name+".png")}'],check=True)
subprocess.run(['magick',str(root/'Art/Aircraft/concepts/orca-bomber-parts-v1.png'),
    '-crop','250x250+125+960','+repage','-trim','+repage','-filter','Lanczos',
    '-resize','42x42','-background','none','-rotate','90',f'PNG32:{out/"ATransportFan.png"}'],check=True)
# The selected crossed-blade shape, sampled from the preserved high-resolution
# source. The former 26px rotor belonged to the old 57px body, not this 126px body.
subprocess.run(['magick',str(root/'Art/Aircraft/parts/heavy-body-v4.png'),
    '-crop','455x455+713+90','+repage','-trim','+repage','-filter','Lanczos',
    '-resize','60x60','-background','none','-rotate','90',f'PNG32:{out/"ALightRotor.png"}'],check=True)
(root/'Art/Aircraft/runtime/approved-restoration.json').write_text(json.dumps(records,indent=2)+'\n')
print(json.dumps(records,indent=2))
