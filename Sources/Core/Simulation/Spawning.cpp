#include "Core/Simulation/Battle.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace md {
namespace {
float randomFloat(DarwinRandom& random) { return float(random.next())*0x1p-31f; }
}
EnemySpawner::EnemySpawner(SpawnRule rule,double time,DarwinRandom& random):rule_(std::move(rule)) {
    if(!std::isfinite(rule_.startTime)||!std::isfinite(rule_.delay)||rule_.name.empty()) throw std::invalid_argument("Invalid spawn rule");
    if(rule_.startTime==-1) rule_.count=int(float(rule_.count)*.5f+(randomFloat(random)*float(rule_.count))*.5f+.5f);
    rule_.count=std::max(1,rule_.count);
    nextTime_=float(time+double(rule_.startTime));
}
std::optional<SpawnEvent> EnemySpawner::tick(double time,int state,BattleGeometry geometry,DarwinRandom& random) {
    if(finished()||state!=0||!(double(nextTime_)<time)) return std::nullopt;
    // Preserve original float operation order: scale enters before randomness.
    float halfWidth=220.f*geometry.deviceScale;
    SpawnEvent event{rule_.name,{randomFloat(random)*(halfWidth+halfWidth)-halfWidth,geometry.halfLength()+32.f*geometry.deviceScale}};
    ++generated_;
    float halfDelay=rule_.delay*.5f;
    nextTime_=float(time+double(halfDelay)+double(randomFloat(random)*halfDelay));
    return event;
}

StageSpawnPlan StageSpawnPlan::parse(const Chunk& map,const DataTable& table,BattleGeometry geometry) {
    StageSpawnPlan plan;
    auto shortAt=[](const Chunk& c,std::size_t p) {return std::bit_cast<std::int16_t>(std::uint16_t(std::uint16_t(c.byte(p))|std::uint16_t(c.byte(p+1))<<8));};
    for(const auto& c:map.children) {
        if(c.type==4) {
            plan.placed.push_back({c.text(2,16),{float(shortAt(c,18))*geometry.deviceScale,float(shortAt(c,20))*geometry.deviceScale}});
        } else if(c.type==3) {
            int count=shortAt(c,0);std::vector<Vec2> polygon;
            for(int i=0;i<count;++i) polygon.push_back({c.real32(2+8*i)*geometry.deviceScale,c.real32(6+8*i)*geometry.deviceScale});
            if(!polygon.empty()) plan.collisionPolygons.push_back(std::move(polygon));
        }
    }
    auto integer=[&](std::size_t row,std::string_view key){auto v=table.integer(row,key);if(!v && table.value(row,key).empty()) return 0;if(!v) throw std::runtime_error("Invalid original ESpawn integer");return int(*v);};
    for(std::size_t row=0;row<table.rowCount();++row) {
        SpawnRule rule;rule.name=table.value(row,"MachName");rule.count=integer(row,"Cnt");rule.delay=float(integer(row,"Dly"));
        int dif=integer(row,"Dif");
        if(dif>=1 && dif<=5) {
            auto& bucket=plan.difficulty[dif-1];
            if(bucket.size()<32) {
                rule.startTime=-1;rule.cumulativeRate=(bucket.empty()?0:bucket.back().cumulativeRate)+integer(row,"Rte");
                bucket.push_back(rule);
            }
        } else {
            rule.startTime=float(dif/100)*60.f+float(dif%100);plan.timed.push_back(rule);
        }
        if(std::find(plan.enemyNames.begin(),plan.enemyNames.end(),rule.name)==plan.enemyNames.end()) plan.enemyNames.push_back(rule.name);
    }
    std::sort(plan.enemyNames.begin(),plan.enemyNames.end());return plan;
}
void StageSpawnPlan::appendGeneratedWaves(int stage,DarwinRandom& random,const std::function<int(std::string_view)>& hp) {
    if(stage<0||stage>=80||enemyNames.empty()) throw std::invalid_argument("Invalid original wave stage");
    int selected=std::min(4,int(enemyNames.size()));
    float interval=std::max(5.f,60.f-float(stage)),base=90,span=30,accumulated=0;
    int scale=stage*10+2000;
    int budget=stage*((stage*10+7000)-std::max(scale,5000))+6000;
    int increment=scale>5000?3000:int(float(scale)*.6f);
    for(int wave=0;wave<7;++wave) {
        float time=base+randomFloat(random)*span;base+=interval;accumulated+=time;
        int allowance=int(float(budget)/float(selected)),remaining=budget,available=0;
        for(int i=int(enemyNames.size())-1;i>=int(enemyNames.size())-selected;--i) {
            available=std::min(remaining,available+allowance);int health=hp(enemyNames[i]);
            if(health>0 && health<=available) {
                SpawnRule rule;rule.name=enemyNames[i];rule.count=available/health;
                float r=randomFloat(random);rule.delay=(r+r)+2.f;
                rule.startTime=accumulated+randomFloat(random)*5.f;
                timed.push_back(rule);remaining-=health*rule.count;available-=health*rule.count;
            }
        }
        span+=10;budget+=increment;
    }
}
EnemyDirector::EnemyDirector(std::array<std::vector<SpawnRule>,5> rules,int completed):rules_(std::move(rules)),scale_(BattleClock::initialDifficultyScale(completed)) {}
std::optional<EnemySpawner> EnemyDirector::tick(double time,DarwinRandom& random) {
    if(double(nextCheck_)<time) {
        nextCheck_=float(time+double(.4f));
        phaseA_+=randomFloat(random)*((1.2f-difficulty_)*.02f);
        phaseB_+=randomFloat(random)*((1.2f-difficulty_)*.01f);
        if(scale_<1) scale_=std::min(1.f,scale_+randomFloat(random)*.002f);
        difficulty_=(((1.f-std::cos(phaseA_))*.5f+(1.f-std::cos(phaseB_))*.5f)*.5f)*scale_;
    }
    if(!(double(nextSpawn_)<time)) return std::nullopt;
    int level=std::clamp(int(difficulty_*5),0,4);
    while(level>0 && rules_[level].empty()) --level;
    auto& bucket=rules_[level];if(bucket.empty()||bucket.back().cumulativeRate<=0) throw std::runtime_error("Empty original difficulty bucket");
    int choice=int(random.next()%unsigned(bucket.back().cumulativeRate));
    auto found=std::find_if(bucket.begin(),bucket.end(),[&](const auto& r){return choice<r.cumulativeRate;});
    EnemySpawner spawner(*found,time,random);
    float oneMinus=1.f-difficulty_;
    float jitter=randomFloat(random)*(15.f+(30.f-15.f)*oneMinus);
    float interval=10.f+(20.f-10.f)*oneMinus;
    nextSpawn_=float(time+double(jitter)+double(interval));return spawner;
}

}
