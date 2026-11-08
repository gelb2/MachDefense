#!/usr/bin/env python3
"""Compare Apple-optimized helicopter PNGs through the production stb decoder."""
import argparse
import hashlib
import json
import subprocess
import tempfile
from pathlib import Path
from audit_aircraft_cgbi import CPP
from prepare_resources import png_pixels
from optimize_aircraft_png import prepared_source

ROOT = Path(__file__).resolve().parents[1]

def audit():
    rows = []
    with tempfile.TemporaryDirectory(prefix='md-aircraft-optimized-') as directory:
        work = Path(directory)
        (work/'decode.cpp').write_text(CPP)
        subprocess.run(['c++', '-std=c++20', '-O2', '-I'+str(ROOT/'Modules/ThirdParty/stb'),
                        str(work/'decode.cpp'), '-o', str(work/'decode')], check=True)
        for source in sorted((ROOT/'Art/Aircraft/runtime/png-source').glob('*.png')):
            source = prepared_source(source)
            destination = ROOT/'Resources/Images'/source.name
            metadata, pixels = png_pixels(source.read_bytes())
            optimized, premultiplied = png_pixels(destination.read_bytes())
            assert optimized['cgbi'] and not metadata['cgbi']
            assert (metadata['width'], metadata['height']) == (optimized['width'], optimized['height'])
            subprocess.run([str(work/'decode'), str(destination), str(work/'decoded'), 'straight'], check=True)
            decoded = (work/'decoded').read_bytes()
            assert len(decoded) == len(pixels)
            assert decoded[3::4] == pixels[3::4]
            opaque = [i for i in range(0, len(pixels), 4) if pixels[i+3] == 255]
            assert all(decoded[i+c] == pixels[i+c] for i in opaque for c in range(3))
            partial = [i for i in range(0, len(pixels), 4) if 0 < pixels[i+3] < 255]
            errors = [abs(decoded[i+c]-pixels[i+c]) * pixels[i+3]/255 for i in partial for c in range(3)]
            rows.append(dict(image=source.name, size=[metadata['width'], metadata['height']],
                             sha256=hashlib.sha256(destination.read_bytes()).hexdigest(),
                             alphaUnchanged=True, opaqueRGBUnchanged=True,
                             partialAlphaPixels=len(partial),
                             maxAlphaWeightedRGBDifference=max(errors, default=0),
                             sourceBytes=source.stat().st_size, optimizedBytes=destination.stat().st_size))
    return dict(decoder='Production stb with iPhone conversion and unpremultiply enabled',
                textures=rows, textureCount=len(rows),
                sourceBytes=sum(r['sourceBytes'] for r in rows),
                optimizedBytes=sum(r['optimizedBytes'] for r in rows),
                maxAlphaWeightedRGBDifference=max(r['maxAlphaWeightedRGBDifference'] for r in rows))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    result = audit()
    args.output.write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps({key:value for key,value in result.items() if key != 'textures'}))
