#pragma once
#include "Core/Contracts/RenderTypes.hpp"
#include <cmath>
#include <string_view>
namespace md {
inline bool helicopterBody(std::string_view name){return name=="ALightBody"||name=="AHeavyBody"||name=="ADuctBody"||name=="ATransportBody";}
// Project the same local point for sockets, weapon muzzle and textured parts.
// Bank is roll around the nose/tail axis; heading remains independent.
inline Vec2 aircraftBankPoint(Vec2 p,float heading,float bank){
 const Vec2 left{-std::sin(heading),std::cos(heading)};
 const float across=p.x*left.x+p.y*left.y;
 const float compression=(std::cos(bank)-1)*across;
 // A slight oblique view exposes the raised/lowered side. A pure cos(bank)
 // squeeze produced identical left/right poses and made banking unreadable.
 const Vec2 forward{std::cos(heading),std::sin(heading)};
 const float rise=std::sin(bank)*across*.35f;
 return {p.x+left.x*compression+forward.x*rise,p.y+left.y*compression+forward.y*rise};
}
inline TextureMesh aircraftBankMesh(const Sprite& s,Vec2 pivot,float heading,float bank,Vec2 imageSize){
 TextureMesh m;m.image=s.image;m.blend=s.blend;m.nearest=s.nearest;
 const auto r=s.sourcePx.value_or(Rect{0,0,imageSize.x,imageSize.y});
 const float angle=-s.rotationDegrees*3.14159265359f/180.f,c=std::cos(angle),t=std::sin(angle);
 const Vec2 center{s.destination.x+s.destination.width*.5f,s.destination.y+s.destination.height*.5f};
 for(auto q:{Vec2{0,0},Vec2{1,0},Vec2{0,1},Vec2{1,1}}){
  float x=(q.x-.5f)*s.destination.width,y=(.5f-q.y)*s.destination.height;
  auto p=aircraftBankPoint({center.x-pivot.x+x*c-y*t,pivot.y-center.y+x*t+y*c},heading,bank);
  const float u=s.flipX?1-q.x:q.x,v=s.flipY?1-q.y:q.y;
  m.vertices.push_back({{pivot.x+p.x,pivot.y-p.y},{(r.x+u*r.width)/imageSize.x,(r.y+v*r.height)/imageSize.y},s.color});
 }
 return m;
}
}
