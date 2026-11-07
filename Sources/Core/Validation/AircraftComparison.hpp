#pragma once
#include "Core/Simulation/Battle.hpp"
#include <bit>
#include <stdexcept>
namespace md::validation {
// Same chunk names, socket types and assembly tree as the original PartInfo/MachInfo.
// This adds prototypes only to the isolated comparison World, never the campaign tables.
inline void appendAircraftModels(BattleModels& models){
 if(models.machine("AircraftLight"))return;
 auto named=[](unsigned type,const char* name){Chunk c;c.type=type;c.data.resize(24);std::string n=name;if(n.size()>=24)throw std::runtime_error("Part name too long");std::copy(n.begin(),n.end(),c.data.begin());return c;};
 auto part=[&](const char* name,unsigned type,std::initializer_list<Vec2> sockets){auto c=named(1,name);c.data.push_back(type);c.data.push_back(1);c.data.push_back(sockets.size());for(auto s:sockets)for(float v:{s.x,s.y}){auto u=std::bit_cast<std::uint32_t>(v);for(int i=0;i<4;++i)c.data.push_back((u>>(8*i))&255);}return c;};
 auto item=[&](const char* name,unsigned socket,unsigned z){auto c=named(4,name);c.data.push_back(socket);c.data.push_back(z);return c;};
 Chunk parts;parts.type=0;
 parts.children={part("ALightBody",0,{{0,0},{0,7}}),part("ALightRotor",7,{{0,0}}),part("AHeavyBody",0,{{0,0},{-34,6},{34,6}}),part("AHeavyRotor",7,{{0,0}})};
 Chunk machines;machines.type=2;
 auto light=named(3,"AircraftLight");auto lb=item("ALightBody",0,1);lb.children={item("ALightRotor",1,2)};light.children={lb};
 auto heavy=named(3,"AircraftHeavy");auto hb=item("AHeavyBody",0,1);hb.children={item("AHeavyRotor",1,2),item("AHeavyRotor",2,2)};heavy.children={hb};machines.children={light,heavy};models.append(parts,machines);
}
}
