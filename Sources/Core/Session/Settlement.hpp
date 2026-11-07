#pragma once
#include "Core/Simulation/Production.hpp"
#include "Core/Session/SaveData.hpp"
namespace md {
struct StageReward {
    unsigned mission=0,nextMission=0;
    int credits=0;
    std::string build,attack;
    unsigned attackCount=0;
};
// DlgSummary::initWithClear: 0x349c8 supplies the base reward flow.
// The restored game also unlocks LaserTurret after mission 30 + Vulcan.
// GameSession saves once when the summary is opened.
class Settlement {
public:
    Settlement(const BuildCatalog&,DataTable regeneration);
    // Campaign completion count is one-based; inventory/serialization stay unchanged.
    static constexpr unsigned laserUnlockMission=30;
    bool grantCampaignUnlock(SaveData&,std::size_t slot,unsigned completedMissions)const;
    std::vector<std::string> grantAircraftUnlocks(SaveData&,std::size_t slot,unsigned completedMissions)const;
    StageReward clear(SaveData&,std::size_t slot,DarwinRandom&)const;
    const BuildDefinition* definition(const OwnedBuild&)const;
    bool upgradeBuild(SaveData&,std::size_t slot,std::string_view name)const;
private:
    const BuildCatalog& catalog_;DataTable regeneration_;
    bool awardBuild(SaveData&,std::size_t,std::string_view name)const;
    std::string addBuild(SaveData&,std::size_t,DarwinRandom&)const;
};
}
