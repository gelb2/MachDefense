#include "Trail.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 md::WeaponDefinition w;w.effectType=5;w.effectImage="Trail.png";md::DarwinRandom random(1);
 auto trail=md::Trail::forWeapon(w,.5f,{100,100});assert(trail&&trail->width()==2&&trail->blockLength()==8);
 trail->add({100,100},1,random);assert(trail->lastPosition()==1);
 trail->add({100,124},1.1,random);assert(trail->lastPosition()==4);
 md::Frame frame;frame.sprites.resize(2);trail->appendMesh(frame,[](md::Vec2 p){return p;});
 assert(frame.meshes.size()==1&&frame.meshes[0].beforeSpriteIndex==2&&frame.meshes[0].vertices.size()==10);
 for(auto& v:frame.meshes[0].vertices)assert(std::isfinite(v.position.x)&&std::isfinite(v.position.y));
 trail->tick(10,random,true);assert(!trail->removed());trail->tick(10,random);assert(trail->removed());
 for(int type=5;type<=7;++type){w.effectType=type;auto t=md::Trail::forWeapon(w,1,{});assert(t);t->add({},0,random);for(int i=1;i<500;++i)t->add({float(i*100),float(i*10)},i*.01,random);assert(t->lastPosition()<=65);for(auto&p:t->points())assert(std::isfinite(p.positive.x));t->tick(10,random);assert(t->removed());}
 w.effectType=8;assert(!md::Trail::forWeapon(w,1,{}));std::cout<<"Original trail segment, mesh, pause, fade and rolling storage passed\n";
}
