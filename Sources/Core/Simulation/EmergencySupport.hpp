#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace md {
struct SupportAmount {
    std::int64_t credits=0,cells=0;
    bool operator==(const SupportAmount&)const=default;
    bool empty()const{return credits==0&&cells==0;}
};
// Per-battle state. Uses the existing battle clock; never reads or writes files.
class EmergencySupport {
public:
    static constexpr std::array<unsigned,3> intervals{30,45,60};
    static std::array<SupportAmount,3> amounts(unsigned missionNumber,float cellCapacity,float researchParameter);
    void reset(unsigned missionNumber);
    void advance(double elapsed,float cellCapacity,float researchParameter);
    SupportAmount pending(std::size_t row)const{return pending_.at(row);}
    SupportAmount available(std::size_t row,std::int64_t creditRoom,std::int64_t cellRoom)const;
    SupportAmount claim(std::size_t row,std::int64_t creditRoom,std::int64_t cellRoom);
    unsigned remaining(std::size_t row)const;
private:
    unsigned mission_=1;double elapsed_=0;
    std::array<double,3> next_{30,45,60};
    std::array<SupportAmount,3> pending_{};
};
}
