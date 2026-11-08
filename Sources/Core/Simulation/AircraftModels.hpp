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
 if(rootPart=="ATransportBody")return .765f*(164.f/196.f);
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
// Uses original PartInfo/MachInfo chunks. All new images face right, like
// original tank parts; no separate rendering-only heading correction is needed.
// Shared assembly source for campaign, menus and isolated validation scenes.
inline void appendAircraftModels(BattleModels& models){
 if(models.machine("AircraftLight"))return;
 auto named=[](unsigned type,const char* name){Chunk c;c.type=type;c.data.resize(24);std::string n=name;if(n.size()>=24)throw std::runtime_error("Part name too long");std::copy(n.begin(),n.end(),c.data.begin());return c;};
 auto part=[&](const char* name,unsigned type,std::initializer_list<Vec2> sockets){auto c=named(1,name);c.data.push_back(type);c.data.push_back(std::string_view(name)=="AHeavyRocket"||std::string_view(name)=="AHeavyAirStrike"?0:1);c.data.push_back(sockets.size());for(auto s:sockets)for(float v:{s.x,s.y}){auto u=std::bit_cast<std::uint32_t>(v);for(int i=0;i<4;++i)c.data.push_back((u>>(8*i))&255);}return c;};
 auto item=[&](const char* name,unsigned socket,unsigned z){auto c=named(4,name);c.data.push_back(socket);c.data.push_back(z);return c;};
 Chunk parts;parts.type=0;
 parts.children={
  // AH-1Z Scout: hub on the spine, chin gun, one existing wing rocket pod.
  // These are sprite attachment/muzzle coordinates, not weapon-stat changes.
  // User's body-only aspect change: length *.70, width *1.30. Move mounts
  // with the fuselage; keep rotor and weapon image dimensions unchanged.
  part("ALightBody",0,{{0,0},{7.21f,0},{28,0},{3.71f,15.6f}}), part("ALightRotor",7,{{0,0}}),
  part("ALightGun",4,{{0,0},{7,1},{7,-1}}), part("ALightMissile",4,{{0,0},{6.5f,1},{6.5f,-1}}),
  part("AHeavyBody",0,{{0,0},{-10,10},{-10,-10},{8,10},{5,-34},{36,0},{5,34},{26,-18}}), part("AHeavyRotor",7,{{0,0}}),
  part("AHeavyGun",4,{{0,0},{12,2},{12,-2}}), part("AHeavyMissile",4,{{0,0},{8,2},{8,-2}}),
  part("AHeavyCannon",4,{{0,0},{9,0}}),
  // These mounts bind launchers already painted into the approved fuselage.
  part("AHeavyRocket",4,{{0,0},{12,0}}),part("AHeavyAirStrike",4,{{0,0},{10,0}}),
  part("ADuctBody",0,{{0,0},{11,28.6f},{11,-28.6f},{54,0},{-13,19},{-13,-19}}),
  part("ADuctFan",7,{{0,0}}),part("ADuctGun",0,{{0,0},{9,0}}),part("ADuctMissile",4,{{0,0},{9,1},{9,-1}}),
  part("ATransportBody",0,{{0,0},{10,1},{0,0},{0,0},{0,0},{8,32},{8,-32},{-2,47},{-2,-47}}),part("ATransportFan",7,{{0,0}}),
  part("ATransportWingL",0,{{0,0}}),part("ATransportWingR",0,{{0,0}}),part("ATransportEngine",0,{{0,0}}),
  part("ATransportGun",4,{{0,0},{10,1},{10,-1}}),part("ATransportMissile",4,{{0,0},{10,1},{10,-1}})
 };
 Chunk machines;machines.type=2;
 auto light=named(3,"AircraftLight");auto lb=item("ALightBody",0,1);
 lb.children={item("ALightGun",2,2),item("ALightMissile",3,2),item("ALightRotor",1,3)};light.children={lb};
 auto heavy=named(3,"AircraftHeavy");auto hb=item("AHeavyBody",0,1);
 hb.children={item("AHeavyGun",3,2),item("AHeavyMissile",4,2),item("AHeavyCannon",5,2),item("AHeavyRocket",6,2),item("AHeavyAirStrike",7,2),item("AHeavyRotor",1,3),item("AHeavyRotor",2,3)};heavy.children={hb};
 auto duct=named(3,"AircraftDuct");auto db=item("ADuctBody",0,1);
 db.children={item("ADuctFan",1,2),item("ADuctFan",2,2),item("ADuctGun",3,3),item("ADuctMissile",4,3),item("ADuctMissileR",5,3)};duct.children={db};
 auto transport=named(3,"AircraftTransport");auto tb=item("ATransportBody",0,1);
 tb.children={item("ATransportFan",1,4),item("ATransportGun",5,3),item("ATransportGun",6,3),item("ATransportMissile",7,3),item("ATransportMissile",8,3)};transport.children={tb};
 machines.children={light,heavy,duct,transport};models.append(parts,machines);
 models.appendPartAlias("ADuctMissileR","ADuctMissile",4);
}
}
