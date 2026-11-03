#include "Loot.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <unordered_map>
using namespace md;
int main(int argc,char**argv) {
    assert(argc==2);auto table=DataTable::load(std::filesystem::path(argv[1])/"ItemInfo.tsv");
    std::unordered_map<std::string,LootDefinition> catalog;
    for(std::size_t i=0;i<table.rowCount();++i) {auto d=LootDefinition::fromTable(table,i);catalog.emplace(d.code,std::move(d));}
    assert(catalog.at("00001").type==LootType::coin&&catalog.at("00101").type==LootType::mineral);
    auto lookup=[&](std::string_view code)->const LootDefinition* {auto i=catalog.find(std::string(code));return i==catalog.end()?nullptr:&i->second;};
    DarwinRandom random(2),expected(2);auto item=LootState::create(catalog.at("00101"),75,{0,0},10,.5f,random);
    for(int i=0;i<3;++i)expected.next();assert(random.next()==expected.next());
    assert(item.checkLength==50&&item.maximumCheckLength==200&&item.checkIncrement==5&&item.createTime==10);
    item.velocity={};std::vector<LootCollector> collectors={{9,{5,5},1,100},{10,{40,40},0,0},{11,{40,40},0,100},{12,{1,1},0,100}};
    assert(!item.tick(10,collectors,random));assert(item.picker==11); // first eligible, not closest
    assert(item.checkLength==55&&item.checkTime==10);
    item.pickerDead(11);assert(!item.picker);item.velocity={};collectors[2].position={1,1};
    item.tick(10.5,collectors,random);assert(!item.picker); // strict half-second
    item.tick(10.6,collectors,random);assert(item.picker==11);
    const auto pickup=item.tick(10.7,collectors,random);assert(pickup&&item.removed&&pickup->value==75&&pickup->label=="75cell");
    assert(pickup->applyCells(90,100)==100&&pickup->sound==3);
    auto coin=LootState::create(catalog.at("00001"),23,{0,0},0,1,random);coin.velocity={};
    collectors={ {1,{20,20},0,100} };coin.tick(.6,collectors,random);assert(coin.picker==1);
    assert(!coin.tick(.7,collectors,random)); // equality20 does not collect
    auto credited=coin.tick(.8,collectors,random);assert(credited&&credited->type==LootType::coin&&credited->label=="23cr");
    auto stale=LootState::create(catalog.at("00001"),1,{0,0},0,1,random);stale.velocity={};
    stale.tick(180,{},random);assert(!stale.removed);stale.tick(180.6,{},random);assert(stale.removed);
    auto drift=LootState::create(catalog.at("00001"),1,{0,0},0,.5f,random);drift.velocity={7,.01f};
    drift.tick(0,{},random);assert(drift.position.x==6&&drift.position.y==0);
    Frame frame;drift.appendSprites(frame,{100,100},[](auto){return Vec2{128,128};});
    assert(frame.sprites.size()==2&&frame.sprites[0].image=="Efx_Item2.png"&&frame.sprites[1].image=="Icon_32.png");
    assert(frame.sprites[1].sourcePx->x==0&&frame.sprites[1].sourcePx->y==64&&frame.sprites[1].destination.width==16);
    for(unsigned seed=1;seed<30;++seed) {
        DarwinRandom dropRandom(seed);auto drops=enemyLootDrops(600,100,1.5f,{},0,.5f,dropRandom,lookup);
        assert(!drops.empty()&&drops.size()<=2&&drops[0].definition.code=="00103");
        assert(drops[0].value>=600&&drops[0].value<=800);
        if(drops.size()==2)assert(drops[1].definition.type==LootType::coin&&drops[1].value>=900&&drops[1].value<=1200);
    }
    assert(enemyLootDrops(600,0,1,{},0,.5f,random,lookup).empty());
    std::cout<<"Loot original spawn, collection, decay, expiry and image checks passed\n";
}
