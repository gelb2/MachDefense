#!/usr/bin/env python3
"""Read-only PNG/CgBI audit; emit metrics, never copy the source archive."""
import argparse, collections, hashlib, json, struct, subprocess, tempfile, zipfile, zlib
from pathlib import Path
from prepare_resources import SOURCE_SHA256, png_pixels

ROOT = Path(__file__).resolve().parents[1]
SAMPLES = ['E_Tank32_Body.png', 'E_Tank32_Wpn.png', 'P_Body_18.png',
           'P_Wpn_Gun_106.png', 'P_Foot_32x16.png']

def chunks(blob):
    result = {}; pos = 8
    while pos < len(blob):
        n = struct.unpack_from('>I', blob, pos)[0]
        result[blob[pos+4:pos+8].decode('ascii')] = blob[pos+8:pos+8+n]
        pos += n+12
    return result

def compare(a, b, channels=4):
    assert len(a) == len(b)
    errors = [abs(a[i+c]-b[i+c]) for i in range(0,len(a),4) for c in range(channels)]
    return dict(differentChannels=sum(x!=0 for x in errors), maxDifference=max(errors,default=0))

CPP = r'''
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#include <fstream>
int main(int n,char**v){
 if(n!=4)return 2;stbi_convert_iphone_png_to_rgb(1);stbi_set_unpremultiply_on_load(1);
 int w,h,c;auto*p=stbi_load(v[1],&w,&h,&c,4);if(!p)return 3;
 if(v[3][0]=='p')for(int i=0;i<w*h;++i)for(int k=0;k<3;++k)p[i*4+k]=static_cast<unsigned char>(float(p[i*4+k])*(float(p[i*4+3])/255.f));
 std::ofstream f(v[2],std::ios::binary);f.write((char*)p,w*h*4);stbi_image_free(p);return f?0:4;
}
'''

def audit(source):
    assert hashlib.sha256(source.read_bytes()).hexdigest() == SOURCE_SHA256
    result = dict(sourceArchiveSHA256=SOURCE_SHA256,
                  scope='PNG metadata inventory; five original unit images; 28 helicopter images; CPU decode and interpolation model, not a GPU capture')
    with zipfile.ZipFile(source) as z, tempfile.TemporaryDirectory(prefix='md-cgbi-audit-') as temp:
        t=Path(temp);(t/'decode.cpp').write_text(CPP)
        subprocess.run(['c++','-std=c++20','-O2','-I'+str(ROOT/'Modules/ThirdParty/stb'),str(t/'decode.cpp'),'-o',str(t/'decode')],check=True)
        def decode(blob, premul=False):
            (t/'input.png').write_bytes(blob)
            subprocess.run([str(t/'decode'),str(t/'input.png'),str(t/'output.rgba'),'premultiplied' if premul else 'straight'],check=True)
            return (t/'output.rgba').read_bytes()
        inventory=collections.Counter()
        for n in z.namelist():
            if n.endswith('.png') and n.startswith('Payload/MachDefense.app/'):
                blob=z.read(n); c=chunks(blob);inventory['png']+=1
                for key in ['CgBI','iCCP','sRGB','gAMA','cHRM']:
                    inventory[key]+=key in c
        result['originalMetadata']=dict(inventory);rows=[]
        for name in SAMPLES:
            original=z.read('Payload/MachDefense.app/'+name);prepared=(ROOT/'Resources/Images'/name).read_bytes()
            meta,premul=png_pixels(original);pm,straight=png_pixels(prepared)
            assert (meta['width'],meta['height'])==(pm['width'],pm['height'])
            expected=bytearray(premul)
            for i in range(0,len(expected),4):
                a=expected[i+3]
                for c in range(3):expected[i+c]=min(255,(expected[i+c]*255+a//2)//a) if a else 0
            rawOriginal=decode(original);rawPrepared=decode(prepared);upload=decode(prepared,True)
            opaque=[i for i in range(0,len(premul),4) if premul[i+3]==255]
            originalChunks=chunks(original);preparedChunks=chunks(prepared)
            row=dict(image=name,size=[meta['width'],meta['height']],originalSHA256=hashlib.sha256(original).hexdigest(),
                     preparedSHA256=hashlib.sha256(prepared).hexdigest(),originalChunks=list(originalChunks),preparedChunks=list(preparedChunks),
                     preparedEqualsPreparationFormula=bytes(expected)==straight,
                     stbDirectCgbiVersusPrepared=compare(rawOriginal,rawPrepared),
                     stbPreparedVersusPython=compare(rawPrepared,straight),
                     alphaChanged=sum(premul[i+3]!=straight[i+3] for i in range(0,len(premul),4)),
                     opaquePixels=len(opaque),opaqueRGBChanged=sum(any(premul[i+c]!=straight[i+c] for c in range(3)) for i in opaque),
                     partialAlphaPixels=sum(0<premul[i+3]<255 for i in range(0,len(premul),4)),
                     reconstructedPremultipliedRGB=compare(premul,upload,3))
            if 'iCCP' in originalChunks:
                profile=originalChunks['iCCP'];sep=profile.index(0);wrapper='zlib'
                try:icc=zlib.decompress(profile[sep+2:])
                except zlib.error:
                    if not meta['cgbi']:raise
                    icc=zlib.decompress(profile[sep+2:],-15);wrapper='raw-deflate'
                row['icc']=dict(name=profile[:sep].decode('latin1'),compressionObserved=wrapper,bytes=len(icc),sha256=hashlib.sha256(icc).hexdigest(),colorSpace=icc[16:20].decode('ascii'))
            # Midpoints between horizontal texels, black destination; mathematical
            # comparison of straight filtering vs premultiplied filtering.
            maximum=0.;count=0;w=meta['width'];h=meta['height']
            for y in range(h):
                for x in range(w-1):
                    i=(y*w+x)*4;j=i+4;a=(straight[i+3]+straight[j+3])/510
                    error=max(abs((straight[i+c]+straight[j+c])*.5*a-(premul[i+c]+premul[j+c])*.5) for c in range(3))
                    maximum=max(maximum,error);count+=error>1
            row['cpuHorizontalMidpointModel']=dict(background='black',maxRGBDifference=round(maximum,6),samplesOver1=count)
            rows.append(row)
        result['originalSamples']=rows
        heli=[]
        for p in sorted((ROOT/'Resources/Images').glob('A*.png')):
            if not p.stem.startswith(('ALight','AHeavy','ADuct','ATransport')):continue
            blob=p.read_bytes();meta,pix=png_pixels(blob);c=chunks(blob)
            heli.append(dict(image=p.name,size=[meta['width'],meta['height']],chunks=list(c),
                             totalPixels=len(pix)//4,partialAlphaPixels=sum(0<a<255 for a in pix[3::4]),
                             stbEqualsPython=decode(blob)==pix))
        result['helicopters']=heli
    return result

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--source',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    r=audit(a.source);a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(r,indent=2)+'\n')
    print(json.dumps({'originalMetadata':r['originalMetadata'],'samples':len(r['originalSamples']),'helicopters':len(r['helicopters']),
                      'allDirectDecodesEqual':all(x['stbDirectCgbiVersusPrepared']['differentChannels']==0 for x in r['originalSamples'])}))
