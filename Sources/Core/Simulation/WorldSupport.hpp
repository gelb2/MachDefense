#pragma once
#include "Core/Simulation/World.hpp"
#include <stdexcept>
namespace md::world_detail {
inline DataTable table(ResourceAccess& resources,std::string_view name){auto b=resources.read("Tables/"+std::string(name)+".tsv");return DataTable::parse(std::string_view(reinterpret_cast<const char*>(b.data()),b.size()));}
inline Chunk chunk(ResourceAccess& resources,std::string_view name){return Chunk::parse(resources.read("RawData/"+std::string(name)+".mcb"));}
inline float distanceSquared(Vec2 a,Vec2 b){float x=a.x-b.x,y=a.y-b.y;return x*x+y*y;}
inline AssemblyPart& partAt(std::vector<AssemblyPart>& tree,const std::vector<std::size_t>& path){auto* node=&tree.at(path.at(0));for(std::size_t i=1;i<path.size();++i)node=&node->children.at(path[i]);return *node;}
inline float upgradeParameter(const BaseUpgradeCatalog& catalog,const SaveData& save,std::size_t slot,BaseUpgrade type){auto p=catalog.level(type,save.baseUpgrade(slot,unsigned(type))).parameter;if(!p)throw std::runtime_error("Missing original base upgrade parameter");return *p;}
}
