#include "Core/Simulation/World.hpp"
#include "Core/Simulation/Formation.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>

#include "Core/Simulation/WorldSupport.hpp"
namespace md {
using namespace world_detail;
World::World(ResourceAccess& r,HostServices& h,SaveData& s,std::size_t slot,BattleGeometry g,std::uint32_t seed):
 resources_(r),host_(h),save_(s),slot_(slot),geometry_(g),random_(seed),
 machinesTable_(table(r,"MachList")),itemsTable_(table(r,"ItemInfo")),mapTable_(table(r,"MapList")),maps_(chunk(r,"MapInfo")),
 models_(BattleModels::load(r)),
 weapons_(WeaponCatalog::load(table(r,"WeaponInfo"),g.deviceScale==.5f)),
 builds_(BuildCatalog::load(table(r,"MachBuild_Parts"),table(r,"MachBuild_Name"))),upgrades_(table(r,"BaseUpgrade")),attacks_(table(r,"SpAttackList")){
    uiMargin_=g.deviceScale==.5f?62.f:178.f;
    if(slot>=SaveData::slotCount||!s.active(slot))throw std::invalid_argument("World requires an original active save slot");
    particles_=std::make_unique<ParticlePool>(float(time()),random_);
    countermeasures_=std::make_unique<AircraftCountermeasures>(geometry_.deviceScale);
    for(std::size_t i=0;i<itemsTable_.rowCount();++i){auto d=LootDefinition::fromTable(itemsTable_,i);if(d.type!=LootType::other)lootDefinitions_.emplace(d.code,std::move(d));}
}
WorldActor& World::addNamed(std::string_view name,Vec2 position,bool player){
    auto row=machinesTable_.findRow(name);if(!row)throw std::runtime_error("Unknown original machine "+std::string(name));
    const auto info=MachineDefinition::fromTable(machinesTable_,*row,geometry_);
    auto value=std::make_unique<WorldActor>();auto& a=*value;
    a.id=nextId_++;a.name=name;a.player=player;a.position=a.destination=position;
    if(!player)a.enemyAI=EnemySteering::begin(time(),random_);
    // makeMachFromName consumes an initial random direction before setDir.
    (void)random_.next();
    a.coin=int(machinesTable_.integer(*row,"Coin").value_or(0));a.coinRate=int(machinesTable_.integer(*row,"Coin%").value_or(0));
    a.hp=a.maxHP=info.hp;a.type=info.type;a.referenceType=info.referenceType;a.experience=info.experience;a.radius=float(info.radius);
    a.angle=player?0x1.921fb6p+0f:-0x1.921fb6p+0f;
    const auto* model=models_.machine(name);if(!model)throw std::runtime_error("Missing original machine assembly");a.assembly=*model;
    a.vehicle.moveSpeed=info.speed;a.aircraft.moveSpeed=info.speed;a.train.moveSpeed=info.speed;
    a.bipod.setStepSize(info.speed);a.bipod.position=a.vehicle.position=a.aircraft.position=a.train.position=position;
    a.bipod.destination=a.vehicle.destination=a.aircraft.destination=a.train.destination=position;
    a.bipod.angle=a.bipod.bodyAngle=a.bipod.baseAngle=a.vehicle.angle=a.aircraft.angle=a.angle;
    if(player)if(const auto* aircraft=alliedAircraftForModel(name)){
        a.flightTactics=AircraftTactics{aircraft->widthFraction,info.speed/geometry_.deviceScale,std::uint32_t(a.id*2654435761u)|1u};
        a.flightTactics->noseStep=aircraft->noseDegreesPerSecond*3.14159265f/10800.f;a.flightTactics->weaponStep=aircraft->weaponDegreesPerSecond*3.14159265f/10800.f;
        a.flightHeight=aircraft->height;a.rotorStep=7.3f*6.28318530718f/60.f;a.aircraft.rotationDivisor=12;
    }
    if(a.flightTactics){
        float low=0,high=0;
        for(const auto& pose:models_.pose(a.assembly,geometry_.deviceScale,0,0,0)){
            const auto size=resources_.imageSize(pose.definition->image);const float half=size.y/pose.definition->splitY*pose.deviceScale*.5f;
            low=std::min(low,pose.localCenter.y-half);high=std::max(high,pose.localCenter.y+half);
        }
        a.countermeasureWidth=high-low;
    }
    configureWeapons(a);actors_.push_back(std::move(value));return a;
}
WorldActor& World::addParts(const BuildDefinition& definition,Vec2 position){
    auto loadout=PlayerLoadout::fromBuild(definition,itemsTable_,geometry_);
    auto value=std::make_unique<WorldActor>();auto& a=*value;
    a.id=nextId_++;a.name=definition.name;a.player=true;a.hp=a.maxHP=loadout.hp;a.radius=loadout.radius;
    a.position=a.destination=position;a.angle=0x1.921fb6p+0f;a.assembly=std::move(loadout.assembly);
    a.bipod.position=a.bipod.destination=position;a.bipod.angle=a.bipod.bodyAngle=a.bipod.baseAngle=a.angle;a.bipod.setStepSize(loadout.stepSize);
    configureWeapons(a);actors_.push_back(std::move(value));return a;
}
void World::start(unsigned stage){
    if(stage>=80||stage>=maps_.children.size()||stage>=mapTable_.rowCount())throw std::out_of_range("Original stage index");
    validationArtScene_=false;
    stage_=stage;startTime_=time();statistics_={};actors_.clear();loot_.clear();projectiles_.clear();trails_.clear();airStrikes_.clear();spawners_.clear();production_.clear();events_.clear();
    for(auto& p:particles_->particles())p.shown=false;
    countermeasures_->clear();
    cellLevel_=0;state_=WorldState::running;paused_=false;started_=true;cameraTouch_.reset();radarTouch_.reset();cameraVelocity_=0;
    save_.writeWord(SaveData::slotStart+slot_*SaveData::slotStride+0x34,stage);
    capacity_=upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::cellStorage);
    buildFactor_=upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::buildTime);
    clock_.start(capacity_,upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::cellResearch));
    baseBoundary_=geometry_.halfLength()-50*geometry_.deviceScale;
    background_=mapTable_.value(stage,"ImageFile");
    char spawn[24];std::snprintf(spawn,sizeof spawn,"ESpawn_%02u",stage+1);
    plan_=StageSpawnPlan::parse(maps_.children.at(stage),table(resources_,spawn),geometry_);
    buildTerrain();
    for(const auto& event:plan_.placed)addNamed(event.name,event.worldPosition,false);
    plan_.appendGeneratedWaves(int(stage),random_,[&](std::string_view n){auto row=machinesTable_.findRow(n);if(!row)throw std::runtime_error("Unknown original wave machine");return MachineDefinition::fromTable(machinesTable_,*row,geometry_).hp;});
    for(const auto& rule:plan_.timed)spawners_.emplace_back(rule,time(),random_);
    director_.emplace(plan_.difficulty,save_.clearCount(slot_,stage));
    auto& enemy=addNamed("E_Base",{0,baseBoundary_},false);enemy.base=true;enemy.type=3;enemy.angle=0;enemy.hp=enemy.maxHP=BattleClock::enemyBaseHP(int(stage));enemyBase_=enemy.id;
    for(auto& w:enemy.weapons)w.controller.setAngle(0);
    auto base=std::make_unique<WorldActor>();auto& a=*base;auto loadout=PlayerLoadout::forBase(save_,slot_,upgrades_,geometry_);
    a.id=nextId_++;a.name="PlayerBase";a.player=a.base=true;a.type=3;a.hp=a.maxHP=loadout.hp;a.radius=loadout.radius;
    a.position=a.destination={0,84*geometry_.deviceScale-geometry_.halfLength()};a.angle=0x1.921fb6p+0f;a.assembly=std::move(loadout.assembly);
    configureWeapons(a);playerBase_=a.id;actors_.push_back(std::move(base));
    camera_=height()*.5f-bottomMargin()-geometry_.halfLength();clampCamera();actualCamera_=camera_;onBottom_=true;
}
WorldActor* World::actor(ActorId id){for(auto& a:actors_)if(a->id==id)return a.get();return nullptr;}
const WorldActor* World::actor(ActorId id)const{for(const auto& a:actors_)if(a->id==id)return a.get();return nullptr;}
void World::tick(){
    if(!started_)return;
    clock_.tick(true,paused_,int(state_));updateCamera();if(paused_)return;
    if(state_==WorldState::running&&!validationArtScene_){
        topPlayerPosition_=-800;
        if(auto generated=director_->tick(time(),random_))spawners_.push_back(std::move(*generated));
        std::vector<SpawnEvent> incoming;
        for(auto& s:spawners_)if(auto e=s.tick(time(),int(state_),geometry_,random_))incoming.push_back(std::move(*e));
        std::erase_if(spawners_,[](const auto& s){return s.finished();});
        for(const auto& e:incoming)addNamed(e.name,e.worldPosition,false);
        production_.tick(time(),geometry_,random_,[&](const ProducedUnit& unit){
            ++statistics_.produced;
            // BuildItem::tick (WeMade 1.0.3, 0x30644) plays sfxID 17 here.
            host_.playSound("BuildMachComplete.wav");
            WorldActor* a;
            if(unit.definition.setType==0)a=&addParts(unit.definition,unit.position);
            else if(unit.definition.placementType==0)a=&addNamed(unit.definition.machine,unit.position,true);
            else {a=&addNamed(unit.definition.bot,unit.position,true);a->postBuild=unit.definition.machine;}
            if(unit.destination)setDestination(*a,*unit.destination);
        });
    }
    // New carriers may create their final unit during this pass; process those
    // children on the following update, keeping actor references stable.
    aircraftThreats_.clear(geometry_.deviceScale);
    const bool aircraftActive=std::any_of(actors_.begin(),actors_.end(),[](const auto& a){return a->flightTactics&&a->hp>0&&!a->removed;});
    if(aircraftActive)for(const auto& p:projectiles_)if(const auto* w=weapons_.find(p.weapon))aircraftThreats_.add(p.motion,*w);
    const auto count=actors_.size();for(std::size_t i=0;i<count;++i)updateActor(*actors_[i]);
    for(auto& strike:airStrikes_)if(const auto* weapon=weapons_.find(strike.weapon))
        if(auto bullet=strike.motion.tick(*weapon,time(),random_))addProjectile(strike.weapon,*bullet,0,0);
    std::erase_if(airStrikes_,[](const auto& strike){return strike.motion.removed;});
    for(auto& trail:trails_)trail.motion->tick(time(),random_);
    std::erase_if(trails_,[](const auto& trail){return trail.motion->removed();});
    updateProjectiles();
    for(auto& p:particles_->particles())p.tick(float(time()),geometry_.deviceScale==.5f?240.f:512.f);
    countermeasures_->tick(float(time()));
    std::vector<LootCollector> collectors;for(const auto& a:actors_)if(a->player&&!a->removed)collectors.push_back({a->id,a->position,a->type,a->hp});
    for(auto& item:loot_)if(auto pickup=item.tick(time(),collectors,random_)){
        host_.playSound("Game_GetItem.wav");
        if(pickup->type==LootType::coin)save_.setCredits(slot_,save_.credits(slot_)+pickup->value);
        else if(pickup->type==LootType::mineral)clock_.setCells(pickup->applyCells(clock_.cells(),capacity_));
    }
    std::erase_if(loot_,[](const auto& l){return l.removed;});
    // New aircraft can be built repeatedly. Release their assembly/weapons and
    // per-entity dodge state after the original wreck lifetime has finished.
    // Original ground actor retention is unchanged.
    std::erase_if(actors_,[](const auto& a){return a->flightTactics&&a->removed;});
}
void World::finish(WorldState state){
    state_=state;events_.push_back({state==WorldState::cleared?WorldEvent::Kind::stageCleared:WorldEvent::Kind::stageFailed,0,0});
    if(finishHandler_)finishHandler_(state);
    std::vector<ActorId> ids;for(const auto& a:actors_)if(a->player==(state==WorldState::failed)&&a->hp>0)ids.push_back(a->id);
    for(auto id:ids){auto* a=actor(id);damage(id,a->hp);}
    if(state==WorldState::cleared)baseBoundary_=2048;
}
std::vector<WorldEvent> World::takeEvents(){auto e=std::move(events_);events_.clear();return e;}
}
