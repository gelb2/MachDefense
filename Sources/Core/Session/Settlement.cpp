#include "Core/Session/Settlement.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdio>
#include <stdexcept>
namespace md {
namespace {
std::string levelName(std::string_view name,unsigned level){
    char suffix[24];std::snprintf(suffix,sizeof suffix,"_%02u",level+1);
    return std::string(name)+suffix;
}
}
Settlement::Settlement(const BuildCatalog& catalog,DataTable regeneration)
    :catalog_(catalog),regeneration_(std::move(regeneration)){}
const BuildDefinition* Settlement::definition(const OwnedBuild& b)const{return catalog_.find(levelName(b.name,b.level));}
std::string Settlement::addBuild(SaveData& save,std::size_t slot,DarwinRandom& rng)const{
    auto owned=save.builds(slot);
    struct Candidate {std::string name;unsigned cumulative;};
    std::vector<Candidate> choices;unsigned total=0;
    for(std::size_t row=0;row<regeneration_.rowCount();++row){
        auto name=std::string(regeneration_.value(row,"MachName"));
        if(std::any_of(owned.begin(),owned.end(),[&](const auto& b){return b.name==name;}))continue;
        auto weight=regeneration_.integer(row,"Rate");
        if(!weight||*weight<0||!catalog_.find(levelName(name,0)))throw std::runtime_error("Invalid unit reward table");
        total+=unsigned(*weight);choices.push_back({std::move(name),total});
    }
    if(choices.empty())return {};
    if(total==0)throw std::runtime_error("Empty unit reward distribution");
    const auto pick=rng.next()%total;
    const auto selected=std::find_if(choices.begin(),choices.end(),[&](const auto& c){return pick<c.cumulative;});
    OwnedBuild added{selected->name,0,std::count_if(owned.begin(),owned.end(),[](const auto& b){return b.onSlot;})<7};
    const auto* d=definition(added);
    auto insertion=owned.end();
    for(auto i=owned.begin();i!=owned.end();++i){
        const auto* existing=definition(*i);if(!existing)throw std::runtime_error("Unknown owned unit");
        if(existing->placementType>d->placementType||(existing->placementType==d->placementType&&existing->cost>d->cost)){insertion=i;break;}
    }
    owned.insert(insertion,added);save.setBuilds(slot,owned);return added.name;
}
StageReward Settlement::clear(SaveData& save,std::size_t slot,DarwinRandom& rng)const{
    if(!save.active(slot))throw std::invalid_argument("Empty profile");
    const auto at=SaveData::slotStart+slot*SaveData::slotStride;
    StageReward result;result.mission=save.readWord(at+0x34);
    if(result.mission>=80)throw std::invalid_argument("Mission outside original campaign");
    // Stage-summary construction runs once; callers must not repeat this per frame.
    result.credits=500+int(result.mission)*25;
    save.writeWord(at+0x3c,save.readWord(at+0x3c)+unsigned(result.credits));
    const auto cleared=save.clearCount(slot,result.mission);
    if(cleared<1)save.writeWord(at+0x2054,save.readWord(at+0x2054)+1000+result.mission*1000);
    if(cleared<=0&&rng.next()%40<=9)result.build=addBuild(save,slot,rng);
    if(result.build.empty()){
        static constexpr std::array<std::string_view,5> names{"AirStrike-Bomb","AirStrike-Missile","AirStrike-Nuclear","AirStrike-EMP","Crossfire-Missile"};
        result.attack=names[rng.next()%names.size()];result.attackCount=rng.next()%4+1;
        auto attacks=save.attacks(slot);
        const bool onSlot=std::count_if(attacks.begin(),attacks.end(),[](const auto& a){return a.onSlot;})<7;
        auto i=std::find_if(attacks.begin(),attacks.end(),[&](const auto& a){return a.name==result.attack;});
        if(i==attacks.end()){attacks.push_back({result.attack,0,0,false});i=attacks.end()-1;}
        i->count+=result.attackCount;if(onSlot)i->onSlot=true;
        save.setAttacks(slot,attacks);
    }
    save.setClearCount(slot,result.mission,unsigned(cleared+1));
    result.nextMission=std::min(79u,result.mission+1);
    save.writeWord(at+0x34,result.nextMission);
    save.writeWord(at+0x38,std::max(save.stage(slot),result.nextMission));
    return result;
}
bool Settlement::upgradeBuild(SaveData& save,std::size_t slot,std::string_view name)const{
    auto owned=save.builds(slot);
    auto i=std::find_if(owned.begin(),owned.end(),[&](const auto& b){return b.name==name;});
    if(i==owned.end())return false;
    const auto* current=definition(*i);
    if(!current||current->upgradeCost<=0||save.credits(slot)<current->upgradeCost||!catalog_.find(levelName(name,i->level+1)))return false;
    save.setCredits(slot,save.credits(slot)-current->upgradeCost);++i->level;
    save.setBuilds(slot,owned);return true;
}
}
