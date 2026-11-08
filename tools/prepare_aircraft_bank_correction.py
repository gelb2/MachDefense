#!/usr/bin/env python3
"""Package selected drawn banks around fixed original sockets; preserve neutral pixels."""
import hashlib
import json
import subprocess
import tempfile
from pathlib import Path
from prepare_resources import png_pixels
from pack_aircraft_banking import bounds

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'Art/Aircraft/banking-correction-20261109'
PAINT = r'''
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Core/Presentation/AircraftPaint.hpp"
#include <fstream>
int main(int argc,char**argv) {
 if(argc!=4)return 2;
 int w,h,c;auto*p=stbi_load(argv[1],&w,&h,&c,4);if(!p)return 3;
 // Reuse the approved colour preparation. Existing drawn markings are retained.
 md::prepareAircraftPaint(argv[3],p,std::size_t(w)*h);
 std::ofstream f(argv[2],std::ios::binary);
 f<<"P7\nWIDTH "<<w<<"\nHEIGHT "<<h<<"\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
 f.write(reinterpret_cast<char*>(p),std::size_t(w)*h*4);
 stbi_image_free(p);return f?0:4;
}
'''

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def run():
    recipe = json.loads((ART/'recipe.json').read_text())
    out = ART/'prepared'
    out.mkdir(exist_ok=True)
    records = []
    with tempfile.TemporaryDirectory(prefix='md-bank-correction-') as directory:
        temp = Path(directory)
        (temp/'paint.cpp').write_text(PAINT)
        subprocess.run(['c++','-std=c++20','-O2','-I'+str(ROOT/'Sources'),
                        '-I'+str(ROOT/'Modules/ThirdParty/stb'),str(temp/'paint.cpp'),
                        '-o',str(temp/'paint')],check=True)
        for entry in recipe['aircraft']:
            body = entry['body']
            assert body in {'ALightBody','ATransportBody'}
            source = ROOT/entry['source']
            neutral = ROOT/'Art/Aircraft/runtime/png-source'/(body+'.png')
            sm,pixels = png_pixels(source.read_bytes())
            nm,original = png_pixels(neutral.read_bytes())
            boxes = [bounds(pixels,sm['width'],(
                i%3*sm['width']//3,i//3*sm['height']//2,
                (i%3+1)*sm['width']//3,(i//3+1)*sm['height']//2)) for i in range(6)]
            scale = nm['width']/max(y1-y0 for x0,y0,x1,y1 in boxes)
            px,py = entry['padding']
            cw,ch = nm['width']+px*2,nm['height']+py*2
            sx,sy = entry['rotorSocket']
            target = (cw/2+sx,ch/2-sy)
            pam,painted = temp/'paint.pam',temp/'paint.png'
            subprocess.run([str(temp/'paint'),str(source),str(pam),body],check=True)
            subprocess.run(['magick',str(pam),'PNG32:'+str(painted)],check=True)
            pm,paint_pixels = png_pixels(painted.read_bytes())
            assert paint_pixels[3::4] == pixels[3::4]
            cells,frames = [],[]
            for tile in range(7):
                frame = temp/f'{body}-{tile}.png'
                if tile == 0:
                    sprite,dx,dy = neutral,px,py
                else:
                    x0,y0,x1,y1 = boxes[tile-1]
                    ax,ay = entry['rotorCenters'][tile-1]
                    sprite = temp/f'{body}-{tile}-sprite.png'
                    subprocess.run(['magick',str(painted),'-crop',f'{x1-x0}x{y1-y0}+{x0}+{y0}',
                                    '+repage','-rotate','90','-filter','Lanczos','-resize',
                                    f'{scale*100:.8f}%','PNG32:'+str(sprite)],check=True)
                    # A fixed original socket anchors each drawn view. Do not recenter its silhouette.
                    ax,ay = (y1-ay)*scale,(ax-x0)*scale
                    dx,dy = round(target[0]-ax),round(target[1]-ay)
                    fm,_ = png_pixels(sprite.read_bytes())
                    assert dx>=0 and dy>=0 and dx+fm['width']<=cw and dy+fm['height']<=ch
                    frames.append({'tile':tile,'requestedRollDegrees':entry['angles'][tile-1],
                                   'cropXYXY':boxes[tile-1],'uniformScale':scale,'pasteOffset':[dx,dy],
                                   'drawnSize':[fm['width'],fm['height']],
                                   'rotorAnchorPixelError':[dx+ax-target[0],dy+ay-target[1]]})
                subprocess.run(['magick','-size',f'{cw}x{ch}','canvas:none',str(sprite),
                                '-geometry',f'+{dx}+{dy}','-compose','Copy','-composite',
                                'PNG32:'+str(frame)],check=True)
                cells.append(frame)
            dest = out/(body+'.png')
            subprocess.run(['magick',*[str(p) for p in cells],'+append','-strip','PNG32:'+str(dest)],check=True)
            dm,drawn = png_pixels(dest.read_bytes())
            preserved = b''.join(drawn[((y+py)*dm['width']+px)*4:
                                      ((y+py)*dm['width']+px+nm['width'])*4] for y in range(nm['height']))
            assert preserved == original
            records.append({'body':body,'source':entry['source'],'sourceSHA256':sha(source),
                            'neutralSHA256':sha(neutral),'preparedSHA256':sha(dest),
                            'cellSize':[cw,ch],'neutralSize':[nm['width'],nm['height']],
                            'neutralPadding':[px,py],'neutralPixelsExact':True,
                            'colourPreparationAlphaExact':True,'frames':frames})
    (ART/'manifest.json').write_text(json.dumps({'method':'Drawn roll views, original paint curves, uniform scale, fixed rotor sockets; neutral pixels retained','textures':records},indent=2)+'\n')
    print('Prepared selected aircraft banks:',', '.join(r['body'] for r in records))

if __name__ == '__main__':
    run()
