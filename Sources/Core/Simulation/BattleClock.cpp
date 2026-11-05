#include "Core/Simulation/Battle.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace md {
ProductionJob ProductionJob::begin(std::string name,double time,float duration,float factor) {
    return {std::move(name),float(time),duration*factor};
}
float ProductionJob::progress(double time) const { return float((time-double(startTime))/double(duration)); }
void BattleClock::start(float capacity,float rate) {
    capacity_=capacity;rate_=rate;cells_=capacity*.4f;
}
void BattleClock::tick(bool shown,bool paused,int state) {
    if(!shown||paused) return;
    time_+=double(.0166f);
    if(state==0 && double(nextCellTime_)<=time_) {
        // Original increment can exceed capacity; subsequent ticks leave it as is.
        if(cells_<capacity_) cells_+=rate_;
        nextCellTime_=float(time_+double(.08f));
    }
}
int BattleClock::enemyBaseHP(int stage) {
    if(stage<0 || stage>(std::numeric_limits<int>::max()-40000)/10000) throw std::out_of_range("Invalid stage index");
    return 40000+stage*10000;
}
float BattleClock::initialDifficultyScale(int completed) { return .3f + (completed>4 ? .7f : .7f*.25f*float(completed)); }

}
