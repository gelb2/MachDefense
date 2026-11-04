#!/usr/bin/env python3
"""Export immutable ARM32 ObjC layout evidence outside the repository."""
import argparse,struct,json,hashlib,re,subprocess
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('--binary',type=Path,required=True)
parser.add_argument('--output',type=Path,required=True)
a=parser.parse_args();P=a.binary.resolve();OUT=a.output.resolve()
ROOT=Path(__file__).resolve().parents[2]
if P.is_relative_to(ROOT) or OUT.is_relative_to(ROOT):
 raise SystemExit('Input binary and analysis output must stay outside repository')
OUT.mkdir(parents=True,exist_ok=True)
b=P.read_bytes();assert struct.unpack_from('<I',b)[0]==0xfeedface
ncmds=struct.unpack_from('<I',b,16)[0];segments=[];sections=[];symtab=None;at=28
for _ in range(ncmds):
 cmd,size=struct.unpack_from('<II',b,at)
 if cmd==1:
  name=b[at+8:at+24].split(b'\0')[0].decode();vm,vms,fo,fs=struct.unpack_from('<IIII',b,at+24);segments.append({'name':name,'vm_address':vm,'vm_size':vms,'file_offset':fo,'file_size':fs})
  count=struct.unpack_from('<I',b,at+48)[0]
  for j in range(count):
   a=at+56+j*68;sn=b[a:a+16].split(b'\0')[0].decode();sg=b[a+16:a+32].split(b'\0')[0].decode();addr,sz,off=struct.unpack_from('<III',b,a+32)
   sections.append({'name':sn,'segment':sg,'address':addr,'size':sz,'offset':off})
 elif cmd==2:symtab=struct.unpack_from('<IIII',b,at+8)
 at+=size
def off(addr,size=1):
 for s in segments:
  if s['vm_address']<=addr and addr+size<=s['vm_address']+s['file_size']:return s['file_offset']+addr-s['vm_address']
 raise ValueError(f'Not file-backed: {addr:#x}+{size}')
def words(addr,count):return struct.unpack_from('<'+'I'*count,b,off(addr,4*count))
def string(addr):
 if not addr:return None
 a=off(addr);return b[a:b.index(b'\0',a)].decode('utf-8','replace')
def methods(addr):
 if not addr:return []
 ents,count=words(addr,2);assert 12<=ents<=64 and count<10000
 result=[]
 for i in range(count):
  name,types,imp=words(addr+8+i*ents,3);result.append({'selector':string(name),'type_encoding':string(types),'implementation_raw':hex(imp),'address':hex(imp&~1),'thumb':bool(imp&1)})
 return result
def ivars(addr):
 if not addr:return []
 ents,count=words(addr,2);assert 20<=ents<=64 and count<10000
 result=[]
 for i in range(count):
  offsetptr,name,typ,align,size=words(addr+8+i*ents,5);result.append({'name':string(name),'type_encoding':string(typ),'offset_slot':hex(offsetptr),'offset':words(offsetptr,1)[0] if offsetptr else None,'alignment_log2':align,'size':size})
 return result
def props(addr):
 if not addr:return []
 ents,count=words(addr,2);assert ents>=8 and count<10000
 return [{'name':string(words(addr+8+i*ents,2)[0]),'attributes':string(words(addr+8+i*ents,2)[1])} for i in range(count)]
def readclass(addr,meta=False):
 isa,superaddr,cache,vtable,data=words(addr,5);data&=~3
 flags,start,size,layout,name,ml,protocol,iv,weak,properties=words(data,10)
 result={'name':string(name),'address':hex(addr),'superclass_address':hex(superaddr),'instance_start':start,'instance_size':size,'flags':flags,'methods':methods(ml),'ivars':ivars(iv),'properties':props(properties),'protocol_list_address':hex(protocol)}
 if not meta:result['class_methods']=readclass(isa,True)['methods']
 return result
classsec=next(s for s in sections if s['name']=='__objc_classlist');classes=[readclass(a) for a in words(classsec['address'],classsec['size']//4)]
cn={c['address']:c['name'] for c in classes}
for c in classes:c['superclass']=cn.get(c['superclass_address']);c['superclass_resolution']='local' if c['superclass'] else 'external_or_root_unresolved'
symbols=[]
if symtab:
 so,n,st,ss=symtab
 for i in range(n):
  idx,typ,sect,desc,val=struct.unpack_from('<IBBHI',b,so+i*12)
  name=b[st+idx:b.index(b'\0',st+idx)].decode('utf-8','replace') if idx else ''
  symbols.append({'name':name,'address':hex(val),'n_type':typ,'section_index':sect,'description':desc,'section':sections[sect-1]['name'] if 0<sect<=len(sections) else None})
# Resolve external superclass fixups from the original relocation table.
reltext=subprocess.check_output(['xcrun','otool','-rv',str(P)],text=True)
(OUT/'relocations-otool.txt').write_text(reltext)
rel={int(a,16):n for a,n in re.findall(r'^([0-9a-f]{8}) False +long +True +VANILLA False +_OBJC_CLASS_\$_(\w+)$',reltext,re.M)}
for c in classes:
 if not c['superclass'] and int(c['address'],16)+4 in rel:
  c['superclass']=rel[int(c['address'],16)+4];c['superclass_resolution']='external_relocation'
result={'source':str(P),'sha256':hashlib.sha256(b).hexdigest(),'architecture':'armv7','pointer_size':4,'executable_source':False,'limitations':['Original 32-bit offsets describe evidence, not native arm64 object layout.','Zero superclass pointers require import/fixup resolution; not treated as NSObject by assumption.'],'classes':classes,'segments':segments,'sections':sections,'symbols':symbols}
(OUT/'objc-layouts.json').write_text(json.dumps(result,indent=2)+'\n')

print(json.dumps({'sha256':result['sha256'],'classes':len(classes),'methods':sum(len(c['methods'])+len(c['class_methods']) for c in classes),'ivars':sum(len(c['ivars']) for c in classes)}))
