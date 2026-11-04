#include "Economy.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
int main(int argc,char** argv){
    assert(argc==2);md::BaseUpgradeCatalog upgrades(md::DataTable::load(std::filesystem::path(argv[1])/"BaseUpgrade.tsv"));
    auto save=md::SaveData::empty(1);save.createProfile("1234",1);
    assert(!upgrades.purchase(save,0,md::BaseUpgrade::cannon));
    save.setCredits(0,1000000);
    // Same six purchases as the port's 2026-11-04 1,000,000 Cr fixture.
    int sum=0;
    for(unsigned i=0;i<6;++i){auto purchase=upgrades.purchase(save,0,md::BaseUpgrade(i));assert(purchase&&purchase->newLevel==1);sum+=purchase->cost;}
    assert(sum==3900&&save.credits(0)==996100);
    const auto reread=md::SaveData::decode(save.encode());
    for(unsigned i=0;i<6;++i)assert(reread.baseUpgrade(0,i)==1);
    assert(upgrades.maximumLevel(md::BaseUpgrade::cannon)==6&&upgrades.maximumLevel(md::BaseUpgrade::buildTime)==9);
    assert(*upgrades.level(md::BaseUpgrade::defense,1).parameter==15000);
    assert(*upgrades.level(md::BaseUpgrade::cellStorage,1).parameter==700);
    save.setBaseUpgrade(0,0,6);const auto before=save.encode();
    assert(!upgrades.purchase(save,0,md::BaseUpgrade::cannon)&&save.encode()==before);
    md::AttackCatalog attacks(md::DataTable::load(std::filesystem::path(argv[1])/"SpAttackList.tsv"));
    assert(attacks.maximumLevel("AirStrike-Bomb")==5&&attacks.maximumLevel("Crossfire-Missile")==8);
    assert(attacks.upgradeCost(save.attacks(0)[0])==600); // 500 + 2 existing bombs * (200-150).
    save.setCredits(0,599);const auto poor=save.encode();assert(!attacks.purchase(save,0,0,true)&&save.encode()==poor);
    save.setCredits(0,800);assert(attacks.purchase(save,0,0,true));assert(save.credits(0)==200&&save.attacks(0)[0].level==1&&save.attacks(0)[0].count==2);
    assert(attacks.purchase(save,0,0,false));assert(save.credits(0)==0&&save.attacks(0)[0].count==3);
    assert(md::SaveData::decode(save.encode()).attacks(0)[0].level==1);
    std::cout<<"Original six base upgrades: 3900 Cr, save/reload, insufficient funds and maximum level passed\n";
}
