#include "Production.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
int main(int argc,char** argv){
    assert(argc==2);std::filesystem::path p=argv[1];
    const auto parts=md::DataTable::load(p/"MachBuild_Parts.tsv"),names=md::DataTable::load(p/"MachBuild_Name.tsv");
    auto catalog=md::BuildCatalog::load(parts,names);assert(catalog.size()==parts.rowCount()+names.rowCount());
    auto gun=catalog.find("MachineGun_01"),missile=catalog.find("Missile_01"),turret=catalog.find("RifleTurret_01");assert(gun&&missile&&turret);
    assert(gun->cost==60&&gun->duration==3&&gun->maximumConcurrent==5&&gun->armor==11001&&gun->weapon==10041);
    md::ProductionQueue queue;float cells=59;
    assert(!queue.enqueue(*gun,0,1,cells)&&cells==59);cells=10000;
    for(int i=0;i<5;++i)assert(queue.enqueue(*gun,0,1,cells));
    assert(!queue.enqueue(*gun,0,1,cells)&&cells==9700);
    assert(queue.enqueue(*missile,0,1,cells)&&queue.enqueue(*missile,0,1,cells));
    const float remaining=cells;assert(!queue.enqueue(*turret,0,1,cells,md::Vec2{10,20})&&cells==remaining);
    md::DarwinRandom rng(1);assert(queue.tick(3,md::BattleGeometry::phone(),rng).empty());
    auto built=queue.tick(3.000001,md::BattleGeometry::phone(),rng);assert(built.size()==5);
    for(const auto& unit:built)assert(unit.position.x>=-110&&unit.position.x<=110&&unit.position.y==-512);
    queue.clear();assert(!queue.enqueue(*turret,4,1,cells));assert(queue.enqueue(*turret,4,1,cells,md::Vec2{10,20}));
    built=queue.tick(17,md::BattleGeometry::phone(),rng);assert(built.size()==1&&built[0].position.x==0&&built[0].destination->x==10);
    queue.clear();cells=1000;
    assert(queue.enqueue(*gun,0,1,cells)&&queue.enqueue(*gun,0,1,cells));
    md::DarwinRandom interleaved(1),expected(1);unsigned count=0;
    queue.tick(4,md::BattleGeometry::phone(),interleaved,[&](const md::ProducedUnit& unit){
        assert(unit.position.x==(float(expected.next())*0x1p-31f*440.f-220.f)*.5f);
        assert(interleaved.next()==expected.next());++count;
    });assert(count==2);
    std::cout<<catalog.size()<<" build definitions: price, seven-slot queue, per-unit limits, completion and spawn positions passed\n";
}
