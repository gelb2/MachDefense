#!/usr/bin/env python3
"""Encode authored, bank-specific Beluga reinforcement parts without repainting."""
import hashlib
import json
import subprocess
import tempfile
from pathlib import Path
from prepare_resources import png_pixels

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'Art/Aircraft/upgrades-20261109/beluga'

def run():
    original = ROOT / 'Art/Aircraft/banking-correction-20261109/prepared/ATransportBody.png'
    meta, body = png_pixels(original.read_bytes())
    assert (meta['width'], meta['height']) == (980, 131)
    paths = sorted((ART / 'rgba').glob('*.png'))
    expected = {f'ATransportRoot{side}_{lv:02}' for side in (0, 1) for lv in range(1, 11)}
    assert {p.stem for p in paths} == expected
    crush = subprocess.check_output(['xcrun', '--find', 'pngcrush'], text=True).strip()
    rows = []
    levels = {side: [] for side in (0, 1)}
    for path in paths:
        side = int(path.stem[len('ATransportRoot')])
        y0 = 28 if side == 0 else 76
        meta, pixels = png_pixels(path.read_bytes())
        assert (meta['width'], meta['height']) == (224, 29), path.name
        cells = [b''.join(pixels[(y*224+i*32)*4:(y*224+(i+1)*32)*4] for y in range(29)) for i in range(7)]
        hashes = [hashlib.sha256(c).hexdigest() for c in cells]
        assert len(set(hashes)) == 7, path.name
        assert not any(cells[i][(y*32+x)*4+3] and not body[((y+y0)*980+i*140+x+64)*4+3]
                       for i in range(7) for y in range(29) for x in range(32)), path.name
        levels[side].append(tuple(hashes))
        dest = ROOT / 'Resources/Images' / path.name
        with tempfile.TemporaryDirectory(prefix='md-beluga-parts-') as tmp:
            encoded = Path(tmp) / path.name
            subprocess.run([crush, '-iphone', '-f', '0', str(path), str(encoded)], check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            result, decoded = png_pixels(encoded.read_bytes())
            assert result['cgbi'] and decoded[3::4] == pixels[3::4]
            assert all(decoded[i+c] == pixels[i+c] for i in range(0, len(pixels), 4)
                       if pixels[i+3] == 255 for c in range(3))
            dest.write_bytes(encoded.read_bytes())
        rows.append({'name': path.stem, 'sourceSHA256': hashlib.sha256(path.read_bytes()).hexdigest(),
                     'runtimeSHA256': hashlib.sha256(dest.read_bytes()).hexdigest(),
                     'size': [224, 29], 'cell': [32, 29], 'bodyTopLeft': [64, y0],
                     'cgbi': True, 'alphaPreserved': True, 'outsideApprovedAlphaPixels': 0,
                     'poseHashes': hashes, 'potRGBABytes': 256*32*4})
    assert all(len(set(hashes)) == 10 for hashes in levels.values())
    report = {'method': 'Bank-specific approved metal fragments assembled into separate reinforcement parts',
              'source': str(original.relative_to(ROOT)), 'sourceSHA256': hashlib.sha256(original.read_bytes()).hexdigest(),
              'records': rows, 'potRGBABytes': sum(r['potRGBABytes'] for r in rows)}
    (ART / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'parts': len(rows), 'poses': len(rows)*7, 'potRGBABytes': report['potRGBABytes']}))

if __name__ == '__main__':
    run()
