#include "Core/Simulation/Economy.hpp"
#include <cstdio>
#include <algorithm>
#include <stdexcept>
namespace md {
BaseUpgradeCatalog::BaseUpgradeCatalog(const DataTable& t){
    constexpr std::array names{"BaseCannon","BaseDefense","BuildTime","CrResearch","CellResearch","CellStorage"};
    for(std::size_t category=0;category<names.size();++category){
        for(unsigned i=1;i<=100;++i){
            char key[64];std::snprintf(key,sizeof key,"%s_%02u",names[category],i);
            const auto row=t.findRow(key);if(!row)break;
            auto cost=t.integer(*row,"UpgCost");if(!cost||*cost<0)throw std::runtime_error("Invalid base upgrade cost");
            levels_[category].push_back({int(*cost),t.number(*row,"Param")});
        }
        if(levels_[category].empty())throw std::runtime_error("Missing base upgrade category");
    }
}
const UpgradeLevel& BaseUpgradeCatalog::level(BaseUpgrade category,unsigned index) const {return levels_.at(unsigned(category)).at(index);}
unsigned BaseUpgradeCatalog::maximumLevel(BaseUpgrade category)const {return unsigned(levels_.at(unsigned(category)).size()-1);}
std::optional<UpgradePurchase> BaseUpgradeCatalog::purchase(SaveData& save,std::size_t slot,BaseUpgrade category)const {
    const auto current=save.baseUpgrade(slot,unsigned(category));
    if(!save.active(slot)||current>=maximumLevel(category))return {};
    const auto cost=level(category,current).cost;
    if(save.credits(slot)<cost)return {};
    save.setBaseUpgrade(slot,unsigned(category),current+1);
    save.setCredits(slot,save.credits(slot)-cost);
    return UpgradePurchase{cost,current+1,category==BaseUpgrade::defense,category==BaseUpgrade::cannon};
}
AttackCatalog::AttackCatalog(const DataTable& t){
    for(std::size_t row=0;row<t.rowCount();++row){
        const auto cost=t.integer(row,"Cost"),upgrade=t.integer(row,"UpgCost"),item=t.integer(row,"ItemId");
        if(!cost||!upgrade||!item||*cost<0||*upgrade<0)throw std::runtime_error("Invalid special attack table");
        definitions_.push_back({std::string(t.value(row,"AttackName")),int(*cost),int(*upgrade),int(*item)});
    }
}
const AttackDefinition& AttackCatalog::definition(const OwnedAttack& a)const{
    char suffix[24];std::snprintf(suffix,sizeof suffix,"_%02u",a.level+1);const auto name=a.name+suffix;
    auto i=std::find_if(definitions_.begin(),definitions_.end(),[&](const auto& d){return d.name==name;});
    if(i==definitions_.end())throw std::out_of_range("Unknown special attack level");return *i;
}
unsigned AttackCatalog::maximumLevel(std::string_view name)const{
    unsigned result=0;for(const auto& d:definitions_)if(d.name.starts_with(std::string(name)+"_"))++result;return result;
}
std::int64_t AttackCatalog::upgradeCost(const OwnedAttack& a)const{
    if(a.level+1>=maximumLevel(a.name))return 0;
    const auto& current=definition(a);auto next=a;++next.level;
    // DlgUpgradeItem::initWithAttackSet 0x39f64 charges the increase in
    // value of all existing charges in addition to the level upgrade.
    return std::int64_t(current.upgradeCost)+std::int64_t(a.count)*(std::int64_t(definition(next).cost)-current.cost);
}
bool AttackCatalog::purchase(SaveData& save,std::size_t slot,std::size_t index,bool upgrade)const{
    auto owned=save.attacks(slot);if(index>=owned.size())return false;auto& a=owned[index];
    const auto cost=upgrade?upgradeCost(a):std::int64_t(definition(a).cost);
    if(cost<0||(upgrade&&cost==0)||save.credits(slot)<cost)return false;
    if(upgrade)++a.level;else ++a.count;
    save.setCredits(slot,save.credits(slot)-int(cost));save.setAttacks(slot,owned);return true;
}
}
