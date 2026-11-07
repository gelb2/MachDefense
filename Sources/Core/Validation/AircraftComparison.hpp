#pragma once
#include "Core/Simulation/Battle.hpp"
#include "Core/Simulation/Combat.hpp"
#include <bit>
#include <stdexcept>
namespace md::validation {
struct AircraftWeaponBinding { const char* part; const char* original; };
inline constexpr AircraftWeaponBinding aircraftWeapons[]{
 {"ALightGun","E_Turret_Wpn_02"}, {"ALightMissile","E_Turret17_Wpn_01"},
 {"AHeavyGun","E_Tank_Wpn_13"}, {"AHeavyMissile","E_Tank28_Wpn_01"},
 {"AHeavyCannon","E_Tank31_Wpn"}
};
inline void appendAircraftWeapons(WeaponCatalog& weapons){
 for(const auto& b:aircraftWeapons)if(!weapons.find(b.part))weapons.addAlias(b.part,b.original);
}
// Uses original PartInfo/MachInfo chunks. All new images face right, like
// original tank parts; no separate rendering-only heading correction is needed.
// Called only by the isolated validation World, never by the campaign.
inline void appendAircraftModels(BattleModels& models){
 if(models.machine("AircraftLight"))return;
 auto named=[](unsigned type,const char* name){Chunk c;c.type=type;c.data.resize(24);std::string n=name;if(n.size()>=24)throw std::runtime_error("Part name too long");std::copy(n.begin(),n.end(),c.data.begin());return c;};
 auto part=[&](const char* name,unsigned type,std::initializer_list<Vec2> sockets){auto c=named(1,name);c.data.push_back(type);c.data.push_back(1);c.data.push_back(sockets.size());for(auto s:sockets)for(float v:{s.x,s.y}){auto u=std::bit_cast<std::uint32_t>(v);for(int i=0;i<4;++i)c.data.push_back((u>>(8*i))&255);}return c;};
 auto item=[&](const char* name,unsigned socket,unsigned z){auto c=named(4,name);c.data.push_back(socket);c.data.push_back(z);return c;};
 Chunk parts;parts.type=0;
 parts.children={
  part("ALightBody",0,{{0,0},{6,0},{4,10},{4,-10}}), part("ALightRotor",7,{{0,0}}),
  part("ALightGun",4,{{0,0},{5,1},{5,-1}}), part("ALightMissile",4,{{0,0},{4,1},{4,-1}}),
  part("AHeavyBody",0,{{0,0},{6,34},{6,-34},{24,0},{-6,-34},{-6,34}}), part("AHeavyRotor",7,{{0,0}}),
  part("AHeavyGun",4,{{0,0},{9,2},{9,-2}}), part("AHeavyMissile",4,{{0,0},{8,2},{8,-2}}),
  part("AHeavyCannon",4,{{0,0},{11,0}})
 };
 Chunk machines;machines.type=2;
 auto light=named(3,"AircraftLight");auto lb=item("ALightBody",0,1);
 lb.children={item("ALightGun",2,2),item("ALightMissile",3,2),item("ALightRotor",1,3)};light.children={lb};
 auto heavy=named(3,"AircraftHeavy");auto hb=item("AHeavyBody",0,1);
 hb.children={item("AHeavyGun",3,2),item("AHeavyMissile",4,2),item("AHeavyCannon",5,2),item("AHeavyRotor",1,3),item("AHeavyRotor",2,3)};heavy.children={hb};
 machines.children={light,heavy};models.append(parts,machines);
}
}
