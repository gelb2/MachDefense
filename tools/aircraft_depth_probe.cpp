#include "../Tests/PNGTestSize.hpp"
#include "Core/Simulation/AircraftModels.hpp"
#include "Core/Simulation/PlayerParts.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

static std::vector<std::uint8_t> read(const std::filesystem::path& p){
 std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error(p.string());
 return {std::istreambuf_iterator<char>(f),{}};
}
int main(int argc,char**argv){
 if(argc!=2)return 2;std::filesystem::path root=argv[1];
 const auto parts=md::Chunk::parse(read(root/"RawData/PartsInfo.mcb"));
 const auto machines=md::Chunk::parse(read(root/"RawData/MachInfo.mcb"));
 auto models=md::BattleModels::parse(parts,machines);
 models.configureTiles(md::DataTable::load(root/"Tables/PartsList.tsv"),md::DataTable::load(root/"Tables/TileList.tsv"));
 auto size=[&](std::string_view n){auto path=root/"Images"/n;if(!std::filesystem::exists(path))return md::Vec2{};auto b=read(path);if(b.size()<24)throw std::runtime_error("PNG header");
  return mdtest::pngSize(b);};
 std::cout<<"original_part_definitions="<<models.partCount()<<" original_named_assemblies="<<models.machineCount()<<"\n";
 std::size_t min=100000,max=0;std::string largest;
 for(const auto& m:machines.children)if(m.type==3){auto name=m.text(0,24);auto n=models.pose(name,1,0,0,0).size();min=std::min(min,n);if(n>max){max=n;largest=name;}}
 std::cout<<"original_named_assembly_nodes_min="<<min<<" max="<<max<<" largest="<<largest<<"\n";
 models.append(md::Chunk::parse(read(root/"RawData/AircraftParts.mcb")),md::Chunk::parse(read(root/"RawData/AircraftMachines.mcb")));
 md::appendAircraftModels(models);
 auto dump=[&](std::string_view name,const std::vector<md::AssemblyPart>& tree){
  auto poses=models.pose(tree,1,0,0,0);md::Frame f;models.appendSprites(f,poses,{},size);
  std::cout<<"\n"<<name<<" nodes="<<poses.size()<<" visible_parts="<<f.sprites.size()/2<<" shadow_sprites="<<f.sprites.size()/2<<"\n";
  for(auto& p:poses)std::cout<<p.definition->name<<"\t"<<p.definition->image<<"\tz="<<unsigned(p.zOrder)<<"\tparent="<<p.hasParent<<"\tvisible="<<(size(p.definition->image).x>0)<<"\n";
 };
 for(const char* n:{"AircraftLight","AircraftDuct","AircraftTransport","AircraftHeavy","E_Tank_01","E_Tank_07","E_Tank_32"})dump(n,*models.machine(n));
 auto builds=md::BuildCatalog::load(md::DataTable::load(root/"Tables/MachBuild_Parts.tsv"),md::DataTable::load(root/"Tables/MachBuild_Name.tsv"));
 auto items=md::DataTable::load(root/"Tables/ItemInfo.tsv");
 for(const char* n:{"MachineGun_01","MachineGun_10","HMG_10"})dump(n,md::PlayerLoadout::fromBuild(*builds.find(n),items,md::BattleGeometry::pad()).assembly);
}
