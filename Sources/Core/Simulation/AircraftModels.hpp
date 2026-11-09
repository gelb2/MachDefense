#pragma once
#include "Core/Simulation/Battle.hpp"
#include "Core/Simulation/Combat.hpp"
#include <bit>
#include <stdexcept>
namespace md {
// Whole-assembly scale, including muzzles. The three larger aircraft retain
// their approved 2026-11-08 art; Scout uses the later AH-1Z design candidate.
inline constexpr float aircraftAssemblyScale(std::string_view rootPart){
 // Same-pose 2x web bounds: MachineGun_10=118px, E_Tank_32=210px.
 // Preserve aspect ratios; targets are tank*1.10, mech, and their midpoint.
 // Scout's new rotor-inclusive target remains Cheyenne's 118px at world 2x.
 if(rootPart=="ALightBody")return 1.f;
 if(rootPart=="AHeavyBody")return 1.32f*(231.f/296.f);
 if(rootPart=="ADuctBody")return .765f*(118.f/134.f);
 if(rootPart=="ATransportBody"||rootPart=="ATransportBodyV5"||rootPart.starts_with("ATransportBodyV5_"))return .765f*(164.f/196.f);
 return 1.f;
}
struct AircraftWeaponBinding { const char* part; const char* original; };
inline constexpr AircraftWeaponBinding aircraftWeapons[]{
 {"ALightGun","ALightGun_01"}, {"ALightMissile","ALightMissile_01"},
 {"AHeavyGun","AHeavyGun_01"}, {"AHeavyMissile","AHeavyMissile_01"},
 {"AHeavyCannon","AHeavyCannon_01"}, {"AHeavyRocket","AHeavyRocket_01"}, {"AHeavyAirStrike","AHeavyAirStrike_01"},
 {"ADuctMissile","ADuctMissile_01"}, {"ADuctMissileR","ADuctMissileR_01"},
 {"ATransportGun","ATransportGun_01"}, {"ATransportMissile","ATransportMissile_01"}
};
inline void appendAircraftWeapons(WeaponCatalog& weapons){
 for(const auto& b:aircraftWeapons)if(!weapons.find(b.part))weapons.addAlias(b.part,b.original);
}
// Definitions are prepared as QChunk files and loaded by the unchanged
// BattleModels resource loader before configureTiles. Never synthesize them here.
inline void appendAircraftModels(BattleModels& models){
 for(const char* name:{"AircraftLight","AircraftHeavy","AircraftDuct","AircraftTransport"})
  if(!models.machine(name))throw std::runtime_error("Missing aircraft MCB definitions; run tools/prepare_aircraft_definitions.py");
}
}
