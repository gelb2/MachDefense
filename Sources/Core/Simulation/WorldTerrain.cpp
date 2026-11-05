#include "Core/Simulation/World.hpp"
#include "Core/Simulation/Formation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>

#include "Core/Simulation/WorldSupport.hpp"
namespace md {
using namespace world_detail;
void World::buildTerrain(){
    terrain_.clear();for(auto& cell:collisionGrid_)cell.clear();
    for(const auto& polygon:plan_.collisionPolygons){
        if(polygon.size()<2)continue;
        const auto first=terrain_.size();
        for(std::size_t i=0;i<polygon.size();++i){auto segment=TerrainSegment::between(polygon[i],polygon[(i+1)%polygon.size()]);if(i)segment.setPrevious(&terrain_.back());terrain_.push_back(segment);}
        // readColBox links each new edge to its preceding edge; the first
        // edge retains its initial corner normal.
        (void)first;
    }
    const float unit=128*geometry_.deviceScale,small=32*geometry_.deviceScale,large=small*3;
    for(std::size_t i=0;i<terrain_.size();++i){const auto& segment=terrain_[i];
        for(int x=0;x<4;++x)for(int y=0;y<16;++y){
            Vec2 origin{x*unit-geometry_.halfLength()*.25f,y*unit-geometry_.halfLength()};
            auto contains=[&](Vec2 p){return p.x>=origin.x-unit*.5f&&p.x<=origin.x+unit*1.5f&&p.y>=origin.y-unit*.5f&&p.y<=origin.y+unit*1.5f;};
            bool include=contains(segment.origin)||contains(segment.second);
            for(Vec2 offset:{Vec2{small,small},Vec2{large,small},Vec2{large,large},Vec2{small,large}})include|=segment.penetration({origin.x+offset.x,origin.y+offset.y},unit)!=0;
            if(include)collisionGrid_[std::size_t(y*4+x)].push_back(i);
        }
    }
}
void World::collideTerrain(WorldActor& a){
    a.collision.reset();const float unit=128*geometry_.deviceScale;
    const int x=int((a.position.x+geometry_.halfLength()*.25f)/unit),y=int((a.position.y+geometry_.halfLength())/unit);
    if(x<0||x>=4||y<0||y>=16)return;
    const auto& cell=collisionGrid_[std::size_t(y*4+x)];
    for(auto i=cell.rbegin();i!=cell.rend();++i){const auto& segment=terrain_[*i];const float penetration=segment.penetration(a.position,16);
        if(penetration>0){a.position.x-=segment.normal.x*penetration;a.position.y-=segment.normal.y*penetration;a.collision=CollisionSurface{segment.normal,segment.direction};}
    }
}
void World::collideGroundBodies(WorldActor& a){
    // Enemy/player overrides (0xdda8/0x10328) visit preceding living
    // same-side ground children, stopping at self so each pair runs once.
    if(a.base||a.type==2||(a.player&&a.type==5))return;
    for(const auto& other:actors_){
        if(other->player!=a.player||other->removed||other->hp<=0||other->base||other->type==2||(a.player&&other->type==5))continue;
        if(other.get()==&a)break;
        separateGroundBodies(a.position,a.radius,a.type==3,
                             other->position,other->radius,other->type==3);
    }
}

}
