#include "Core/Simulation/World.hpp"
#include "Core/Simulation/Formation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>

#include "Core/Simulation/WorldSupport.hpp"
#include "Core/Validation/AircraftComparison.hpp"
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
void World::validationAircraftComparison(){
    if(!started_)throw std::runtime_error("Start comparison World first");
    validation::appendAircraftModels(models_);validationArtScene_=true;
    actors_.clear();spawners_.clear();loot_.clear();projectiles_.clear();trails_.clear();airStrikes_.clear();production_.clear();
    for(auto& p:particles_->particles())p.shown=false;
    const float s=geometry_.deviceScale;
    // All four use the same world scale, facing north. Camera centers on them.
    camera_=actualCamera_=0;onBottom_=onTop_=false;cameraVelocity_=0;
    for(unsigned i=0;i<4;++i){
        const bool heli=i%2==1,heavy=i>=2;
        auto& a=addNamed(heavy?"E_Tank_32":"E_Tank_01",{(heli?100.f:-100.f)*s,(heavy?-100.f:100.f)*s},false);
        a.angle=a.vehicle.angle=a.aircraft.angle=1.57079632679f;
        a.enemyAI.stopDelay=a.attackCheck=1e9f;a.weapons.clear();a.range=0;
        if(heli){a.name=heavy?"AircraftHeavy":"AircraftLight";a.assembly=*models_.machine(a.name);a.type=2;a.player=true;a.artHeadingOffset=-1.57079632679f;a.flightHeight=heavy?18.f:12.f;a.rotorStep=7.3f*6.28318530718f/60.f;}
    }
}

}
