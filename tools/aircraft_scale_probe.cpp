#include "Core/Simulation/Battle.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

std::vector<std::uint8_t> read(const std::filesystem::path& p) {
    std::ifstream f(p,std::ios::binary); if(!f) throw std::runtime_error("Missing test resource: "+p.string());
    return {std::istreambuf_iterator<char>(f),{}};
}
int main(int argc,char** argv) {
    assert(argc==2); std::filesystem::path root=argv[1];
    auto partsBytes=read(root/"PartsInfo.mcb"),machBytes=read(root/"MachInfo.mcb"),mapBytes=read(root/"MapInfo.mcb");
    auto parts=md::Chunk::parse(partsBytes),machines=md::Chunk::parse(machBytes),maps=md::Chunk::parse(mapBytes);
    assert(parts.children.size()==463 && machines.children.size()==126 && maps.children.size()==80);
    assert(parts.consumedSize==partsBytes.size() && parts.declaredSize!=parts.consumedSize);
    auto models=md::BattleModels::parse(parts,machines);
    assert(models.partCount()==463 && models.machineCount()==126);
    auto resourceRoot=root.parent_path();
    models.configureTiles(md::DataTable::load(resourceRoot/"Tables/PartsList.tsv"),md::DataTable::load(resourceRoot/"Tables/TileList.tsv"));
    auto imageSize=[&](std::string_view name) {
        if(!std::filesystem::exists(resourceRoot/"Images"/name)) return md::Vec2{};
        auto bytes=read(resourceRoot/"Images"/name);
        assert(bytes.size()>=24);
        auto be=[&](unsigned offset) { return std::uint32_t(bytes[offset])<<24 | std::uint32_t(bytes[offset+1])<<16 | std::uint32_t(bytes[offset+2])<<8 | bytes[offset+3]; };
        return md::Vec2{float(be(16)),float(be(20))};
    };
    std::cout<<"["; bool first=true;
    for(const char* name:{"E_Tank_01","E_Tank_07","E_Tank_32"}) {
      md::Frame f;models.appendSprites(f,name,{},1,0,0,0,imageSize);
      if(!first)std::cout<<",";first=false;
      std::cout<<"{\"machine\":\""<<name<<"\",\"sprites\":[";bool fs=true;
      for(const auto& p:f.sprites){if(!fs)std::cout<<",";fs=false;auto r=p.destination;auto q=*p.sourcePx;
       std::cout<<"{\"image\":\""<<p.image<<"\",\"destination\":["<<r.x<<","<<r.y<<","<<r.width<<","<<r.height<<"],\"source\":["<<q.x<<","<<q.y<<","<<q.width<<","<<q.height<<"],\"rotation\":"<<p.rotationDegrees<<",\"shadow\":"<<(p.color.r==0?"true":"false")<<"}";
      }std::cout<<"]}";
    }std::cout<<"]\n";
}
