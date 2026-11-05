#include "Core/Simulation/Battle.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace md {
std::optional<std::size_t> nearestLivingTarget(Vec2 from,float range,std::span<const TargetCandidate> candidates) {
    float shortest=range*range;std::optional<std::size_t> result;
    for(std::size_t i=0;i<candidates.size();++i) {
        if(candidates[i].hp<=0) continue;
        float dx=from.x-candidates[i].position.x,dy=from.y-candidates[i].position.y;
        float distance=dx*dx+dy*dy;
        if(distance<shortest) {shortest=distance;result=i;}
    }
    return result;
}
int hpAfterDamage(int hp,int damage) {
    auto result=std::int64_t(hp)-std::int64_t(damage);
    return int(std::clamp(result,std::int64_t(0),std::int64_t(std::numeric_limits<int>::max())));
}

}
