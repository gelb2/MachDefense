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
WorldActor* World::selectTarget(WorldActor& a){
    auto* target=actor(a.target);if(target&&(target->hp<=0||target->removed)){a.target=0;a.moveToTarget=false;target=nullptr;}
    const bool aiEnabled=a.player||!(a.statusFlags&2);
    if(aiEnabled&&(!a.player||a.type==0||a.type==1||a.type==3)&&!target&&time()>a.attackCheck){
        float shortest=a.range*a.range;
        for(const auto& b:actors_)if(b->player!=a.player&&!b->removed&&b->hp>0){float d=distanceSquared(a.position,b->position);if(d<shortest){shortest=d;target=b.get();}}
        if(target){a.target=target->id;a.playerAI.isOrder=false;a.moveToTarget=a.player?target->position.y<baseBoundary_+.8f*a.range:target->position.y>-.8f*a.range-baseBoundary_;}
        a.attackCheck=float(time()+1);
    }
    return target;
}
void World::updatePursuit(WorldActor& a,WorldActor* target){
    if(target&&time()>double(a.targetCheck)+.5&&!(a.statusFlags&2)){
        if(a.moveToTarget)setDestination(a,distanceSquared(a.position,target->position)<(.8f*a.range)*(.8f*a.range)?a.position:target->position);
        a.targetCheck=float(time());
    }
}
}
