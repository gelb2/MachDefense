#!/usr/bin/env python3
"""Prepare new-aircraft QChunk resources; preserve every pre-existing non-aircraft node.

The existing loader reads the Level10 extension containers. Append aircraft nodes
there at build preparation time, without editing shared/original loader sources.
Standalone aircraft MCB files retain the exact authored aircraft definitions.
"""
import copy,hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PREFIXES=('ALight','AHeavy','ADuct','ATransport','Aircraft')
def encode(kind,data=b'',children=()):
 children=list(children);payload=data+struct.pack('<I',len(children))+b''.join(children)
 return struct.pack('<III',12+len(payload),kind,len(data))+payload
def named(name):
 b=name.encode('ascii');assert len(b)<24;return b.ljust(24,b'\0')
def decode(blob,pos=0):
 start=pos;size,kind,n=struct.unpack_from('<III',blob,pos);pos+=12;data=blob[pos:pos+n];pos+=n
 count,=struct.unpack_from('<I',blob,pos);pos+=4;children=[]
 for _ in range(count):
  child,pos=decode(blob,pos);children.append(child)
 return {'kind':kind,'data':data,'children':children,'encoded':blob[start:pos]},pos
def node_name(n):return n['data'][:24].split(b'\0')[0].decode('ascii') if n['data'] else ''
def run():
 src=ROOT/'Art/Aircraft/runtime/assembly.json';spec=json.loads(src.read_text());parts=[]
 baseparts={p['name']:p for p in spec['parts']}
 basemachines={m['name']:m for m in spec['machines']}
 authored_parts=list(spec['parts']);authored_machines=list(spec['machines']);aliases={};alias_images={}
 for variant in spec['levelVariants']:
  machine=copy.deepcopy(basemachines[variant['source']]);machine['name']=variant['name']
  def visit(nodes):
   for node in nodes:
    source=baseparts[node['name']]
    if source['type']==4:
     alias=source['name']+variant['weaponSuffix'];node['name']=alias
     if alias not in aliases:
      p=copy.deepcopy(source);p['name']=alias;p.pop('imageSource',None)
      authored_parts.append(p);aliases[alias]=source
      alias_images[alias]=variant.get('weaponImages',{}).get(source['name'],source['name'])
    visit(node['children'])
  visit(machine['parts'])
  if 'parts' in variant:
   machine['parts']=copy.deepcopy(variant['parts'])
   def validate(nodes):
    for node in nodes:
     assert node['name'] in baseparts or node['name'] in aliases,node['name']
     validate(node['children'])
   validate(machine['parts'])
  authored_machines.append(machine)
 # Existing table aliases select the original image without copying pixels.
 # Keep all pre-existing rows byte-identical, including original unit aliases.
 marker=b'// Aircraft persisted level aliases 2026-11-09'
 table_rows={'PartsList':[],'TileList':[]};tile_names=set()
 # Explicit texture selections may share a wider atlas while each part keeps
 # its authored tileCount (number of bank poses) and independent assembly node.
 selections={p['name']:(p,p['name']) for p in spec['parts'] if p.get('texture')}
 selections.update({alias:(source,alias_images[alias]) for alias,source in aliases.items()})
 for alias,(source,image_name) in selections.items():
  if not source['tileCount']:continue
  texture=source.get('texture',{}) if image_name==source['name'] else {}
  image_name=texture.get('image',image_name)
  first=texture.get('firstTile',0);grid=texture.get('grid',[source['tileCount'],1])
  assert 0<=first and first+source['tileCount']<=grid[0]*grid[1]
  table_rows['PartsList'].append(f"{alias}\tWPON\t{first}\t{image_name}\n".encode())
  image=image_name+'.png'
  if image not in tile_names:
   table_rows['TileList'].append(f"{image}\t{grid[0]}\t{grid[1]}\n".encode());tile_names.add(image)
 for table,rows in table_rows.items():
  path=ROOT/'Resources/Tables'/(table+'.tsv');before=path.read_bytes().split(marker)[0]
  # Replace only explicitly authored atlas rows if present before our marker.
  # Every unrelated/original row retains its exact bytes and line ending.
  explicit={p['name'] if table=='PartsList' else p['texture']['image']+'.png' for p in spec['parts'] if p.get('texture')}
  replacement={row.split(b'\t')[0]:row for row in rows}
  before=b''.join(replacement.get(line.split(b'\t')[0],line) if line.split(b'\t')[0].decode(errors='replace') in explicit else line for line in before.splitlines(keepends=True))
  existing={line.split(b'\t')[0].strip() for line in before.splitlines() if line and not line.startswith(b'//')}
  additions=[row for row in rows if row.split(b'\t')[0] not in existing]
  separator=b'' if before.endswith(b'\n') else b'\n'
  path.write_bytes(before+separator+marker+b'\n'+b''.join(additions))
 for p in authored_parts:
  sockets=b''.join(struct.pack('<ff',*xy) for xy in p['sockets'])
  parts.append(encode(1,named(p['name'])+bytes([p['type'],p['tileCount'],len(p['sockets'])])+sockets))
  if p.get('imageSource'):
   original=ROOT/'Resources/Images'/(p['imageSource']+'.png');dest=ROOT/'Resources/Images'/(p['name']+'.png');dest.write_bytes(original.read_bytes())
 def part(p):return encode(4,named(p['name'])+bytes([p['parentSocket'],p['zOrder']]),[part(c) for c in p['children']])
 machines=[encode(3,named(m['name']),[part(p) for p in m['parts']]) for m in authored_machines]
 report={'source':str(src.relative_to(ROOT)),'sourceSHA256':hashlib.sha256(src.read_bytes()).hexdigest(),'parts':len(parts),'machines':len(machines),'persistedLevelVariants':len(spec['levelVariants']),'runtimeVariantGeneration':False,'visualLevelVariationImplemented':any('parts' in v for v in spec['levelVariants']),'visualLevelVariationScope':[v['name'] for v in spec['levelVariants'] if 'parts' in v],'containers':[]}
 for name,kind,additions in [('Parts',0,parts),('Machines',2,machines)]:
  standalone=ROOT/'Resources/RawData'/('Aircraft'+name+'.mcb');standalone.write_bytes(encode(kind,children=additions))
  path=ROOT/'Resources/RawData'/('Level10'+name+'.mcb');tree,end=decode(path.read_bytes());assert end==path.stat().st_size
  kept=[n['encoded'] for n in tree['children'] if not node_name(n).startswith(PREFIXES)]
  data=encode(tree['kind'],tree['data'],kept+additions);path.write_bytes(data)
  parsed,end=decode(data);assert [n['encoded'] for n in parsed['children'][:len(kept)]]==kept
  report['containers'].append({'path':str(path.relative_to(ROOT)),'retainedNodes':len(kept),'retainedNodeSHA256':hashlib.sha256(b''.join(kept)).hexdigest(),'addedNodes':len(additions),'sha256':hashlib.sha256(data).hexdigest()})
 out=ROOT/'Art/Aircraft/runtime/definition-resources.json';out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
if __name__=='__main__':run()
