#include "Core/Session/Settlement.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
int main(int argc,char** argv){
    assert(argc==2);const std::filesystem::path p=argv[1];
    const auto catalog=md::BuildCatalog::load(md::DataTable::load(p/"MachBuild_Parts.tsv"),md::DataTable::load(p/"MachBuild_Name.tsv"));
    const auto table=md::DataTable::load(p/"MachRegen.tsv");
    md::Settlement settlement(catalog,table);
    auto save=md::SaveData::empty(1);save.createProfile("PLAYER",100);
    constexpr auto at=md::SaveData::slotStart;
    md::DarwinRandom rng(1);
    auto reward=settlement.clear(save,0,rng);
    // Darwin seed 1: 1804289383 % 40 = 23, 846930886 % 5 = 1,
    // 1681692777 % 4 = 1. The first clear awards two missiles.
    assert(reward.mission==0&&reward.nextMission==1&&reward.credits==500);
    assert(reward.build.empty()&&reward.attack=="AirStrike-Missile"&&reward.attackCount==2);
    assert(save.credits(0)==500&&save.readWord(at+0x2054)==1000&&save.clearCount(0,0)==1&&save.stage(0)==1);
    assert(save.attacks(0)[1].count==4);
    // Replaying a cleared mission consumes no unit-award roll and adds no score.
    save.writeWord(at+0x34,0);md::DarwinRandom repeat(1);
    reward=settlement.clear(save,0,repeat);
    assert(reward.attack=="AirStrike-EMP"&&reward.attackCount==3&&reward.build.empty());
    assert(save.credits(0)==1000&&save.readWord(at+0x2054)==1000&&save.clearCount(0,0)==2&&save.stage(0)==1);
    auto restored=md::SaveData::decode(save.encode());
    assert(restored.attacks(0).back().name=="AirStrike-EMP"&&restored.attacks(0).back().count==3);
    assert(restored.builds(0).size()==4&&restored.builds(0)[0].name=="MachineGun");
    assert(!settlement.upgradeBuild(save,0,"unknown"));
    const auto cost=settlement.definition(save.builds(0)[0])->upgradeCost;
    save.setCredits(0,cost-1);auto before=save.encode();assert(!settlement.upgradeBuild(save,0,"MachineGun")&&save.encode()==before);
    save.setCredits(0,cost);assert(settlement.upgradeBuild(save,0,"MachineGun")&&save.credits(0)==0&&save.builds(0)[0].level==1);
    // All 80 missions use the original bonus formula and cap at mission 79.
    auto campaign=md::SaveData::empty(4);campaign.createProfile("CAMPAIGN",10);md::DarwinRandom campaignRng(5);
    for(unsigned mission=0;mission<80;++mission){
        assert(campaign.readWord(at+0x34)==mission);
        auto earned=settlement.clear(campaign,0,campaignRng);
        assert(earned.credits==500+int(mission)*25&&campaign.clearCount(0,mission)==1);
        const auto units=campaign.builds(0);
        for(std::size_t i=0;i<units.size();++i){
            const auto* d=settlement.definition(units[i]);assert(d);
            if(i){const auto* prev=settlement.definition(units[i-1]);assert(prev->placementType<d->placementType||(prev->placementType==d->placementType&&prev->cost<=d->cost));}
        }
    }
    assert(campaign.stage(0)==79&&campaign.readWord(at+0x34)==79&&campaign.readWord(at+0x2054)==3240000);
    assert(campaign.credits(0)==119000);
    // Inventories retain levels, visibility, quantities and neighboring slots.
    auto b=campaign.builds(0);b[0].onSlot=false;b[0].level=3;campaign.setBuilds(0,b);
    auto a=campaign.attacks(0);a[0].onSlot=false;a[0].level=2;a[0].count=12345;campaign.setAttacks(0,a);
    auto decoded=md::SaveData::decode(campaign.encode());
    assert(!decoded.builds(0)[0].onSlot&&decoded.builds(0)[0].level==3);
    assert(!decoded.attacks(0)[0].onSlot&&decoded.attacks(0)[0].level==2&&decoded.attacks(0)[0].count==12345);
    assert(!decoded.active(1));
    std::cout<<"Original clear/replay rewards, 80-stage progression, inventories and upgrade costs passed\n";
}
