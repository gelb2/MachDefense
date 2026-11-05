#include "Core/Simulation/Formation.hpp"
namespace md {
std::optional<Vec2> originalRetreatDestination(unsigned order,Vec2 position,int hp,int type,float top){
    const float distance=top-position.y;
    if(order!=0||hp<=0||type!=0||distance>=120.f)return {};
    return Vec2{position.x,(top-80.f)+distance*-.8f};
}
}
