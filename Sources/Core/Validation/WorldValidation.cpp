#include "Core/Simulation/World.hpp"
#include "Core/Simulation/Formation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>

#include "Core/Simulation/WorldSupport.hpp"
#include "Core/Validation/AircraftComparison.hpp"
#include "Core/Validation/AlliedAircraft.hpp"
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
    validation::appendAircraftModels(models_);validation::appendAircraftWeapons(weapons_);validationArtScene_=true;
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
        if(heli){a.name=heavy?"AircraftHeavy":"AircraftLight";a.assembly=*models_.machine(a.name);a.type=2;a.player=true;a.artHeadingOffset=0;a.flightHeight=heavy?18.f:12.f;a.rotorStep=7.3f*6.28318530718f/60.f;configureWeapons(a);}
    }
}

ActorId World::validationAlliedAircraft(unsigned kind,Vec2 position){
    if(!started_||kind>=validation::alliedAircraft.size()||!std::isfinite(position.x)||!std::isfinite(position.y))throw std::invalid_argument("Aircraft trial spawn");
    auto economy=validation::aircraftTrialEconomy(kind,1,builds_,itemsTable_,geometry_);
    validation::appendAircraftModels(models_);validation::appendAircraftWeapons(weapons_);
    const auto& trial=validation::alliedAircraft[kind];
    auto& a=addNamed("E_Tank_01",position,true);
    a.name=trial.model;a.assembly=*models_.machine(a.name);a.type=2;a.hp=a.maxHP=economy.hp;
    a.coin=a.coinRate=a.experience=0;
    a.radius=(kind==0?12.f:kind==1?28.f:kind==2?25.f:30.f)*geometry_.deviceScale;
    a.flightHeight=trial.height;a.rotorStep=7.3f*6.28318530718f/60.f;
    a.aircraft.rotationDivisor=12;a.aircraft.moveSpeed=trial.speed*geometry_.deviceScale;
    a.flightTactics=AircraftTactics{trial.widthFraction,trial.speed,std::uint32_t(a.id*2654435761u)|1u};
    configureWeapons(a);
    // Only these new aircraft acquire using the longest attached weapon.
    for(const auto& w:a.weapons)if(auto d=weapons_.find(w.name))a.range=std::max(a.range,float(d->shotRange));
    return a.id;
}

void World::validationAlliedAircraftComparison(){
    validationAircraftComparison();actors_.clear();
    const float scale=geometry_.deviceScale;
    for(unsigned i=0;i<4;++i){
        const float x=(-165.f+110.f*i)*scale;
        auto& target=addNamed(i<2?"E_Tank_01":"E_Tank_32",{x,110*scale},false);
        target.hp=target.maxHP=100000;target.enemyAI.stopDelay=target.attackCheck=1e9f;
        target.weapons.clear();target.range=0;
        validationAlliedAircraft(i,{x,-90*scale});
    }
}

}
