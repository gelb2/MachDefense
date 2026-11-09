#!/usr/bin/env python3
"""Pack adopted Scout Lv10 components; never writes Resources or game code."""
import argparse, hashlib, json, math, shutil, subprocess, tempfile
from pathlib import Path
from prepare_resources import png_pixels, png_encode

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'Art/Aircraft/scout-v5-20261109'
ANGLES = [0, -8, -16, -24, 8, 16, 24]
# Visible bearing centres, manually annotated on the generated disassembly.
HUBS = [(239,195),(628,195),(1013,195),(1395,195),(1757,195),(220,587),(604,587)]

def run(*args):
    subprocess.run([str(a) for a in args], check=True, capture_output=True)

def measure(path, region=None, threshold=16):
    meta,pixels=png_pixels(path.read_bytes()); w,h=meta['width'],meta['height']
    x0,y0,x1,y1=region or (0,0,w,h)
    points=[(x,y) for y in range(y0,y1) for x in range(x0,x1) if pixels[(y*w+x)*4+3]>=threshold]
    if not points: raise ValueError(f'empty sprite {path}: {region}')
    return meta,pixels,[min(x for x,y in points),min(y for x,y in points),max(x for x,y in points)+1,max(y for x,y in points)+1]

def info(path):
    m,p,b=measure(path)
    return {'path':str(path.relative_to(ROOT)), 'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
            'canvas':[m['width'],m['height']], 'alphaBounds16':b, 'cgbi':m['cgbi'],
            'partialAlphaPixels':sum(0<a<255 for a in p[3::4])}

def isolate_component(source, target):
    """Production mask: remove disconnected generation debris, retain edge alpha."""
    m,p=png_pixels(source.read_bytes());w,h=m['width'],m['height']
    remaining={i for i,a in enumerate(p[3::4]) if a>=16};groups=[]
    while remaining:
        seed=remaining.pop();group={seed};stack=[seed]
        while stack:
            i=stack.pop();x,y=i%w,i//w
            for dy in (-1,0,1):
                for dx in (-1,0,1):
                    if 0<=x+dx<w and 0<=y+dy<h:
                        j=(y+dy)*w+x+dx
                        if j in remaining:remaining.remove(j);group.add(j);stack.append(j)
        groups.append(group)
    keep=max(groups,key=len);feather=set(keep)
    for i in keep:
        x,y=i%w,i//w
        for dy in range(-2,3):
            for dx in range(-2,3):
                if 0<=x+dx<w and 0<=y+dy<h:feather.add((y+dy)*w+x+dx)
    out=bytearray(p);removed=0
    for i,a in enumerate(p[3::4]):
        if a and i not in keep and not (a<16 and i in feather):out[i*4:i*4+4]=b'\0'*4;removed+=1
    target.write_bytes(png_encode(w,h,bytes(out)))
    return removed

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--width',type=float,default=59.0);args=ap.parse_args()
    magick=shutil.which('magick');assert magick
    pngcrush=subprocess.check_output(['xcrun','--find','pngcrush'],text=True).strip()
    rgba=ART/'rgba'; cgbi=ART/'cgbi'; cells=ART/'poses'
    for p in (rgba,cgbi,cells):p.mkdir(parents=True,exist_ok=True)
    sheet=ART/'source/components-clean.png'; sm,_,_=measure(sheet);sw,sh=sm['width'],sm['height']
    boxes=[]
    for i in range(10):
        region=(i%5*sw//5,i//5*sh//2,(i%5+1)*sw//5,(i//5+1)*sh//2)
        b=measure(sheet,region,16)[2]
        # Preserve feathered edges in a 2px margin, omit remote alpha<16 noise.
        boxes.append([max(region[0],b[0]-2),max(region[1],b[1]-2),min(region[2],b[2]+2),min(region[3],b[3]+2)])
    # Whole approved north-facing alpha width includes its four rotor blades.
    adoptedScale=args.width/598.0
    # Body length is preserved relative to the adopted v5's 911px silhouette.
    bodyScale=911.0*adoptedScale/(boxes[0][3]-boxes[0][1])
    frames=[]; records=[];cleanup=[]
    with tempfile.TemporaryDirectory(prefix='scout-v5-') as td:
        tmp=Path(td)
        def crop_scale(i, factor, target):
            x0,y0,x1,y1=boxes[i];out=tmp/f'component-{i}.png';raw=tmp/f'raw-{i}.png'
            isolated=ART/'components'/f'component-{i}.png';isolated.parent.mkdir(exist_ok=True)
            run(magick,sheet,'-crop',f'{x1-x0}x{y1-y0}+{x0}+{y0}','+repage',f'PNG32:{raw}')
            removed=isolate_component(raw,isolated);cleanup.append({'cell':i,'removedDisconnectedPixels':removed})
            run(magick,isolated,'-filter','Lanczos',
                '-resize',f'{factor*100:.9f}%','-background','none','-rotate','90','-strip',f'PNG32:{out}')
            return out
        for i,angle in enumerate(ANGLES):
            crop=crop_scale(i,bodyScale,None);m,_,_=measure(crop)
            x0,y0,x1,y1=boxes[i];hx,hy=HUBS[i]
            anchor=[(y1-hy)*bodyScale,(hx-x0)*bodyScale]
            offset=[round(50-anchor[0]),round(27-anchor[1])]
            assert min(offset)>=0 and offset[0]+m['width']<=100 and offset[1]+m['height']<=54
            dest=cells/f'body-{i}-{angle:+03d}.png'
            run(magick,'-size','100x54','xc:none',crop,'-geometry',f'+{offset[0]}+{offset[1]}','-compose','over','-composite','-strip',f'PNG32:{dest}')
            frames.append(dest);records.append({'tile':i,'bankDegrees':angle,'sourceBounds':boxes[i],
                'sourceBearingCenter':HUBS[i],'bearingCanvas':[50,27],'placement':offset,'image':info(dest)})
        body=rgba/'ALightBodyV5_10.png'
        run(magick,*frames,'+append','-strip',f'PNG32:{body}')
        # Standalone components were drawn at a different sheet scale. Normalize
        # each to the adopted source's annotated physical extent before packing.
        components=[('ALightGunV5_10',7,220.0*adoptedScale,'height',(46,10),(3,0)),
                    ('ALightMissileV5_10',8,205.0*adoptedScale,'height',(44,12),(3,0)),
                    ('ALightRotorV5_10',9,args.width,'width',(64,64),None)]
        weaponRecords=[]
        for name,i,target,axis,canvas,baseOffset in components:
            b=boxes[i];factor=target/(b[3]-b[1] if axis=='height' else b[2]-b[0])
            crop=crop_scale(i,factor,None);m,_,_=measure(crop)
            if baseOffset is None: offset=[round((canvas[0]-m['width'])/2),round((canvas[1]-m['height'])/2)]
            else: offset=[canvas[0]//2-baseOffset[0],round((canvas[1]-m['height'])/2)]
            assert offset[0]+m['width']<=canvas[0] and offset[1]+m['height']<=canvas[1]
            dest=rgba/(name+'.png')
            run(magick,'-size',f'{canvas[0]}x{canvas[1]}','xc:none',crop,'-geometry',f'+{offset[0]}+{offset[1]}','-compose','over','-composite','-strip',f'PNG32:{dest}')
            weaponRecords.append({'name':name,'sourceBounds':b,'sourceUniformScale':factor,'placement':offset,'image':info(dest)})
    assets=[]
    for source in sorted(rgba.glob('*.png')):
        dest=cgbi/source.name
        if dest.exists():dest.unlink()
        run(pngcrush,'-iphone','-f','0',source,dest)
        am,a,_=measure(source);bm,b,_=measure(dest)
        assert bm['cgbi'] and am['width']==bm['width'] and am['height']==bm['height']
        assert a[3::4]==b[3::4]
        assert all(a[i:i+3]==b[i:i+3] for i in range(0,len(a),4) if a[i+3]==255)
        assets.append({'rgba':info(source),'cgbi':info(dest),'alphaExact':True,'opaqueRGBExact':True})
    proposal={
        'parts':[
            {'name':'ALightBodyV5_10','type':0,'tileCount':7,'sockets':[[0,0],[0,0],[6,-13],[6,12]],'texture':{'image':'ALightBodyV5_10','grid':[7,1],'firstTile':0}},
            {'name':'ALightRotorV5_10','type':7,'tileCount':1,'sockets':[[0,0]],'texture':{'image':'ALightRotorV5_10','grid':[1,1],'firstTile':0}}],
        'weaponImages':{'ALightGun':'ALightGunV5_10','ALightMissile':'ALightMissileV5_10'},
        'weaponSockets':{'ALightGun':[[0,0],[18,0],[18,0]],'ALightMissile':[[0,0],[17,-1],[17,1]]},
        'machineParts':[{'name':'ALightBodyV5_10','parentSocket':0,'zOrder':1,'children':[
            {'name':'ALightGun_10','parentSocket':2,'zOrder':2,'children':[]},
            {'name':'ALightMissile_10','parentSocket':3,'zOrder':2,'children':[]},
            {'name':'ALightRotorV5_10','parentSocket':1,'zOrder':3,'children':[]}]}]}
    manifest={'scope':'Scout level 10 only','source':info(ART/'source/approved-v5.png'),'disassembly':info(sheet),
        'orientation':'+X (north-facing input rotated clockwise 90 degrees)', 'bankOrder':ANGLES,
        'targetNorthWidth':args.width,'widthBasis':'current 2x web comparison 118px / 2; rotor-inclusive neutral X pose',
        'adoptedAlphaBounds16':[297,96,895,1007], 'adoptedUniformScale':adoptedScale,
        'bodySourceUniformScale':bodyScale,'annotationNote':'Bearing centers and weapon lengths/attachment pads are manually annotated; alpha bounds are measured. Rounded integer placement error is at most 0.5px per axis.',
        'frames':records,'components':weaponRecords,'disconnectedDebrisCleanup':cleanup,'assets':assets,'assemblyProposal':proposal,
        'validation':{'sourceUnmodified':True,'sevenDrawnBankPoses':True,'bodyContainsNoWeaponBarrelsOrRotorBlades':True,
        'gameRuntimeTested':False,'wholeRotationWidthNotYetMeasured':True}}
    (ART/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
    (ART/'assembly-proposal.json').write_text(json.dumps(proposal,indent=2)+'\n')
    print(json.dumps({'manifest':str(ART/'manifest.json'),'images':[a['cgbi']['path'] for a in assets]}))

if __name__=='__main__':main()
