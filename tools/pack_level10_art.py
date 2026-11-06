#!/usr/bin/env python3
"""Pack approved sheets into original-size game textures. Requires Pillow.

No drawing, per-axis stretching or gameplay geometry changes. Only slice,
uniform downsample and position. Sources and every transform are recorded.
"""
from pathlib import Path
import hashlib, json
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'Art/Level10/20261107'
IMAGES = ROOT / 'Resources/Images'
CONFIG = [
    ('Turret_b_', 5, 5, 'body-geometry-v2.png', 'y'),
    ('Turret_gun_', 5, 5, 'rifle-geometry-v2.png', 'x'),
    ('Turret_msl_', 5, 5, 'missile-geometry-v2.png', 'x'),
    ('Turret_mg_', 5, 5, 'vulcan-geometry-v4.png', 'x'),
    ('Turret_ion_', 5, 5, 'ion-geometry-v3.png', 'x'),
    ('Turret_lsr_', 5, 5, 'laser-geometry-v3.png', 'x'),
    ('Turret_pla_', 5, 5, 'plasma-geometry-v2.png', 'x'),
    ('Base', 7, 3, 'base-body-v2.png', 'y'),
    ('BaseWpn', 7, 3, 'base-gun-v1.png', 'x'),
    ('Base{level}_Rader', 7, 3, 'base-radar-v2.png', 'x'),
]
def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def name(prefix, level):
    return prefix.format(level=f'{level:02}') if '{level}' in prefix else f'{prefix}{level:02}'
def bounds(image): return image.getchannel('A').point(lambda a: 255 if a > 32 else 0).getbbox()

records = []
for prefix, cap, count, filename, axis in CONFIG:
    old = IMAGES / (name(prefix, cap) + '.png')
    reference = Image.open(old).convert('RGBA')
    ob = bounds(reference)
    ow, oh = ob[2]-ob[0], ob[3]-ob[1]
    source = ART / filename
    sheet = Image.open(source).convert('RGBA')
    for index in range(count):
        left, right = round(sheet.width*index/count), round(sheet.width*(index+1)/count)
        cell = sheet.crop((left, 0, right, sheet.height))
        box = bounds(cell)
        assert box
        crop = cell.crop(box)
        scale = ow/crop.width if axis == 'x' else oh/crop.height
        size = (round(crop.width*scale), round(crop.height*scale))
        # These are integer raster dimensions of one uniform transform.
        # Retain the longitudinal size; never min-fit around an assumed pivot.
        assert abs(size[0]-ow) <= 2 and abs(size[1]-oh) <= 2, (filename,index,size,(ow,oh))
        sprite = crop.resize(size, Image.Resampling.LANCZOS)
        x = ob[2]-size[0] if axis == 'x' else round((ob[0]+ob[2]-size[0])/2)
        y = round((ob[1]+ob[3]-size[1])/2)
        assert x >= 0 and y >= 0 and x+size[0] <= reference.width and y+size[1] <= reference.height
        out = Image.new('RGBA', reference.size)
        out.alpha_composite(sprite, (x,y))
        output = IMAGES / (name(prefix,cap+index+1)+'.png')
        out.save(output, optimize=True)
        records.append(dict(output=str(output.relative_to(ROOT)),sha256=digest(output),
            original=str(old.relative_to(ROOT)),original_sha256=digest(old),
            source=str(source.relative_to(ROOT)),source_sha256=digest(source),
            source_rect=[left+box[0],box[1],box[2]-box[0],box[3]-box[1]],
            uniform_scale=scale,raster_size=list(size),offset=[x,y],canvas=list(reference.size),
            original_visible_size=[ow,oh],visible_bbox=list(bounds(out))))
manifest={'generator':'tools/pack_level10_art.py','filter':'Lanczos, uniform scale then integer pixel rounding',
          'images':records}
(ART/'runtime-art.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(f'{len(records)} additional-level textures packed; original canvases retained.')
