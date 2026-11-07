#pragma once
#include "Core/Simulation/Battle.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace md {
// Experimental routes and inertial movement for the four trial aircraft only.
// Projectile physics, hit probability, terrain and original motion stay unchanged.
struct AircraftTactics {
    float widthFraction=.5f, speed=2.f;
    std::uint32_t random=1;
    unsigned leg=0;
    double nextChange=0;
    Vec2 anchor{}, waypoint{};
    bool engaged=false, braking=false;
    std::uint64_t targetId=0;
    std::uint32_t next(){random^=random<<13;random^=random>>17;random^=random<<5;return random;}
    Vec2 destination(double time,Vec2 position,Vec2 target,float range,float mapWidth,float mapHalfLength,float radius){
        const float edge=std::max(1.f,mapWidth*.5f-radius);
        const float span=std::min(mapWidth*widthFraction*.5f,edge);
        const float depth=std::min(span*.45f,range*.22f);
        const bool starting=!engaged;
        if(!engaged){anchor={std::clamp(position.x,-edge+span,edge-span),std::clamp(target.y-range*.55f,-mapHalfLength+radius+depth,mapHalfLength-radius-depth)};engaged=true;nextChange=time;}
        const float turnDistance=std::max(mapWidth/64.f,radius*.5f);
        const float dx=position.x-waypoint.x,dy=position.y-waypoint.y;
        if(time>=nextChange||(!starting&&dx*dx+dy*dy<=turnDistance*turnDistance)){
            // Alternating sides, with all three vertical offsets in six legs.
            anchor.y=std::clamp(target.y-range*.55f,-mapHalfLength+radius+depth,mapHalfLength-radius-depth);
            ++leg;nextChange=time+3.0+double(next()%1000)*.001;
        }
        const unsigned direction=(leg-1)%6;
        const float side=direction%2?1.f:-1.f;
        constexpr float levels[]{0,0,1,1,-1,-1};
        waypoint={std::clamp(anchor.x+side*span,-edge,edge),std::clamp(anchor.y+levels[direction]*depth,-mapHalfLength+radius,mapHalfLength-radius)};
        return waypoint;
    }
    // New aircraft only. Velocity changes by bounded acceleration at the
    // existing fixed 60 Hz simulation step. Opposite lateral orders brake to
    // zero for one simulation step before acceleration resumes.
    void advance(AircraftMotion& motion){
        if(motion.state!=1)return;
        const Vec2 delta{motion.destination.x-motion.position.x,motion.destination.y-motion.position.y};
        const float distance=std::hypot(delta.x,delta.y),cruise=motion.moveSpeed;
        const float acceleration=cruise/30.f,deceleration=cruise/24.f;
        const float wantedSpeed=std::min(cruise,std::sqrt(std::max(0.f,2*acceleration*distance)));
        const Vec2 desired=distance>.001f?Vec2{delta.x/distance*wantedSpeed,delta.y/distance*wantedSpeed}:Vec2{};
        if(!braking&&motion.direction.x*desired.x<0)braking=true;
        const Vec2 goal=braking?Vec2{}:desired;
        const float step=braking?deceleration:acceleration;
        Vec2 change{goal.x-motion.direction.x,goal.y-motion.direction.y};const float length=std::hypot(change.x,change.y);
        if(length<=step)motion.direction=goal;
        else{motion.direction.x+=change.x/length*step;motion.direction.y+=change.y/length*step;}
        motion.speed=std::hypot(motion.direction.x,motion.direction.y);
        if(braking&&motion.speed==0){braking=false;return;}
        if(!braking&&wantedSpeed>0){
            const float desiredAngle=std::atan2(desired.y,desired.x);
            const float turn=std::remainder(desiredAngle-motion.angle,6.28318530718f);
            motion.angle+=turn/motion.rotationDivisor;
        }
        motion.position.x+=motion.direction.x;motion.position.y+=motion.direction.y;
        if(distance<.001f&&motion.speed==0)motion.state=0;
    }

};
}
