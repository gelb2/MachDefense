#include "Core/Simulation/EmergencySupport.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
namespace md {
std::array<SupportAmount,3> EmergencySupport::amounts(unsigned mission,float capacity,float research){
    const auto reward=500+25*(std::clamp(mission,1u,80u)-1);
    const auto cells=std::int64_t(std::floor(std::max(0.,std::min(double(capacity)*.1,double(research)*48.))));
    return {{{reward*4/100,0},{0,cells},{reward*2/100,cells/2}}};
}
void EmergencySupport::reset(unsigned mission){mission_=mission;elapsed_=0;next_={30,45,60};pending_={};}
void EmergencySupport::advance(double elapsed,float capacity,float research){
    if(!std::isfinite(elapsed)||elapsed<elapsed_)return;
    elapsed_=elapsed;
    const auto grants=amounts(mission_,capacity,research);
    for(std::size_t i=0;i<3;++i)if(elapsed>=next_[i]){
        const double count=std::floor((elapsed-next_[i])/intervals[i])+1;
        next_[i]+=count*intervals[i];
        auto add=[count](std::int64_t& balance,std::int64_t amount){
            const auto room=std::numeric_limits<std::int64_t>::max()-balance;
            if(amount>0)balance+=count>double(room/amount)?room:std::int64_t(count)*amount;
        };
        add(pending_[i].credits,grants[i].credits);add(pending_[i].cells,grants[i].cells);
    }
}
SupportAmount EmergencySupport::available(std::size_t row,std::int64_t credits,std::int64_t cells)const{
    const auto p=pending_.at(row);return {std::min(p.credits,std::max(std::int64_t(0),credits)),std::min(p.cells,std::max(std::int64_t(0),cells))};
}
SupportAmount EmergencySupport::claim(std::size_t row,std::int64_t credits,std::int64_t cells){
    auto result=available(row,credits,cells);auto& p=pending_.at(row);p.credits-=result.credits;p.cells-=result.cells;return result;
}
unsigned EmergencySupport::remaining(std::size_t row)const{return unsigned(std::max(0.,std::ceil(next_.at(row)-elapsed_)));}
}
