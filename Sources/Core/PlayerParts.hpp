#pragma once
#include "Battle.hpp"
#include "Economy.hpp"
#include "Production.hpp"
namespace md {
// GobWorld::addPlayerMachWithParts (0x175c4), setStage (0x1627c),
// GobHvM_Player::setParts:partsType: (0xfa8c).
struct PlayerLoadout {
    std::vector<AssemblyPart> assembly;
    int hp=0;
    float stepSize=0,radius=0;
    std::string weapon;
    static PlayerLoadout fromBuild(const BuildDefinition&,const DataTable& items,BattleGeometry);
    static PlayerLoadout forBase(const SaveData&,std::size_t slot,const BaseUpgradeCatalog&,BattleGeometry);
};
}
