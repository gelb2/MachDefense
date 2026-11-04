#include "PlayerParts.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
std::vector<std::uint8_t> read(const std::filesystem::path&p){std::ifstream f(p,std::ios::binary);assert(f);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char**argv){
    assert(argc==2);std::filesystem::path root=argv[1];
    const auto parts=md::DataTable::load(root/"Tables/MachBuild_Parts.tsv"),names=md::DataTable::load(root/"Tables/MachBuild_Name.tsv"),items=md::DataTable::load(root/"Tables/ItemInfo.tsv");
    const auto builds=md::BuildCatalog::load(parts,names);
    auto models=md::BattleModels::parse(md::Chunk::parse(read(root/"RawData/PartsInfo.mcb")),md::Chunk::parse(read(root/"RawData/MachInfo.mcb")));
    std::function<void(const md::AssemblyPart&)> verify=[&](const auto& node){const auto* p=models.part(node.name);assert(p);for(const auto& child:node.children){assert(child.parentSocket<p->sockets.size());verify(child);}};
    for(std::size_t i=0;i<parts.rowCount();++i){auto loadout=md::PlayerLoadout::fromBuild(*builds.find(parts.value(i,"MachName")),items,md::BattleGeometry::phone());assert(loadout.hp>0&&loadout.stepSize>0);verify(loadout.assembly[0]);}
    auto save=md::SaveData::empty(1);save.createProfile("TEST",1);
    md::BaseUpgradeCatalog upgrades(md::DataTable::load(root/"Tables/BaseUpgrade.tsv"));
    for(unsigned i=0;i<7;++i){save.setBaseUpgrade(0,0,i);save.setBaseUpgrade(0,1,i);auto loadout=md::PlayerLoadout::forBase(save,0,upgrades,md::BattleGeometry::phone());verify(loadout.assembly[0]);assert(loadout.hp==int(*upgrades.level(md::BaseUpgrade::defense,i).parameter));}
    std::cout<<parts.rowCount()<<" original player equipment assemblies and seven base upgrade assemblies passed\n";
}
