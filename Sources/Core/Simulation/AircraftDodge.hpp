#pragma once
#include "Core/Simulation/Projectile.hpp"
#include <array>
#include <algorithm>
#include <limits>
namespace md {
// Rebuilt once per world step, reused by all new aircraft. No actor timers,
// threads, per-query allocations, or changes to original projectile physics.
class AircraftThreatGrid {
public:
 struct Threat { Vec2 impact; float radius; bool toPlayer; int next=-1; };
 void clear(float scale){scale_=scale;heads_.fill(-1);threats_.clear();maxRadius_=0;}
 void add(const Projectile& source,const WeaponDefinition& weapon){
  if(source.removed||weapon.type==WeaponType::gun||weapon.type==WeaponType::sniper)return;
  auto predicted=source;bool impact=false;
  for(int i=0;i<21&&!impact;++i)impact=predicted.tick(weapon);
  if(!impact)return;
  const float radius=std::max(float(weapon.damageRange),weapon.type==WeaponType::laser?weapon.special1:0.f);
  const int cell=index(predicted.position);maxRadius_=std::max(maxRadius_,radius);
  threats_.push_back({predicted.position,radius,source.toPlayer,heads_[cell]});heads_[cell]=int(threats_.size()-1);
 }
 template<class Visitor> void near(Vec2 position,float movement,float bodyRadius,bool player,Visitor&& visit)const{
  const float extent=maxRadius_+movement+bodyRadius;
  const int x0=x(position.x-extent),x1=x(position.x+extent),y0=y(position.y-extent),y1=y(position.y+extent);
  for(int yy=y0;yy<=y1;++yy)for(int xx=x0;xx<=x1;++xx)
   for(int i=heads_[yy*columns+xx];i>=0;i=threats_[i].next){const auto& t=threats_[i];if(t.toPlayer==player)visit(t);}
 }
 std::size_t size()const{return threats_.size();}
 std::size_t retainedBytes()const{return threats_.capacity()*sizeof(Threat)+sizeof(heads_);}
private:
 static constexpr int columns=8,rows=32;
 float scale_=1,maxRadius_=0;
 std::array<int,columns*rows> heads_{};
 std::vector<Threat> threats_;
 int x(float n)const{return std::clamp(int(std::floor((n/scale_+256)/64)),0,columns-1);}
 int y(float n)const{return std::clamp(int(std::floor((n/scale_+1024)/64)),0,rows-1);}
 int index(Vec2 p)const{return y(p.y)*columns+x(p.x);}
};
}
