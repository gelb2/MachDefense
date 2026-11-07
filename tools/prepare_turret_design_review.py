#!/usr/bin/env python3
"""Package generated review art; never writes runtime textures.

Only crop, uniformly resize, and lay out comparisons. No recoloring or drawing
of game art. Full generated sources remain alongside these review exports.
"""
from pathlib import Path
import hashlib, json, shutil
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'Art/Level10/20261107/design-v5'
OUT = ART / 'review'
OUT.mkdir(exist_ok=True)
ROWS = [('body','b'),('rifle','gun')]
layout = json.loads((ART.parent/'body-composition.json').read_text())
font = ImageFont.truetype('/System/Library/Fonts/Menlo.ttc', 14)
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def bounds(im): return im.getchannel('A').point(lambda a: 255 if a>32 else 0).getbbox()
def tone(im):
    values=[.2126*r+.7152*g+.0722*b for r,g,b,a in im.get_flattened_data() if a>=224]
    return {'mean_luma':round(sum(values)/len(values),2),'bright_pct':round(sum(v>=160 for v in values)*100/len(values),2)}
records=[]
for family,prefix in ROWS:
    original=ROOT/f'Resources/Images/Turret_{prefix}_05.png'
    ref=Image.open(original).convert('RGBA');ob=bounds(ref)
    ow,oh=ob[2]-ob[0],ob[3]-ob[1]
    sheet=Image.open(ART/f'{family}.png').convert('RGBA')
    for level in [1,2,3,4,5]:
        p=ROOT/f'Resources/Images/Turret_{prefix}_{level:02}.png'
        shutil.copy2(p,OUT/f'{family}-{level}.png')
    # Find empty gutters near nominal fifths; sheets need not have equal margins.
    alpha=sheet.getchannel('A')
    cuts=[0]
    for j in range(1,5):
        mid=round(sheet.width*j/5);radius=round(sheet.width/5*.18)
        counts=[(sum(v>32 for v in alpha.crop((x,0,x+1,sheet.height)).get_flattened_data()),x) for x in range(mid-radius,mid+radius+1)]
        minimum=min(n for n,x in counts)
        candidates=[x for n,x in counts if n==minimum]
        runs=[]
        for x in candidates:
            if not runs or x>runs[-1][-1]+1: runs.append([])
            runs[-1].append(x)
        run=min(runs,key=lambda r:abs((r[0]+r[-1])/2-mid))
        cuts.append(round((run[0]+run[-1])/2))
    cuts.append(sheet.width)
    for i in range(5):
        left,right=cuts[i:i+2]
        cell=sheet.crop((left,0,right,sheet.height));b=bounds(cell);crop=cell.crop(b)
        scale=oh/crop.height if family=='body' else ow/crop.width
        size=(round(crop.width*scale),round(crop.height*scale))
        # Preserve source aspect. Never squeeze one axis or shrink all levels.
        small=crop.resize(size,Image.Resampling.LANCZOS)
        x=round((ob[0]+ob[2]-size[0])/2) if family=='body' else ob[2]-size[0]
        y=round((ob[1]+ob[3]-size[1])/2)
        assert x>=0 and y>=0 and x+size[0]<=ref.width and y+size[1]<=ref.height
        canvas=Image.new('RGBA',ref.size);canvas.alpha_composite(small,(x,y))
        dest=OUT/f'{family}-{i+6}.png';canvas.save(dest,optimize=True)
        records.append({'family':family,'level':i+6,'source':f'{family}.png','source_sha256':sha(ART/f'{family}.png'),
            'original':str(original.relative_to(ROOT)),'original_sha256':sha(original),'source_rect':[left+b[0],b[1],crop.width,crop.height],
            'canvas':list(ref.size),'visible_size':list(size),'original_visible_size':[ow,oh],
            'uniform_scale':scale,'offset':[x,y],'height_delta':size[1]-oh,'width_delta':size[0]-ow,
            'tone':tone(canvas),'original_tone':tone(ref),'review_sha256':sha(dest)})
    # Comparative presentation, not a replacement texture.
    board=Image.new('RGB',(8*300,380),(60,65,66));d=ImageDraw.Draw(board)
    for col,level in enumerate([3,4,5,6,7,8,9,10]):
        im=Image.open(OUT/f'{family}-{level}.png').convert('RGBA')
        d.text((col*300+10,10),f'{family} {"ORIGINAL" if level<6 else "DRAFT"} L{level}',font=font,fill='white')
        # Show the full canvas at native size, then 2x nearest-neighbor.
        board.paste(im,(col*300+(300-im.width)//2,38),im)
        zoom=im.resize((im.width*2,im.height*2),Image.Resampling.NEAREST)
        board.paste(zoom,(col*300+(300-zoom.width)//2,370-zoom.height),zoom)
    board.save(OUT/f'{family}-comparison.webp',lossless=True)

# Assembly uses original canvas anchors. This is a visual draft, not a runtime
# pivot/muzzle acceptance test; game Resources remain untouched.
for f in layout['families'][:1]:
    family=next(name for name,prefix in ROWS if prefix==f['id'])
    p=f['original'];anchor=(p['png_size'][0]//2+int(p['sockets'][0][0]),p['png_size'][1]//2-int(p['sockets'][0][1]))
    board=Image.new('RGB',(6*300,370),(60,65,66));d=ImageDraw.Draw(board)
    for col,level in enumerate(range(5,11)):
        c=Image.new('RGBA',(150,150));b=Image.open(OUT/f'body-{level}.png').convert('RGBA');w=Image.open(OUT/f'{family}-{level}.png').convert('RGBA')
        c.alpha_composite(b,(75-38,75-31));c.alpha_composite(w,(75-anchor[0],75-anchor[1]))
        d.text((col*300+10,10),f'{family} {"ORIGINAL" if level==5 else "DRAFT"} L{level}',font=font,fill='white')
        zoom=c.resize((300,300),Image.Resampling.NEAREST);board.paste(zoom,(col*300,50),zoom)
    board.save(OUT/f'{family}-assembled.webp',lossless=True)

manifest={'status':'review-only-not-runtime','filter':'uniform Lanczos reduction; comparison enlargement nearest neighbor',
          'parts':records,'original_references':[{'file':f'Resources/Images/Turret_{p}_{l:02}.png','sha256':sha(ROOT/f'Resources/Images/Turret_{p}_{l:02}.png')} for _,p in ROWS for l in [1,2,3,4,5]]}
(OUT/'measurements.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('10 review sprites, 2 part comparisons, 1 assembly comparison; runtime unchanged')
for family,_ in ROWS:
    r=[x for x in records if x['family']==family]
    print(family,r[0]['original_visible_size'],[x['visible_size'] for x in r], 'luma',r[0]['original_tone']['mean_luma'],round(sum(x['tone']['mean_luma'] for x in r)/5,1))

# Individual native-size assemblies and lossless generated-sheet previews.
anchors=json.loads((ART/'original-anchors.json').read_text())
for level in range(1,11):
    c=Image.new('RGBA',(96,96))
    b=Image.open(OUT/f'body-{level}.png').convert('RGBA')
    w=Image.open(OUT/f'rifle-{level}.png').convert('RGBA')
    ba=anchors[str(min(level,5))]['body'];wa=anchors[str(min(level,5))]['rifle']
    c.alpha_composite(b,(48-ba[0],48-ba[1]))
    c.alpha_composite(w,(48-wa[0],48-wa[1]))
    c.save(OUT/f'assembled-{level}.png')
for family,_ in ROWS:
    Image.open(ART/f'{family}.png').save(OUT/f'{family}-sheet.webp',lossless=True,exact=True)
