#pragma once
#include "Core/Contracts/RenderTypes.hpp"
#include <cmath>
#include <algorithm>
#include <string_view>
namespace md {
inline bool helicopterBody(std::string_view name){return name=="ALightBody"||name=="AHeavyBody"||name=="ADuctBody"||name=="ATransportBody";}
inline unsigned aircraftBankTile(float bank){
 const auto step=unsigned(std::clamp(std::lround(std::abs(bank)/.13962634f),0l,3l));
 return !step?0:step+(bank>0?3:0);
}
inline float aircraftDrawnBank(float bank){
 const unsigned tile=aircraftBankTile(bank);
 return tile==0?0.f:(tile<=3?-float(tile):float(tile-3))*.13962634f;
}
// Project the same local point for sockets, weapon muzzle and textured parts.
// Bank is roll around the nose/tail axis; heading remains independent.
inline Vec2 aircraftBankPoint(Vec2 p,float heading,float bank){
 bank=aircraftDrawnBank(bank);
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
 auto r=s.sourcePx.value_or(Rect{0,0,imageSize.x,imageSize.y});
 const auto name=std::string_view(s.image);
 const bool body=name.ends_with(".png")&&helicopterBody(name.substr(0,name.size()-4));
 if(body)r.x+=float(aircraftBankTile(bank))*r.width;
 const float angle=-s.rotationDegrees*3.14159265359f/180.f,c=std::cos(angle),t=std::sin(angle);
 const Vec2 center{s.destination.x+s.destination.width*.5f,s.destination.y+s.destination.height*.5f};
 for(auto q:{Vec2{0,0},Vec2{1,0},Vec2{0,1},Vec2{1,1}}){
  float x=(q.x-.5f)*s.destination.width,y=(.5f-q.y)*s.destination.height;
  const Vec2 local{center.x-pivot.x+x*c-y*t,pivot.y-center.y+x*t+y*c};
  // The body tile already depicts volume at this roll. Do not flatten/shear it
  // again. Independently mounted parts and their muzzles use the same roll step.
  auto p=body?local:aircraftBankPoint(local,heading,bank);
  const float u=s.flipX?1-q.x:q.x,v=s.flipY?1-q.y:q.y;
  m.vertices.push_back({{pivot.x+p.x,pivot.y-p.y},{(r.x+u*r.width)/imageSize.x,(r.y+v*r.height)/imageSize.y},s.color});
 }
 return m;
}
}
