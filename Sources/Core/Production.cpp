#include "Production.hpp"
#include <algorithm>
#include <stdexcept>
namespace md {
BuildCatalog BuildCatalog::load(const DataTable& parts,const DataTable& names){
    BuildCatalog result;
    auto load=[&](const DataTable& t,int setType){
        for(std::size_t row=0;row<t.rowCount();++row){
            auto n=[&](std::string_view key){auto value=t.integer(row,key);if(!value)throw std::runtime_error("Invalid build integer: "+std::string(key));return int(*value);};
            BuildDefinition d;d.name=t.value(row,"MachName");d.setType=setType;
            d.cost=n("Cost");d.upgradeCost=n("UpgCost");d.maximumConcurrent=n("Mul");
            const auto duration=t.number(row,"Bld");if(!duration||*duration<=0)throw std::runtime_error("Invalid production duration");d.duration=*duration;
            const auto type=t.value(row,"Type");
            if(type=="MACH")d.placementType=0;else if(type=="UNIT")d.placementType=1;else throw std::runtime_error("Unknown build type");
            if(setType==0){d.armor=n("Armor");d.weapon=n("Weapon");d.foot=n("Foot");d.radius=n("Size");}
            else {d.bot=t.value(row,"Bot");d.machine=t.value(row,"Turret");}
            result.definitions_.insert_or_assign(d.name,std::move(d));
        }
    };
    load(parts,0);load(names,1);return result;
}
const BuildDefinition* BuildCatalog::find(std::string_view name)const {auto i=definitions_.find(std::string(name));return i==definitions_.end()?nullptr:&i->second;}
std::vector<ProductionProgress> ProductionQueue::rows(double time)const{
    std::vector<ProductionProgress> result;for(const auto& j:jobs_)result.push_back({j.definition.name,j.timer.progress(time)});return result;
}
unsigned ProductionQueue::occupied(std::string_view name)const{return unsigned(std::count_if(jobs_.begin(),jobs_.end(),[&](const Job& j){return j.definition.name==name;}));}
bool ProductionQueue::enqueue(const BuildDefinition& d,double time,float factor,float& cells,std::optional<Vec2> destination){
    const auto same=std::count_if(jobs_.begin(),jobs_.end(),[&](const Job& j){return j.definition.name==d.name;});
    if(jobs_.size()>=7||float(d.cost)>cells||same>=d.maximumConcurrent)return false;
    if(d.placementType!=0&&!destination)return false;
    jobs_.push_back({d,ProductionJob::begin(d.name,time,d.duration,factor),destination});
    cells-=float(d.cost);return true;
}
std::vector<ProducedUnit> ProductionQueue::tick(double time,BattleGeometry geometry,DarwinRandom& random){
    std::vector<ProducedUnit> result;
    tick(time,geometry,random,[&](const ProducedUnit& unit){result.push_back(unit);});
    return result;
}
void ProductionQueue::tick(double time,BattleGeometry geometry,DarwinRandom& random,const std::function<void(const ProducedUnit&)>& spawn){
    for(auto i=jobs_.begin();i!=jobs_.end();){
        if(!i->timer.complete(time)){++i;continue;}
        Vec2 position{0,-geometry.halfLength()};
        // BuildItem::tick 0x30644: random X only for the parts-built walkers.
        if(i->definition.setType==0)position.x=(float(random.next())*0x1p-31f*440.f-220.f)*geometry.deviceScale;
        const ProducedUnit unit{i->definition,position,i->destination};
        i=jobs_.erase(i);
        spawn(unit);
    }
}
}
