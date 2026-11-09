#!/usr/bin/env python3
"""Validate authored AH-10 upgrades and encode only their 40 added CgBI images."""
import hashlib
import json
import subprocess
import tempfile
from pathlib import Path
from prepare_resources import png_pixels

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'Art/Aircraft/upgrades-20261109/ah10'
WEAPONS = {'AHeavyGun': (25, 10, 10), 'AHeavyCannon': (18, 10, 7), 'AHeavyMissile': (16, 12, 6)}

def run():
    meta, body = png_pixels((ROOT / 'Resources/Images/AHeavyBody.png').read_bytes())
    assert (meta['width'], meta['height']) == (1596, 112)
    original = {name: png_pixels((ROOT / 'Resources/Images' / (name + '.png')).read_bytes())[1] for name in WEAPONS}
    crush = subprocess.check_output(['xcrun', '--find', 'pngcrush'], text=True).strip()
    expected = {f'{name}_{lv:02}' for name in ('AHeavyCover', *WEAPONS) for lv in range(1, 11)}
    paths = sorted((ART / 'rgba').glob('*.png'))
    assert {p.stem for p in paths} == expected
    records = []
    for path in paths:
        source_meta, pixels = png_pixels(path.read_bytes())
        name = path.stem.rsplit('_', 1)[0]
        cover = name == 'AHeavyCover'
        width, height = (168, 22) if cover else WEAPONS[name][:2]
        assert (source_meta['width'], source_meta['height']) == (width, height)
        if cover:
            cells = [b''.join(pixels[(y*width+i*24)*4:(y*width+(i+1)*24)*4] for y in range(height)) for i in range(7)]
            assert len({hashlib.sha256(c).hexdigest() for c in cells}) == 7
            assert not any(cells[i][(y*24+x)*4+3] and not body[((y+47)*1596+i*76+x+28)*4+3] for i in range(7) for y in range(22) for x in range(24))
        else:
            approved = original[name]
            assert pixels[3::4] == approved[3::4], path.name
            rear = WEAPONS[name][2]
            # Authored PNG is straight RGBA; existing CgBI RGB is premultiplied.
            # Compare the complete front bytes after encoding below.
            assert all(pixels[(y*width+x)*4:(y*width+x+1)*4] == approved[(y*width+x)*4:(y*width+x+1)*4] for y in range(height) for x in range(rear, width) if pixels[(y*width+x)*4+3] == 255), path.name
        dest = ROOT / 'Resources/Images' / path.name
        with tempfile.TemporaryDirectory(prefix='md-ah10-upgrade-') as td:
            encoded = Path(td) / path.name
            subprocess.run([crush, '-iphone', '-f', '0', str(path), str(encoded)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            encoded_meta, decoded = png_pixels(encoded.read_bytes())
            assert encoded_meta['cgbi'] and decoded[3::4] == pixels[3::4]
            assert all(decoded[i+c] == pixels[i+c] for i in range(0, len(pixels), 4) if pixels[i+3] == 255 for c in range(3))
            if not cover:
                assert all(decoded[(y*width+x)*4:(y*width+x+1)*4] == approved[(y*width+x)*4:(y*width+x+1)*4] for y in range(height) for x in range(rear, width)), path.name
            dest.write_bytes(encoded.read_bytes())
        records.append({'name': path.stem, 'sourceSHA256': hashlib.sha256(path.read_bytes()).hexdigest(), 'runtimeSHA256': hashlib.sha256(dest.read_bytes()).hexdigest(), 'size': [width, height], 'cgbi': True, 'alphaPreserved': True, 'frontPixelsPreserved': not cover, 'potRGBABytes': (1 << (width-1).bit_length()) * (1 << (height-1).bit_length()) * 4})
    for name in ('AHeavyCover', *WEAPONS):
        assert len({r['sourceSHA256'] for r in records if r['name'].startswith(name + '_')}) == 10
    report = {'scope': 'AH-10 ten levels, armor and three weapon housings only; rocket/airstrike atlas unchanged', 'registration': {'bodyCell': [76,112], 'coverCell': [24,22], 'bodyTopLeft': [28,47], 'selfSocket': [-2,2], 'outsideBodyAlphaPixels': 0}, 'records': records, 'potRGBABytes': sum(r['potRGBABytes'] for r in records)}
    (ART / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'parts': len(records), 'potRGBABytes': report['potRGBABytes']}))

if __name__ == '__main__':
    run()
