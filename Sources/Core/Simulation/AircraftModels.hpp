#pragma once
#include "Core/Simulation/Battle.hpp"
#include "Core/Simulation/Combat.hpp"
#include <bit>
#include <stdexcept>
namespace md {
// 2026-11-08 size-only revision, relative to the packed art in 7e7fa8f.
// Keep the PNG pixels intact; apply this to the whole pose, including muzzles.
inline constexpr float aircraftAssemblyScale(std::string_view rootPart){
 if(rootPart=="AHeavyBody")return 1.20f;
 if(rootPart=="ADuctBody"||rootPart=="ATransportBody")return .85f;
 return 1.f;
}
struct AircraftWeaponBinding { const char* part; const char* original; };
inline constexpr AircraftWeaponBinding aircraftWeapons[]{
 {"ALightGun","E_Turret_Wpn_02"}, {"ALightMissile","E_Turret17_Wpn_01"},
 {"AHeavyGun","E_Tank_Wpn_13"}, {"AHeavyMissile","E_Tank28_Wpn_01"},
 {"AHeavyCannon","E_Tank31_Wpn"},
 {"ADuctGun","P_Wpn_HMG_01"}, {"ADuctMissile","E_Tank28_Wpn_01"},
 {"ATransportGun","E_Tank31_Wpn"}, {"ATransportMissile","P_Wpn_Msl_206"}
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
 auto part=[&](const char* name,unsigned type,std::initializer_list<Vec2> sockets){auto c=named(1,name);c.data.push_back(type);c.data.push_back(1);c.data.push_back(sockets.size());for(auto s:sockets)for(float v:{s.x,s.y}){auto u=std::bit_cast<std::uint32_t>(v);for(int i=0;i<4;++i)c.data.push_back((u>>(8*i))&255);}return c;};
 auto item=[&](const char* name,unsigned socket,unsigned z){auto c=named(4,name);c.data.push_back(socket);c.data.push_back(z);return c;};
 Chunk parts;parts.type=0;
 parts.children={
  part("ALightBody",0,{{0,0},{0,0},{12,15},{12,-15}}), part("ALightRotor",7,{{0,0}}),
  part("ALightGun",4,{{0,0},{8,2},{8,-2}}), part("ALightMissile",4,{{0,0},{7,2},{7,-2}}),
  part("AHeavyBody",0,{{0,0},{-10,10},{-10,-10},{8,10},{5,-34},{36,0}}), part("AHeavyRotor",7,{{0,0}}),
  part("AHeavyGun",4,{{0,0},{12,2},{12,-2}}), part("AHeavyMissile",4,{{0,0},{8,2},{8,-2}}),
  part("AHeavyCannon",4,{{0,0},{9,0}}),
  part("ADuctBody",0,{{0,0},{11,28.6f},{11,-28.6f},{54,0},{-13,19},{-13,-19}}),
  part("ADuctFan",7,{{0,0}}),part("ADuctGun",4,{{0,0},{9,0}}),part("ADuctMissile",4,{{0,0},{9,1},{9,-1}}),
  part("ATransportBody",0,{{0,0},{10,1},{0,0},{0,0},{0,0},{8,32},{8,-32},{-2,47},{-2,-47}}),part("ATransportFan",7,{{0,0}}),
  part("ATransportWingL",0,{{0,0}}),part("ATransportWingR",0,{{0,0}}),part("ATransportEngine",0,{{0,0}}),
  part("ATransportGun",4,{{0,0},{10,1},{10,-1}}),part("ATransportMissile",4,{{0,0},{10,1},{10,-1}})
 };
 Chunk machines;machines.type=2;
 auto light=named(3,"AircraftLight");auto lb=item("ALightBody",0,1);
 lb.children={item("ALightGun",2,2),item("ALightMissile",3,2),item("ALightRotor",1,3)};light.children={lb};
 auto heavy=named(3,"AircraftHeavy");auto hb=item("AHeavyBody",0,1);
 hb.children={item("AHeavyGun",3,2),item("AHeavyMissile",4,2),item("AHeavyCannon",5,2),item("AHeavyRotor",1,3),item("AHeavyRotor",2,3)};heavy.children={hb};
 auto duct=named(3,"AircraftDuct");auto db=item("ADuctBody",0,1);
 db.children={item("ADuctFan",1,2),item("ADuctFan",2,2),item("ADuctGun",3,3),item("ADuctMissile",4,3),item("ADuctMissile",5,3)};duct.children={db};
 auto transport=named(3,"AircraftTransport");auto tb=item("ATransportBody",0,1);
 tb.children={item("ATransportFan",1,4),item("ATransportGun",5,3),item("ATransportGun",6,3),item("ATransportMissile",7,3),item("ATransportMissile",8,3)};transport.children={tb};
 machines.children={light,heavy,duct,transport};models.append(parts,machines);
}
}
