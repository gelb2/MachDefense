#pragma once
#include "Core/Contracts/Game.hpp"
#include "Core/Simulation/Economy.hpp"
#include "Core/Simulation/Production.hpp"
#include "Core/Validation/ValidationPanel.hpp"
namespace md {
class World;
enum class ValidationCommand:unsigned {
    profilePrevious=1,profileNext,newProfile,missionPrevious,missionNext,startMission,
    creditMinus,creditPlus,creditStep,cellsMinus,cellsPlus,fillCells,
    baseMinus=100,basePlus=110,
    buildPrevious=200,buildNext,buildLevelMinus,buildLevelPlus,buildGrant,buildInstall,
    attackPrevious=300,attackNext,attackLevelMinus,attackLevelPlus,attackCountMinus,attackCountPlus,attackGrant,attackInstall,
    machinePrevious=400,machineNext,spawnSide,spawnMachine,actorPrevious,actorNext,healthMinus,healthPlus,healthFull,win,lose,
    save=500
};
struct ValidationResult {bool changed=false,start=false,persist=false;};
// Test controls modify the same in-memory save/world used by normal gameplay.
// Session gates this object to the separate validation app and owns file writes.
class ValidationTools {
public:
    explicit ValidationTools(ResourceAccess&);
    ValidationPanelState panel(const SaveData&,const World*,bool open)const;
    ValidationResult apply(unsigned command,SaveData&,World*);
    void setSlot(std::size_t slot){slot_=slot;}
    std::size_t slot()const{return slot_;}
    unsigned mission()const{return mission_;}
    unsigned page=0;
private:
    BuildCatalog builds_;BaseUpgradeCatalog bases_;AttackCatalog attacks_;
    std::vector<std::string> buildNames_,attackNames_,machineNames_;
    std::size_t slot_=0,build_=0,attack_=0,machine_=0,actor_=0;
    unsigned mission_=0,creditStep_=1000;bool playerSpawn_=false;
};
}
