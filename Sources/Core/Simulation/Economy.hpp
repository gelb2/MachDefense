#pragma once
#include "Core/Data/DataTable.hpp"
#include "Core/Session/SaveData.hpp"
#include <array>
#include <optional>
#include <vector>

namespace md {
enum class BaseUpgrade:unsigned { cannon,defense,buildTime,creditResearch,cellResearch,cellStorage };
struct UpgradeLevel { int cost=0;std::optional<float> parameter; };
struct UpgradePurchase { int cost=0;unsigned newLevel=0;bool refreshBaseHP=false,replaceBaseWeapon=false; };
class BaseUpgradeCatalog {
public:
    explicit BaseUpgradeCatalog(const DataTable&);
    [[nodiscard]] const UpgradeLevel& level(BaseUpgrade,unsigned level) const;
    [[nodiscard]] unsigned maximumLevel(BaseUpgrade) const;
    // DlgUpgradeBase::setUpgrade 0x344cc mutates current game/slot memory.
    // It does not write a file. Game save points persist the resulting slot.
    std::optional<UpgradePurchase> purchase(SaveData&,std::size_t slot,BaseUpgrade) const;
private:
    std::array<std::vector<UpgradeLevel>,6> levels_;
};
struct AttackDefinition {std::string name;int cost=0,upgradeCost=0,itemId=0;};
class AttackCatalog {
public:
    explicit AttackCatalog(const DataTable&);
    const AttackDefinition& definition(const OwnedAttack&)const;
    unsigned maximumLevel(std::string_view name)const;
    int upgradeCost(const OwnedAttack&)const;
    bool purchase(SaveData&,std::size_t slot,std::size_t index,bool upgrade)const;
private:std::vector<AttackDefinition> definitions_;
};
}
