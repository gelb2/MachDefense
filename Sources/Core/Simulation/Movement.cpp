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
int originalStepSpeedPoint(float stepSize) { // setStepSize 0x13de0, ARM conditional instructions.
    if(stepSize==0)return 0;
    float position=0,total=0,elapsed=0;
    do {
        const float next=std::abs(static_cast<double>(position-stepSize))>.5
            ?position+(stepSize-position)*.125f:stepSize;
        elapsed+=.0166f;total+=next-position;position=next;
    } while(position!=stepSize);
    return static_cast<int>(total/elapsed);
}
int originalVehicleSpeedPoint(float moveSpeed) {return static_cast<int>(moveSpeed/.0166f);}

void separateGroundBodies(Vec2& first,float firstRadius,bool firstTurret,
                          Vec2& second,float secondRadius,bool secondTurret) {
    const float radius=firstRadius+secondRadius;
    Vec2 delta{first.x-second.x,first.y-second.y};
    if(std::abs(delta.x)>=radius||std::abs(delta.y)>=radius)return;
    const float squared=delta.x*delta.x+delta.y*delta.y;
    if(squared==0||squared>=radius*radius)return;
    const float distance=std::sqrt(squared);
    delta.x/=distance;delta.y/=distance;
    const float push=(radius-distance)*.3f;
    delta.x*=push;delta.y*=push;
    if(firstTurret){second.x-=delta.x;second.y-=delta.y;}
    else if(secondTurret){first.x+=delta.x;first.y+=delta.y;}
    else {
        const float firstShare=firstRadius/radius,secondShare=secondRadius/radius;
        first.x+=delta.x*firstShare;first.y+=delta.y*firstShare;
        second.x-=delta.x*secondShare;second.y-=delta.y*secondShare;
    }
}
void VehicleMotion::tick(DarwinRandom& random,std::optional<CollisionSurface> collision) {
    if(state!=1) return;
    Vec2 delta{destination.x-position.x,destination.y-position.y};
    if(std::abs(delta.x)<=4 && std::abs(delta.y)<=4) {state=0;speed=0;return;}
    float desired=float(std::atan2(double(delta.y),double(delta.x)));
    if(collision && delta.x*collision->normal.x+delta.y*collision->normal.y>0) {
        desired=float(std::atan2(double(collision->direction.y),double(collision->direction.x)));
        if(delta.x*collision->direction.x+delta.y*collision->direction.y<0)
            desired=float(double(desired)+3.2415926550799092);
        else desired-=.1f;
    }
    float difference=desired-angle;
    constexpr float pi=0x1.921fb6p+1f,turn=0x1.921fb4p+2f;
    if(difference>pi) angle+=turn;else if(difference < -pi) angle-=turn;
    angle+=(desired-angle)/rotationDivisor;
    float distanceSquared=delta.x*delta.x+delta.y*delta.y;
    speed+=(distanceSquared>900 ? (moveSpeed-speed)/100.f : (.5f-speed)/60.f);
    direction={std::cos(angle)*speed,std::sin(angle)*speed};
    position.x+=direction.x;position.y+=direction.y;
    if(random.next()%5==0) {
        position.x+=randomFloat(random)-.5f;
        position.y+=randomFloat(random)-.5f;
        scale=(randomFloat(random)*.02f-.01f)+1.f;
    }
}

void AircraftMotion::tick() {
    if(state!=1) return;
    Vec2 delta{destination.x-position.x,destination.y-position.y};
    if(std::abs(delta.x)<=4 && std::abs(delta.y)<=4) {state=0;return;}
    float desired=float(std::atan2(double(delta.y),double(delta.x))),difference=desired-angle;
    constexpr float pi=0x1.921fb6p+1f,turn=0x1.921fb4p+2f;
    if(difference>pi) angle+=turn;else if(difference < -pi) angle-=turn;
    angle+=(desired-angle)/rotationDivisor;
    float distanceSquared=delta.x*delta.x+delta.y*delta.y;
    speed+=(distanceSquared>900 ? (moveSpeed-speed)/100.f : (.5f-speed)/60.f);
    direction={std::cos(angle)*speed,std::sin(angle)*speed};position.x+=direction.x;position.y+=direction.y;
}

void TrainMotion::tick() { // GobHvMach::processMove_Train 0x135dc
    if(state!=1) return;
    const Vec2 delta{destination.x-position.x,destination.y-position.y};
    if(std::abs(delta.x)<=4&&std::abs(delta.y)<=4) {state=0;speed=0;return;}
    const float squared=delta.x*delta.x+delta.y*delta.y;
    speed+=(squared>900?(moveSpeed-speed)/100.f:(.5f-speed)/60.f);
    const float length=std::sqrt(squared);
    direction={delta.x/length*speed,delta.y/length*speed};
    position.x+=direction.x;position.y+=direction.y;
}

EnemySteering EnemySteering::begin(double time,DarwinRandom& random) {
    EnemySteering result;result.firstAttack=random.next()%15==0;
    result.stopDelay=(float(time)+randomFloat(random))+.5f;return result;
}
std::optional<Vec2> EnemySteering::vehicle(double time,Vec2 position,float targetRange,float boundary,float scale,bool hasTarget,bool moveToTarget,DarwinRandom& random) {
    if(time<=double(stopDelay)||(hasTarget&&moveToTarget)) return std::nullopt;
    float x=position.x+(randomFloat(random)*32.f-16.f);
    x=std::clamp(x,-110.f*scale,110.f*scale);
    float y=position.y-160.f*scale,limit=targetRange*.8f-boundary;
    if(y<limit) {y=limit;stopDelay=float(time+100000.0);}
    else {float r=randomFloat(random);stopDelay=float(time+double(r+r)+1.0);}
    return Vec2{x,y};
}
std::optional<Vec2> EnemySteering::aircraft(double time,int state,DarwinRandom& random) {
    if(double(stopDelay)>=time && state!=0) return std::nullopt;
    stopDelay=float(time+double(randomFloat(random)*5.f)+5.0);
    float x=randomFloat(random)*1200.f-600.f,y=randomFloat(random)*1200.f-600.f;
    return Vec2{x,y};
}

}
