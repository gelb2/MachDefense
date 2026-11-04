#pragma once
#include "Production.hpp"
#include "SaveData.hpp"
namespace md {
struct StageReward {
    unsigned mission=0,nextMission=0;
    int credits=0;
    std::string build,attack;
    unsigned attackCount=0;
};
// DlgSummary::initWithClear: 0x349c8. Applies only the original local
// reward. GameSession writes the save once when the summary is opened.
class Settlement {
public:
    Settlement(const BuildCatalog&,DataTable regeneration);
    StageReward clear(SaveData&,std::size_t slot,DarwinRandom&)const;
    const BuildDefinition* definition(const OwnedBuild&)const;
    bool upgradeBuild(SaveData&,std::size_t slot,std::string_view name)const;
private:
    const BuildCatalog& catalog_;DataTable regeneration_;
    std::string addBuild(SaveData&,std::size_t,DarwinRandom&)const;
};
}
