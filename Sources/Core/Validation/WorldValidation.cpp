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
void World::setValidationCells(float value){
    if(!std::isfinite(value)||value<0)throw std::invalid_argument("Invalid validation cell amount");
    clock_.setCells(value);
}
bool World::setValidationHealth(ActorId id,int value){
    auto* a=actor(id);if(!a||a->removed)return false;
    if(value<=0){if(a->hp>0)damage(id,a->hp);}else a->hp=value;
    return true;
}
ActorId World::validationSpawn(std::string_view name,bool player,Vec2 position){
    if(!started_||!std::isfinite(position.x)||!std::isfinite(position.y))throw std::invalid_argument("Invalid validation spawn");
    return addNamed(name,position,player).id;
}

}
