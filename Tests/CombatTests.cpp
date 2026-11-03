#include "Combat.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
int main(int argc,char** argv){
    assert(argc==2);auto table=md::DataTable::load(std::filesystem::path(argv[1])/"WeaponInfo.tsv");
    const auto phone=md::WeaponCatalog::load(table,true),pad=md::WeaponCatalog::load(table,false);
    assert(phone.size()==table.rowCount()&&pad.size()==phone.size());
    auto p=phone.find("BaseWpn01"),q=pad.find("BaseWpn01");assert(p&&q);
    assert(p->damage==10&&p->shotCount==10&&p->shotRange==225&&q->shotRange==450);
    assert(p->speed==.5f&&q->speed==1&&p->attackPoint()==24);
    assert(p->fireSound=="MachineGun04.wav");
    for(std::size_t i=0;i<table.rowCount();++i){
        const auto* a=phone.find(table.value(i,"Parts"));const auto* b=pad.find(table.value(i,"Parts"));
        assert(a&&b&&a->damage==b->damage&&a->shotCount==b->shotCount&&a->shotDelay==b->shotDelay);
        assert(a->shotRange==int(float(b->shotRange)*.5f)&&a->speed==b->speed*.5f);
        assert(a->attackPoint()>=0&&a->attackPoint()==b->attackPoint());
    }
    std::cout<<phone.size()<<" original weapon definitions: phone/pad ranges, speed, damage, bursts and UI attack estimate passed\n";
}
