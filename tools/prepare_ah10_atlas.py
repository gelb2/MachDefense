#!/usr/bin/env python3
"""Pack three existing AH-10 layers without changing their 76x112 cells.

The original PartsList/TileList atlas path keeps all three assembly nodes,
quads, sockets and layer order. Do not crop transparent full-canvas margins.
"""
import hashlib, json, subprocess, tempfile
from pathlib import Path
from prepare_resources import png_pixels, png_encode
ROOT = Path(__file__).resolve().parents[1]
NAMES = ('AHeavyBody', 'AHeavyRocket', 'AHeavyAirStrike')

def run():
    source = ROOT/'Art/Aircraft/parts-separation-20261109/ah10'
    output = source/'atlas'
    output.mkdir(exist_ok=True)
    layers = []
    records = []
    crush = subprocess.check_output(['xcrun', '--find', 'pngcrush'], text=True).strip()
    for name in NAMES:
        path = source/(name+'.png')
        meta, pixels = png_pixels(path.read_bytes())
        assert (meta['width'], meta['height']) == (532, 112)
        layers.append(pixels)
        records.append({'part': name, 'source': str(path.relative_to(ROOT)),
                        'sourceSHA256': hashlib.sha256(path.read_bytes()).hexdigest()})
    joined = b''.join(layer[y*532*4:(y+1)*532*4] for y in range(112) for layer in layers)
    prepared = output/'AHeavyBody.png'
    prepared.write_bytes(png_encode(1596, 112, joined))
    dest = ROOT/'Resources/Images/AHeavyBody.png'
    with tempfile.TemporaryDirectory(prefix='md-ah10-atlas-') as td:
        encoded = Path(td)/'atlas.png'
        subprocess.run([crush, '-iphone', '-f', '0', str(prepared), str(encoded)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        meta, decoded = png_pixels(encoded.read_bytes())
        assert meta['cgbi']
        for index, record in enumerate(records):
            prior = Path(td)/(record['part']+'.png')
            subprocess.run([crush, '-iphone', '-f', '0', str(ROOT/record['source']), str(prior)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            _, old = png_pixels(prior.read_bytes())
            strip = b''.join(decoded[(y*1596+index*532)*4:(y*1596+(index+1)*532)*4] for y in range(112))
            assert strip == old, record['part']
            record.update(baseTile=index*7, decodedStripSHA256=hashlib.sha256(strip).hexdigest())
        dest.write_bytes(encoded.read_bytes())
    report = {'atlas': str(prepared.relative_to(ROOT)), 'runtime': str(dest.relative_to(ROOT)),
              'runtimeSHA256': hashlib.sha256(dest.read_bytes()).hexdigest(),
              'dimensions': [1596,112], 'grid': [21,1], 'cellSize': [76,112],
              'decodedStripsIdentical': True, 'parts': records,
              'textureBackingBeforeBytes': 1572864, 'textureBackingAfterBytes': 1048576,
              'textureBackingSavedBytes': 524288, 'GPUTimeImprovementMeasured': False}
    (output/'manifest.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report))

if __name__ == '__main__':
    run()
