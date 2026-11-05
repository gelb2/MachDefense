#pragma once
#include "Core/Contracts/DarwinRandom.hpp"
#include "Core/Data/DataTable.hpp"
#include "Core/Contracts/RenderTypes.hpp"
#include <functional>
#include <optional>
#include <span>
#include <string>

namespace md {
enum class LootType { coin,mineral,other };
struct LootDefinition {
    std::string code,name; int icon=0; LootType type=LootType::other;
    static LootDefinition fromTable(const DataTable&,std::size_t row);
};
struct LootCollector { std::uint64_t id=0;Vec2 position;int machineType=0,hp=0; };
struct LootPickup {
    std::uint64_t collector=0;LootType type=LootType::other;int value=0;std::string label;int sound=3;
    [[nodiscard]] float applyCells(float cells,float capacity) const;
};
// GobFieldItem 0xbdc8 / 0xc270 / 0xc938. Item movement uses original world units.
struct LootState {
    LootDefinition definition;
    Vec2 position,velocity;
    float iconAngle=0,glowAngle=0,glowScale=.5f,glowAlpha=1,speed=0;
    float checkLength=0,maximumCheckLength=0,checkIncrement=0,createTime=0,checkTime=0;
    int value=0;bool removed=false;
    std::optional<std::uint64_t> picker;
    static LootState create(LootDefinition,int value,Vec2 position,double time,float deviceScale,DarwinRandom&);
    std::optional<LootPickup> tick(double time,std::span<const LootCollector>,DarwinRandom&);
    void pickerDead(std::uint64_t id) { if(picker==id) picker.reset(); }
    void appendSprites(Frame&,Vec2 screenPosition,const std::function<Vec2(std::string_view)>& imageSize,
                       float retinaScale=.5f) const;
};
// Call after original superclass crash effects and before killEnemy/crashTime.
// Each item's constructor consumes its three RNG values before the next drop.
std::vector<LootState> enemyLootDrops(int coin,int coinRate,float creditResearchFactor,Vec2 position,
    double time,float deviceScale,DarwinRandom&,
    const std::function<const LootDefinition*(std::string_view)>& lookup);
}
