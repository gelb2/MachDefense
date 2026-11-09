#!/usr/bin/env python3
"""Package drawn roll poses into seven-tile helicopter-only PNG sheets.

Tile zero copies the approved neutral pixels exactly. Six drawn poses share one
uniform scale per aircraft; there is no non-uniform stretch or runtime redraw.
"""
import hashlib,json,subprocess,tempfile
from pathlib import Path
from prepare_resources import png_pixels

def bounds(pixels,width,region,threshold=16):
 x0,y0,x1,y1=region;xs=[];ys=[]
 for y in range(y0,y1):
  row=pixels[(y*width+x0)*4+3:(y*width+x1)*4:4]
  visible=[x0+x for x,a in enumerate(row) if a>threshold]
  if visible:xs.extend((visible[0],visible[-1]));ys.append(y)
 assert xs
 return min(xs),min(ys),max(xs)+1,max(ys)+1

ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'Art/Aircraft/banking-20261109'
PAIRS=[('scout','ALightBody'),('ah10','AHeavyBody'),('cheyenne','ADuctBody'),('beluga','ATransportBody')]
def run():
 out=ART/'prepared';out.mkdir(exist_ok=True);rows=[]
 with tempfile.TemporaryDirectory(prefix='md-roll-') as directory:
  temp=Path(directory)
  for name,body in PAIRS:
   original=ROOT/'Art/Aircraft/runtime/png-source'/(body+'.png');nm,neutral=png_pixels(original.read_bytes())
   source=ART/'generated'/(name+'.png');sm,sheet=png_pixels(source.read_bytes());sw,sh=sm['width'],sm['height']
   boxes=[]
   for index in range(6):
    col,row=index%3,index//3;x0,x1=round(col*sw/3),round((col+1)*sw/3);y0,y1=round(row*sh/2),round((row+1)*sh/2)
    box=bounds(sheet,sw,(x0,y0,x1,y1))
    box=(max(x0,box[0]-2),max(y0,box[1]-2),min(x1,box[2]+2),min(y1,box[3]+2))
    boxes.append((box[0],box[1],box[2]-box[0],box[3]-box[1]))
   box=bounds(neutral,nm['width'],(0,0,nm['width'],nm['height']));factor=(box[2]-box[0])/max(b[2] for b in boxes)
   w,h=nm['width'],nm['height'];pad=0;cw,ch=w,h;cells=[];placements=[]
   for index in range(7):
    output=temp/f'{name}-{index}.png'
    if index==0:
     subprocess.run(['magick','-size',f'{cw}x{ch}','canvas:none',str(original),'-gravity','center','-compose','Copy','-composite','PNG32:'+str(output)],check=True)
    else:
     x,y,bw,bh=boxes[index-1];rw,rh=round(bw*factor),round(bh*factor)
     assert rw<=cw and rh<=ch,(name,index,rw,rh,cw,ch)
     subprocess.run(['magick',str(source),'-crop',f'{bw}x{bh}+{x}+{y}','+repage','-filter','Lanczos','-resize',f'{factor*100:.8f}%','-background','none','-gravity','center','-extent',f'{cw}x{ch}','PNG32:'+str(output)],check=True)
     placements.append({'frame':index,'rollDegrees':[-8,-16,-24,8,16,24][index-1],'sourceCrop':[x,y,bw,bh],'uniformScale':factor})
    cells.append(output)
   target=out/(body+'.png')
   subprocess.run(['magick',*[str(p) for p in cells],'+append','-strip','PNG32:'+str(target)],check=True)
   am,atlas=png_pixels(target.read_bytes());tile=b''.join(atlas[((y+pad)*am['width']+pad)*4:((y+pad)*am['width']+pad+w)*4] for y in range(h));assert tile==neutral
   rows.append({'body':body,'tileCount':7,'cellSize':[cw,ch],'neutralSize':[w,h],'padding':pad,'source':str(source.relative_to(ROOT)),'sourceSHA256':hashlib.sha256(source.read_bytes()).hexdigest(),'neutralSHA256':hashlib.sha256(original.read_bytes()).hexdigest(),'preparedSHA256':hashlib.sha256(target.read_bytes()).hexdigest(),'frames':placements,'neutralPixelsUnchanged':True})
 manifest={'method':'ImageGen drawn roll poses; uniform-scale packaging; original neutral pixels in tile 0','textures':rows}
 (ART/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps({'sheets':len(rows),'drawnPoses':24,'neutralFramesPreserved':4}))
if __name__=='__main__':run()
