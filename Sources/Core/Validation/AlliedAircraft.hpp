#pragma once
#include "Core/Simulation/PlayerParts.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <stdexcept>
namespace md::validation {
struct AlliedAircraftTrial {const char* model;const char* reference;float widthFraction,speed,height;};
// Width fractions are full left-to-right trial spans, not altitude/range rules.
// Medium-heavy uses medium's span until the user's "10+" unit is resolved.
inline constexpr std::array<AlliedAircraftTrial,4> alliedAircraft{{
 {"AircraftLight","MachineGun",.50f,2.0f,12},
 {"AircraftHeavy","HMG",.425f,1.8f,18},
 {"AircraftDuct","HeavyMissile",.425f,1.6f,18},
 {"AircraftTransport","IonRifle",.25f,1.4f,22}
}};
struct AircraftTrialEconomy {int cells=0,upgradeCredits=0,hp=0;float duration=0;};
inline AircraftTrialEconomy aircraftTrialEconomy(unsigned kind,unsigned level,const BuildCatalog& builds,const DataTable& items,BattleGeometry g){
 if(kind>=alliedAircraft.size()||level<1||level>10)throw std::out_of_range("Aircraft trial kind/level");
 char name[48];std::snprintf(name,sizeof name,"%s_%02u",alliedAircraft[kind].reference,level);
 auto d=builds.find(name);if(!d)throw std::runtime_error("Missing reference build");
 auto p=PlayerLoadout::fromBuild(*d,items,g);
 return {((d->cost*5+3)/4+4)/5*5,d->upgradeCost,std::max(1,int(std::lround(p.hp*.70))),d->duration};
}
}
