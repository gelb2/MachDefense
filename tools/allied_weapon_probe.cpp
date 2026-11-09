// Read-only inventory of original/current allied build assemblies.
#include "Core/Simulation/PlayerParts.hpp"
#include "Core/Simulation/Combat.hpp"
#include "Core/Contracts/Game.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>
namespace fs=std::filesystem;
struct Resources:md::ResourceAccess{
 fs::path root;
 md::Bytes read(std::string_view path)override{std::ifstream f(root/path,std::ios::binary);if(!f)throw std::runtime_error(std::string(path));return {std::istreambuf_iterator<char>(f),{}};}
 md::Vec2 imageSize(std::string_view)override{return {};}
};
int main(int argc,char**argv){
 if(argc!=4)throw std::runtime_error("Usage: allied_weapon_probe Resources original-tables output.tsv");
 Resources r;r.root=argv[1];std::ofstream out(argv[3]);
 out<<"source\tkind\tbuild\tweapon_parts\tdistinct_weapons\tweapons\n";
 for(bool original:{true,false}){
  auto tableRoot=original?fs::path(argv[2]):r.root/"Tables";
  auto load=[&](const char*n){return md::DataTable::load(tableRoot/(std::string(n)+".tsv"));};
  const auto parts=load("MachBuild_Parts"),names=load("MachBuild_Name"),items=load("ItemInfo");
  const auto builds=md::BuildCatalog::load(parts,names);const auto weapons=md::WeaponCatalog::load(load("WeaponInfo"),false);
  const auto models=original?md::BattleModels::parse(md::Chunk::parse(r.read("RawData/PartsInfo.mcb")),md::Chunk::parse(r.read("RawData/MachInfo.mcb"))):md::BattleModels::load(r);
  auto inspect=[&](std::string_view kind,std::string_view build,const std::vector<md::AssemblyPart>& nodes){
   std::map<std::string,unsigned> counts;unsigned count=0;
   auto walk=[&](auto&& self,const auto& ns)->void{for(const auto& p:ns){if(weapons.find(p.name)){++counts[p.name];++count;}self(self,p.children);}};walk(walk,nodes);
   out<<(original?"original":"current")<<'\t'<<kind<<'\t'<<build<<'\t'<<count<<'\t'<<counts.size()<<'\t';bool first=true;
   for(const auto& [name,n]:counts){if(!first)out<<';';first=false;out<<name<<'x'<<n;}out<<'\n';
  };
  for(std::size_t i=0;i<parts.rowCount();++i){auto name=parts.value(i,"MachName");auto d=builds.find(name);if(!d)throw std::runtime_error("Missing parts build");inspect("mech",name,md::PlayerLoadout::fromBuild(*d,items,md::BattleGeometry::pad()).assembly);}
  for(std::size_t i=0;i<names.rowCount();++i){auto name=names.value(i,"MachName");auto d=builds.find(name);if(!d)throw std::runtime_error("Missing named build");auto a=models.machine(d->machine);if(!a)throw std::runtime_error("Missing named assembly");inspect("named-unit",name,*a);}
  auto upgrades=md::BaseUpgradeCatalog(load("BaseUpgrade"));auto save=md::SaveData::empty(1);save.createProfile("Audit",1);
  for(unsigned level=0;level<=upgrades.maximumLevel(md::BaseUpgrade::cannon);++level){save.setBaseUpgrade(0,unsigned(md::BaseUpgrade::cannon),level);inspect("base","cannon-"+std::to_string(level+1),md::PlayerLoadout::forBase(save,0,upgrades,md::BattleGeometry::pad()).assembly);}
 }
}
