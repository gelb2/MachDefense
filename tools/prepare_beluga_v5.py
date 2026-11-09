#!/usr/bin/env python3
"""Prepare the adopted Beluga v5 sheet as separate, registered game parts.

Only Art/Aircraft/beluga-v5-20261109 and .local/beluga-v5-integration are
written. Resource registration, MCB generation and runtime changes belong to
the integration owner. Neutral and roll source images are retained verbatim.
"""
from pathlib import Path
import hashlib, json, math, subprocess, tempfile
from prepare_resources import png_pixels, png_encode

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'Art/Aircraft/beluga-v5-20261109'
EVIDENCE = ROOT / '.local/beluga-v5-integration'
CW, CH = 140, 147
PAD = (CH-131)/2
ANCHOR = (CW/2+10, CH/2-1)
ANGLES = [0, -8, -16, -24, 8, 16, 24]
LEVEL_ROWS = [0, 299, 599, 892, 1185, 1536]
LEVEL_HUBS = [(265,139),(758,139),(265,439),(758,439),(265,735),
              (758,735),(265,1029),(758,1029),(265,1322),(758,1322)]
# These hubs and masks were measured on the NEW source images, not copied
# from the old Beluga's SEAMS. New drawings expose different side faces.
BANK_HUBS = [(298,218),(861,218),(1429,218),(260,682),(813,683),(1385,682)]
PARTS = ['ATransportBodyV5','ATransportWingLV5','ATransportWingRV5','ATransportEngineV5']
GUN_TOP = [55,50,48,47,59,61,61]
GUN_BOTTOM = [83,81,80,77,88,90,91]
WEAPON_SOCKETS = [[30,14],[30,-14],[-1,47],[-1,-47]]

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):
    m, rgba = png_pixels(p.read_bytes())
    return m['width'], m['height'], rgba
def bounds(data, w, h, box=None, threshold=16):
    x0,y0,x1,y1 = box or (0,0,w,h)
    xs=[];ys=[]
    for y in range(y0,y1):
        row=[x for x in range(x0,x1) if data[(y*w+x)*4+3]>threshold]
        if row: xs.extend((row[0],row[-1]));ys.append(y)
    if not xs:return None
    return [min(xs),min(ys),max(xs)+1,max(ys)+1]
def sample(src,w,h,x,y):
    # Bilinear sampling in premultiplied space avoids dark transparent fringes.
    # Source-to-canonical registration and reduction happen in this one pass.
    x0,y0=math.floor(x),math.floor(y);fx,fy=x-x0,y-y0
    a=0.;rgb=[0.,0.,0.]
    for dx,dy,weight in [(0,0,(1-fx)*(1-fy)),(1,0,fx*(1-fy)),
                         (0,1,(1-fx)*fy),(1,1,fx*fy)]:
        xx,yy=x0+dx,y0+dy
        if not (0<=xx<w and 0<=yy<h):continue
        i=(yy*w+xx)*4;alpha=src[i+3]*weight;a+=alpha
        for c in range(3):rgb[c]+=src[i+c]*alpha
    if a<.5:return (0,0,0,0)
    return tuple(round(v/a) for v in rgb)+(min(255,round(a)),)
def register(src,w,h,hub,scale,source_box,mask=None,roll=0.):
    """Map a north-facing source to the +X-facing canonical body frame."""
    out=bytearray(CW*CH*4);co=math.cos(roll);si=math.sin(roll)*.35
    for y in range(CH):
        for x in range(CW):
            # Invert optional local wing-panel roll projection, then the
            # quarter-turn + uniform scale. This is not used for body poses:
            # their visible side faces are drawn in the authored bank source.
            across=(y-ANCHOR[1])/co
            longitudinal=(x-ANCHOR[0])-si*across
            sx=hub[0]+across/scale;sy=hub[1]-longitudinal/scale
            if not (source_box[0]<=sx<source_box[2] and source_box[1]<=sy<source_box[3]):continue
            if mask and not mask(sx-hub[0],sy-hub[1]):continue
            rgba=sample(src,w,h,sx,sy)
            if rgba[3]<2:continue # discard only 0/1-alpha generation halo
            i=(y*CW+x)*4;out[i:i+4]=bytes(rgba)
    return out
def compose(layers):
    out=bytearray(CW*CH*4)
    for layer in layers:
        for i in range(0,len(out),4):
            sa=layer[i+3]/255
            if not sa:continue
            da=out[i+3]/255;oa=sa+da*(1-sa)
            for c in range(3):out[i+c]=round((layer[i+c]*sa+out[i+c]*da*(1-sa))/oa)
            out[i+3]=round(oa*255)
    return out
def split_pose(frame, angle):
    """Disjoint ownership of the NEW registered source, no repainted pixels."""
    layers=[bytearray(len(frame)) for _ in PARTS]
    # Distinct per-roll registered seams, in the new 140x131 body canvas.
    tilt=math.sin(math.radians(angle));center=ANCHOR[1]
    for y in range(CH):
        for x in range(CW):
            idx=0;wing_x=x-tilt*(y-center)*.35
            if 48<=wing_x<69 and 50+PAD<=y<80+PAD:idx=3
            elif 61<=wing_x<102 and y<55+PAD:idx=1
            elif 61<=wing_x<102 and y>=74+PAD:idx=2
            i=(y*CW+x)*4;layers[idx][i:i+4]=frame[i:i+4]
    assert bytes(sum(z[i] for z in layers) for i in range(len(frame)))==bytes(frame)
    return layers
def take_box(src,box):
    x0,y0,x1,y1=box;w,h=x1-x0,y1-y0
    return b''.join(src[((y+y0)*CW+x0)*4:((y+y0)*CW+x1)*4] for y in range(h))
def weapon_boxes(tile,angle):
    co=math.cos(math.radians(angle));si=math.sin(math.radians(angle))*.35
    # Include the complete barrel edge; a narrower mask leaves a second
    # non-firing barrel silhouette behind the separately registered weapon.
    out=[(87,GUN_TOP[tile],113,GUN_TOP[tile]+9),
         (87,GUN_BOTTOM[tile],113,GUN_BOTTOM[tile]+9)]
    for sy in [47,-47]:
        # Match the existing weapon/socket bank transform in local world space.
        x=CW/2-1+si*sy;y=CH/2-co*sy
        out.append((round(x-5),round(y-4.5),round(x+5),round(y+4.5)))
    return out
def remove_weapons(frame,tile,angle):
    cleaned=bytearray(frame);taken=bytearray(len(frame))
    for x0,y0,x1,y1 in weapon_boxes(tile,angle):
        for y in range(max(0,y0),min(CH,y1)):
            for x in range(max(0,x0),min(CW,x1)):
                i=(y*CW+x)*4;taken[i:i+4]=cleaned[i:i+4];cleaned[i:i+4]=b'\0'*4
    assert bytes(a+b for a,b in zip(cleaned,taken))==bytes(frame)
    return cleaned,taken
def pack(name,frames,force_full=False):
    if force_full:box=[0,0,CW,CH]
    else:
        bs=[bounds(f,CW,CH,threshold=1) for f in frames];bs=[b for b in bs if b]
        assert bs,name
        x0=min(b[0] for b in bs);y0=min(b[1] for b in bs)
        x1=max(b[2] for b in bs);y1=max(b[3] for b in bs)
        # Preserve integer socket offsets with an even-width/odd-height cell.
        if (x1-x0)%2:x1=min(CW,x1+1);x0=x1-((x1-x0+1)//2)*2
        if (y1-y0)%2==0:y1=min(CH,y1+1)
        if (y1-y0)%2==0:y0=max(0,y0-1)
        box=[x0,y0,x1,y1]
    x0,y0,x1,y1=box;w,h=x1-x0,y1-y0;n=len(frames)
    data=bytearray(w*n*h*4)
    for f,src in enumerate(frames):
        for y in range(h):
            start=((y+y0)*CW+x0)*4;dest=(y*w*n+f*w)*4
            data[dest:dest+w*4]=src[start:start+w*4]
    rgba=ART/'rgba'/f'{name}.png';rgba.parent.mkdir(parents=True,exist_ok=True)
    rgba.write_bytes(png_encode(w*n,h,bytes(data)))
    center=[x0+w/2-CW/2,CH/2-(y0+h/2)]
    rec={'name':name,'preparedImage':str(rgba.relative_to(ROOT)),
         'sha256':sha(rgba),'cellSize':[w,h],'tileCount':n,'crop':box,
         'attachmentOffset':center,'sourcePoseHashes':[hashlib.sha256(f).hexdigest() for f in frames]}
    return rec
def cgbi(records):
    crush=subprocess.check_output(['xcrun','--find','pngcrush'],text=True).strip()
    out=ART/'cgbi';out.mkdir(exist_ok=True)
    for r in records:
        src=ROOT/r['preparedImage'];dest=out/src.name
        subprocess.run([crush,'-iphone','-f','0',str(src),str(dest)],check=True,
                       stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
        m,a=png_pixels(src.read_bytes());n,b=png_pixels(dest.read_bytes())
        assert n['cgbi'] and (m['width'],m['height'])==(n['width'],n['height'])
        assert a[3::4]==b[3::4]
        assert all(a[i+c]==b[i+c] for i in range(0,len(a),4) if a[i+3]==255 for c in range(3))
        r.update(cgbiImage=str(dest.relative_to(ROOT)),cgbiSHA256=sha(dest),alphaPreserved=True)

def run():
    EVIDENCE.mkdir(parents=True,exist_ok=True)
    source=ART/'source/rotor-removed-generated.png';sw,sh,sp=read(source)
    approved=ART/'source/approved-v5.png'
    bank=ART/'source/bank-registered-generated.png';bw,bh,bp=read(bank)
    old=ROOT/'Art/Aircraft/banking-correction-20261109/prepared/ATransportBody.png'
    ow,oh,op=read(old);oldbox=bounds(op,ow,oh,(0,0,140,131));target_span=oldbox[3]-oldbox[1]
    neutral_box=bounds(sp,sw,sh,(0,0,512,299));scale=target_span/(neutral_box[2]-neutral_box[0])
    full=[register(sp,sw,sh,LEVEL_HUBS[0],scale,(0,0,512,299))]
    pose_records=[{'tile':0,'angle':0,'source':str(source.relative_to(ROOT)),
                   'sourceBox':neutral_box,'sourceHub':LEVEL_HUBS[0],'uniformScale':scale}]
    for k,angle in enumerate(ANGLES[1:]):
        col,row=k%3,k//3;box=(round(col*bw/3),round(row*bh/2),round((col+1)*bw/3),round((row+1)*bh/2))
        bb=bounds(bp,bw,bh,box)
        # One uniform scale per authored frame, calibrated by lateral span.
        # Record the resulting longitudinal extent rather than stretching it.
        factor=target_span*math.cos(math.radians(angle))/(bb[2]-bb[0])
        frame=register(bp,bw,bh,BANK_HUBS[k],factor,box)
        full.append(frame)
        pose_records.append({'tile':k+1,'angle':angle,'source':str(bank.relative_to(ROOT)),
                             'sourceBox':bb,'sourceHub':BANK_HUBS[k],'uniformScale':factor})
    weapon_layers=[];clean_full=[]
    for tile,(f,angle) in enumerate(zip(full,ANGLES)):
        clean,taken=remove_weapons(f,tile,angle);clean_full.append(clean);weapon_layers.append(taken)
    split=[split_pose(f,a) for f,a in zip(clean_full,ANGLES)]
    records=[]
    for i,name in enumerate(PARTS):records.append(pack(name,[s[i] for s in split],i==0))
    weapon_specs=[]
    for name,box,muzzle in [('ATransportGunV5',(87,55,113,64),[11,0]),
                             ('ATransportMissileV5',(64,22,74,31),[4,0])]:
        pixels=take_box(full[0],box);w,h=box[2]-box[0],box[3]-box[1]
        out=ART/'rgba'/f'{name}.png';out.write_bytes(png_encode(w,h,pixels))
        r={'name':name,'preparedImage':str(out.relative_to(ROOT)),'sha256':sha(out),
           'cellSize':[w,h],'tileCount':1,'attachmentOffset':[0,0],
           'neutralSourceCrop':list(box),'muzzles':[[muzzle[0],1],[muzzle[0],-1]],
           'sockets':[[0,0],[muzzle[0],1],[muzzle[0],-1]],
           'runtimeBank':'existing aircraftBankPoint projection; static neutral weapon'}
        records.append(r);weapon_specs.append(r)
    # Upgrade armor is copied from each approved-derived level's wing-root
    # surface, then projected as an independent plate onto the drawn roll body.
    # Main fuselage/wing/engine bank silhouettes are NEVER a flat neutral warp.
    for lv in range(1,11):
        col,row=(lv-1)%2,(lv-1)//2;box=(col*512,LEVEL_ROWS[row],(col+1)*512,LEVEL_ROWS[row+1]);hub=LEVEL_HUBS[lv-1]
        for side in ['L','R']:
            def mask(dx,dy,side=side):
                return (-86<=dx<=-19 if side=='L' else 19<=dx<=86) and -40<=dy<=43
            fs=[register(sp,sw,sh,hub,scale,box,mask,math.radians(a)) for a in ANGLES]
            records.append(pack(f'ATransportRoot{side}V5_{lv:02}',fs))
    # Trace the visible four blades and rotating hub directly from the adopted
    # v5 sheet. These are separate pixels, not a rotor baked into the body.
    aw,ah,ap=read(approved);hub=(265.5,141.0)
    blade_ends=[(195,75),(336,75),(195,208),(336,208)]
    def rotor_mask(x,y):
        dx,dy=x-hub[0],y-hub[1]
        if dx*dx+dy*dy<=10.5**2:return True
        for ex,ey in blade_ends:
            vx,vy=ex-hub[0],ey-hub[1];t=(dx*vx+dy*vy)/(vx*vx+vy*vy)
            if 0<=t<=1 and abs(dx*vy-dy*vx)/math.hypot(vx,vy)<=3.4:return True
        return False
    # Coverage AA is computed only for the newly traced cutout boundary. It
    # does not quantize the source's colors or alter existing game alpha.
    x0,y0,x1,y1=188,66,344,216;rw,rh=x1-x0,y1-y0;cut=bytearray(rw*rh*4)
    for y in range(rh):
        for x in range(rw):
            hits=sum(rotor_mask(x+x0+dx,y+y0+dy) for dx in [.125,.375,.625,.875] for dy in [.125,.375,.625,.875])
            if hits:
                i=(y*rw+x)*4;j=((y+y0)*aw+x+x0)*4
                cut[i:i+3]=ap[j:j+3];cut[i+3]=round(hits*255/16)
    rotor_width=58;rotor_height=57;rotor_pixels=bytearray(rotor_width*rotor_height*4)
    for y in range(rotor_height):
        for x in range(rotor_width):
            # North reference rotates clockwise to match the game's +X art.
            sx=hub[0]+(y-(rotor_height-1)/2)/scale-x0
            sy=hub[1]-(x-(rotor_width-1)/2)/scale-y0
            i=(y*rotor_width+x)*4;rotor_pixels[i:i+4]=bytes(sample(cut,rw,rh,sx,sy))
    rotor_out=ART/'rgba/ATransportFanV5.png'
    rotor_out.write_bytes(png_encode(rotor_width,rotor_height,bytes(rotor_pixels)))
    rm,rp=png_pixels(rotor_out.read_bytes())
    records.append({'name':'ATransportFanV5','preparedImage':str(rotor_out.relative_to(ROOT)),
                    'sha256':sha(rotor_out),'cellSize':[rm['width'],rm['height']],
                    'tileCount':1,'attachmentOffset':[10,1],
                    'source':str(approved.relative_to(ROOT)),
                    'sourceSHA256':sha(approved),'uniformScale':scale,
                    'sourceHub':list(hub),'sourceBladeEnds':blade_ends,
                    'rotorBladeCount':4,'requiresV5RotorRegistration':False})
    cgbi(records)
    for i,(f,p) in enumerate(zip(full,pose_records)):
        path=EVIDENCE/f'base-pose-{i}.png';path.write_bytes(png_encode(CW,CH,bytes(f)))
        p['outputBoundsAlpha16']=bounds(f,CW,CH);p['rotorCanvasPixel']=list(ANCHOR)
        p['removedWeaponBoxes']=weapon_boxes(i,ANGLES[i])
        p['bodyWithoutWeaponSHA256']=hashlib.sha256(clean_full[i]).hexdigest()
    neutral_width=pose_records[0]['outputBoundsAlpha16'][3]-pose_records[0]['outputBoundsAlpha16'][1]
    manifest={'approvedSource':str(approved.relative_to(ROOT)),'approvedSHA256':sha(approved),
        'rotorRemovalSource':str(source.relative_to(ROOT)),'rotorRemovalSHA256':sha(source),
        'bankSource':str(bank.relative_to(ROOT)),'bankSHA256':sha(bank),
        'poseOrder':ANGLES,'canonicalCell':[CW,CH],'rotorSocket':[10,1],
        'originalNeutralAlphaBounds':oldbox,'originalNorthWidthPixels':target_span,
        'newNorthWidthPixels':neutral_width,'neutralUniformScale':scale,
        'separatePartRecompositionExact':True,'poseRecords':pose_records,'records':records,
        'weaponSource':{'gun':'v5 forward side barrel','missile':'v5 under-wing pod',
                        'rootSockets':WEAPON_SOCKETS,'sameNeutralArtworkForSymmetricInstances':True,
                        'bankBodyWeaponPixelsRemoved':True,'runtimeProjectionUnchanged':True},
        'pending':['Runtime UI, bank alignment, firing and original-unit preservation checks belong to integration validation.']}
    (ART/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
    definitions=[]
    for r in records:
        if r['name'] in ['ATransportGunV5','ATransportMissileV5']:continue
        rotor=r['name']=='ATransportFanV5';root=r['name']=='ATransportBodyV5'
        off=r['attachmentOffset']
        sockets=([[0,0],[10,1],[0,0],[0,0],[0,0],*WEAPON_SOCKETS] if root else
                 [[0,0]] if rotor else [[-off[0],-off[1]]])
        definitions.append({'name':r['name'],'type':7 if rotor else 0,'tileCount':r['tileCount'],
                            'sockets':sockets,'preparedImage':r['preparedImage'],
                            'texture':{'image':r['name'],'grid':[r['tileCount'],1],'firstTile':0}})
    def tree(lv):
        suffix='' if lv is None else f'_{lv:02}'
        level=1 if lv is None else lv
        fixed=[{'name':n,'parentSocket':0,'zOrder':z,'children':[]} for n,z in
                [('ATransportWingLV5',1),('ATransportWingRV5',1),('ATransportEngineV5',2),
                 (f'ATransportRootLV5_{level:02}',2),(f'ATransportRootRV5_{level:02}',2)]]
        weapons=[{'name':'ATransportGun'+suffix,'parentSocket':s,'zOrder':3,'children':[]} for s in [5,6]]
        weapons += [{'name':'ATransportMissile'+suffix,'parentSocket':s,'zOrder':3,'children':[]} for s in [7,8]]
        return [{'name':'ATransportBodyV5','parentSocket':0,'zOrder':1,
                 'children':fixed+weapons+[{'name':'ATransportFanV5','parentSocket':1,'zOrder':4,'children':[]}]}]
    proposal={'parts':definitions,'machine':{'name':'AircraftTransport','parts':tree(None)},
              'levelVariants':[{'name':f'AircraftTransport_{lv:02}','source':'AircraftTransport',
                                'weaponSuffix':f'_{lv:02}','parts':tree(lv)} for lv in range(1,11)],
              'weaponImages':{'ATransportGun':'ATransportGunV5','ATransportMissile':'ATransportMissileV5'},
              'weaponSockets':{base:r['sockets'] for base,r in zip(['ATransportGun','ATransportMissile'],weapon_specs)},
              'weaponPreparedImages':{r['name']:r['preparedImage'] for r in weapon_specs},
              'bankTileNames':[r['name'] for r in records if r['tileCount']==7],
              'requiredAssemblyScale':'.765 * (164 / 196)',
              'noWeaponInfoOrMovementChanges':True}
    (ART/'assembly-proposal.json').write_text(json.dumps(proposal,ensure_ascii=False,indent=2)+'\n')
    print(json.dumps({'parts':len(records),'width':neutral_width,'sourceWidth':target_span,'manifest':str(ART/'manifest.json')}))

if __name__=='__main__':run()
