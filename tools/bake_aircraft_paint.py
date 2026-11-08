#!/usr/bin/env python3
"""Bake approved helicopter-only paint; never read source pixels from outputs."""
from pathlib import Path
import hashlib,json,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
source=root/'Art/Aircraft/paint-baked-20261109/source'
output=root/'Resources/Images'
cpp=r'''
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Core/Presentation/AircraftPaint.hpp"
#include <fstream>
#include <string>
int main(int argc,char**argv){
 if(argc!=4)return 2;
 int w,h,c;auto*p=stbi_load(argv[1],&w,&h,&c,4);if(!p)return 3;
 md::prepareAircraftPaint(argv[3],p,std::size_t(w)*h,w,h);
 std::ofstream f(argv[2],std::ios::binary);
 f<<"P7\nWIDTH "<<w<<"\nHEIGHT "<<h<<"\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
 f.write(reinterpret_cast<char*>(p),std::size_t(w)*h*4);stbi_image_free(p);return f?0:4;
}
'''
rows=[]
with tempfile.TemporaryDirectory(prefix='machdefense-paint-') as td:
 td=Path(td);(td/'bake.cpp').write_text(cpp)
 subprocess.run(['c++','-std=c++20','-O2','-I'+str(root/'Sources'),'-I'+str(root/'Modules/ThirdParty/stb'),str(td/'bake.cpp'),'-o',str(td/'bake')],check=True)
 for p in sorted(source.glob('*.png')):
  assert p.name.startswith(('ALight','AHeavy','ADuct','ATransport'))
  subprocess.run([str(td/'bake'),str(p),str(td/'paint.pam'),p.name],check=True)
  target=output/p.name
  subprocess.run(['magick',str(td/'paint.pam'),'-strip','PNG32:'+str(target)],check=True)
  rows.append({'image':p.name,'source_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'output_sha256':hashlib.sha256(target.read_bytes()).hexdigest()})
assert len(rows)==28
(source.parent/'manifest.json').write_text(json.dumps({'source_commit':'5eefc3e','transform':'prepareAircraftPaint; approved palette and markings unchanged','textures':rows},indent=2)+'\n')
print('Baked 28 helicopter textures from preserved sources.')
