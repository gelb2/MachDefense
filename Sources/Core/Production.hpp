#pragma once
#include "Battle.hpp"
#include <array>
#include <functional>
#include <optional>
#include <unordered_map>
namespace md {
struct BuildDefinition {
    std::string name;
    int setType=0,placementType=0,cost=0,upgradeCost=0,maximumConcurrent=0;
    float duration=0;
    int armor=0,weapon=0,foot=0,radius=0;
    std::string bot,machine;
};
class BuildCatalog {
public:
    static BuildCatalog load(const DataTable& parts,const DataTable& names);
    const BuildDefinition* find(std::string_view name) const;
    std::size_t size()const{return definitions_.size();}
private:
    std::unordered_map<std::string,BuildDefinition> definitions_;
};
struct ProducedUnit { BuildDefinition definition;Vec2 position;std::optional<Vec2> destination; };
struct ProductionProgress {std::string name;float progress=0;};
class ProductionQueue {
public:
    // WndBuildSlot::addBuildItem... 0x30000/0x30104. Seven simultaneous
    // jobs, with an independent Mul limit for each build definition.
    bool enqueue(const BuildDefinition&,double time,float factor,float& cells,std::optional<Vec2> destination={});
    std::vector<ProducedUnit> tick(double time,BattleGeometry,DarwinRandom&);
    void tick(double time,BattleGeometry,DarwinRandom&,const std::function<void(const ProducedUnit&)>&);
    std::size_t size()const{return jobs_.size();}
    std::vector<ProductionProgress> rows(double time)const;
    unsigned occupied(std::string_view definitionName)const;
    void clear(){jobs_.clear();}
private:
    struct Job { BuildDefinition definition;ProductionJob timer;std::optional<Vec2> destination; };
    std::vector<Job> jobs_;
};
}
