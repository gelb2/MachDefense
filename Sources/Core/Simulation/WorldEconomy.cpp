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
bool World::enqueueBuild(std::string_view name,std::optional<Vec2> destination){
    if(!started_||paused_||state_!=WorldState::running)return false;
    const auto owned=save_.builds(slot_);const auto selected=std::find_if(owned.begin(),owned.end(),[&](const auto& b){return b.name==name;});if(selected==owned.end())return false;
    char suffix[16];std::snprintf(suffix,sizeof suffix,"_%02u",selected->level+1);const auto* definition=builds_.find(selected->name+suffix);if(!definition)return false;
    float cells=clock_.cells();bool accepted=production_.enqueue(*definition,time(),buildFactor_,cells,destination);clock_.setCells(cells);
    // BuildItem::init (0x30254) plays this only after a queue item is created.
    if(accepted)host_.playSound("BuildMach.wav");return accepted;
}
std::size_t World::productionOccupied(std::string_view baseName)const{
    const auto owned=save_.builds(slot_);const auto i=std::find_if(owned.begin(),owned.end(),[&](const auto& b){return b.name==baseName;});
    if(i==owned.end())return 0;char suffix[16];std::snprintf(suffix,sizeof suffix,"_%02u",i->level+1);return production_.occupied(i->name+suffix);
}
void World::refreshBaseUpgrades(){
    const float baseCapacity=upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::cellStorage);
    capacity_=baseCapacity+float(cellLevel_)*baseCapacity;buildFactor_=upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::buildTime);
    clock_.configure(capacity_,upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::cellResearch));
    auto* a=actor(playerBase_);if(!a)return;auto loadout=PlayerLoadout::forBase(save_,slot_,upgrades_,geometry_);
    if(a->maxHP!=loadout.hp)a->hp=a->maxHP=loadout.hp;
    if(a->weapons.empty()||a->weapons.front().name!=loadout.weapon){
        // DlgUpgradeBase::setUpgrade (0x344cc) replaces only the two guns.
        // setStage (0x1627c) applies body/radar appearances on stage entry.
        auto replaceGuns=[&](auto&& self,std::vector<AssemblyPart>& parts)->void{
            for(auto& part:parts){if(weapons_.find(part.name))part.name=loadout.weapon;self(self,part.children);}
        };
        replaceGuns(replaceGuns,a->assembly);configureWeapons(*a);
    }
}
bool World::upgradeCells(){
    if(!started_||paused_||state_!=WorldState::running||cellLevel_>=5)return false;
    const int cost=cellUpgradeCost();if(clock_.cells()<float(cost))return false;
    clock_.setCells(clock_.cells()-float(cost));++cellLevel_;refreshBaseUpgrades();
    host_.playSound("UpgradeBase.wav");return true;
}

}
