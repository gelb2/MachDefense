#pragma once
#include "Core/Data/DataTable.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>

namespace md {
enum class WeaponType:std::int16_t { gun,shotgun,laser,railgun,emp,bomb,missile,sniper,nuclear,bullet };
struct WeaponDefinition {
    std::string name;
    WeaponType type=WeaponType::gun;
    std::int16_t damage=0,damageRange=0,shotRange=0,shell=0;
    std::uint8_t shotCount=0,uiTile=0;
    float shotDelay=0,reloadDelay=0,speed=0,aimRate=0,special1=0,special2=0,explodeScale=0;
    std::string bulletImage,effectImage,explodeImage,fireSound;
    int bulletTile=0,effectType=0,effectColumns=1,effectRows=1;
    // WeaponInfo::calcAtkPoint 0x1fcac. This is the original UI estimate,
    // not a replacement for per-projectile damage.
    [[nodiscard]] int attackPoint() const;
    static WeaponDefinition fromTable(const DataTable&,std::size_t row,bool phone);
};
class WeaponCatalog {
public:
    static WeaponCatalog load(const DataTable&,bool phone);
    // Reuse an existing complete weapon definition for a newly drawn part.
    // Existing entries are immutable; aliases cannot replace an original name.
    void addAlias(std::string name,std::string_view source);
    [[nodiscard]] const WeaponDefinition* find(std::string_view name) const;
    [[nodiscard]] std::size_t size() const { return definitions_.size(); }
private:
    std::unordered_map<std::string,WeaponDefinition> definitions_;
};
}
