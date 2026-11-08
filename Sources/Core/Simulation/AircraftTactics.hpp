#pragma once
#include "Core/Simulation/Battle.hpp"
#include "Core/Simulation/AircraftDodge.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace md {
// Experimental routes and inertial movement for the four trial aircraft only.
// Projectile physics, hit probability, terrain and original motion stay unchanged.
struct AircraftTactics {
    float widthFraction=.5f, speed=2.f;
    std::uint32_t random=1;
    Vec2 anchor{}, waypoint{};
    bool engaged=false, braking=false, holdingRange=false;
    float bank=0; // radians, positive toward the local left side
    Vec2 velocityChange{};
    float noseStep=.02908882f, weaponStep=.05235988f;
    void face(AircraftMotion& motion,float desired){
        const float delta=std::remainder(desired-motion.angle,6.28318530718f);
        motion.angle+=std::clamp(delta/8.f,-noseStep,noseStep);
    }
    std::uint64_t targetId=0;
    bool dodging=false;
    double dodgeEnd=0, dodgeReady=0;
    Vec2 dodgeDestination{},dodgeOrigin{},dodgeAxis{},flightVelocity{};
    double dodgeStart=0,dodgeNow=0,dodgeArrival=0;
    float dodgeTicks=0,dodgeLength=0,dodgeInitialSpeed=0,dodgePeakSpeed=0,dodgeRiseTicks=0;
    static constexpr double stepSeconds=double(.0166f);
    // Bound preparation to the previous 0.45-second maneuver. Predict its
    // arrival tick so changing the speed curve does not extend immunity.
    static unsigned priorArrivalTicks(Vec2 position,Vec2 destination,Vec2 velocity,float cruise,float arrival){
        const float maximum=cruise*2.5f,acceleration=maximum/6.f;
        for(unsigned tick=0;tick<28;++tick){
            const Vec2 delta{destination.x-position.x,destination.y-position.y};
            const float distance=std::hypot(delta.x,delta.y);
            if(tick&&distance<=arrival)return tick;
            const float wanted=std::min(maximum,std::sqrt(std::max(0.f,2*acceleration*distance)));
            const Vec2 goal=distance>.001f?Vec2{delta.x/distance*wanted,delta.y/distance*wanted}:Vec2{};
            Vec2 change{goal.x-velocity.x,goal.y-velocity.y};const float length=std::hypot(change.x,change.y);
            if(length<=acceleration)velocity=goal;
            else{velocity.x+=change.x/length*acceleration;velocity.y+=change.y/length*acceleration;}
            position.x+=velocity.x;position.y+=velocity.y;
        }
        return 28;
    }
    void prepareDodgeCurve(double now,Vec2 position,float cruise,float arrival){
        dodgeOrigin=position;dodgeStart=dodgeNow=now;
        dodgeLength=std::hypot(dodgeDestination.x-position.x,dodgeDestination.y-position.y);
        dodgeAxis={(dodgeDestination.x-position.x)/dodgeLength,(dodgeDestination.y-position.y)/dodgeLength};
        const auto priorTicks=priorArrivalTicks(position,dodgeDestination,flightVelocity,cruise,arrival);
        const double duration=std::min(.45,double(priorTicks)*stepSeconds);
        dodgeArrival=now+duration;dodgeTicks=float(duration/stepSeconds);
        dodgeInitialSpeed=std::clamp(flightVelocity.x*dodgeAxis.x+flightVelocity.y*dodgeAxis.y,0.f,dodgeLength/dodgeTicks);
        const float initialTravel=dodgeTicks*dodgeInitialSpeed;
        dodgePeakSpeed=(2*dodgeLength-initialTravel+std::sqrt(initialTravel*initialTravel+4*dodgeLength*dodgeLength))/(2*dodgeTicks);
        dodgeRiseTicks=dodgeLength/(dodgeInitialSpeed+dodgePeakSpeed);
    }
    void updateBank(AircraftMotion& motion,Vec2 previous,float deceleration,float cruise){
        velocityChange={motion.direction.x-previous.x,motion.direction.y-previous.y};
        const Vec2 left{-std::sin(motion.angle),std::cos(motion.angle)};
        const float lateral=(velocityChange.x*left.x+velocityChange.y*left.y)/std::max(.0001f,deceleration);
        const float drag=(motion.direction.x*left.x+motion.direction.y*left.y)/std::max(.0001f,cruise);
        const float requested=std::clamp(lateral*.38f+drag*.07f,-.4f,.4f);
        bank+=std::clamp((requested-bank)*.18f,-.035f,.035f);
        flightVelocity=motion.direction;
    }
    unsigned dodgeCount=0;
    bool invulnerable(double now)const{return dodging&&now<dodgeEnd;}
    void finishDodge(double now){
        dodging=false;dodgeReady=now+.25+double(next()%251)*.001;
        engaged=false;braking=false;
    }
    bool updateDodge(double now,Vec2 position,bool player,float cruise,
                     float radius,float mapWidth,float halfLength,const AircraftThreatGrid& threats){
        if(dodging){
            dodgeNow=now;
            if(now>=dodgeEnd||(dodgeTicks>0&&now>=dodgeArrival)||
               std::hypot(position.x-dodgeDestination.x,position.y-dodgeDestination.y)<=.0001f)finishDodge(now);
            return dodging;
        }
        if(now<dodgeReady||!(cruise>0)||!std::isfinite(cruise))return false;
        const AircraftThreatGrid::Threat* danger=nullptr;float closest=1e30f;
        threats.near(position,cruise*21,radius,player,[&](const auto& t){
            // Existing weapons impact their fixed destination. Check the current
            // and near-future position against that impact area.
            const float reach=std::max(t.radius,radius)+cruise*6;
            const float dx=t.impact.x-position.x,dy=t.impact.y-position.y;
            const float distance=dx*dx+dy*dy;
            if(distance<=reach*reach&&distance<closest){closest=distance;danger=&t;}
        });
        if(!danger)return false;
        const float distance=std::clamp(std::max(danger->radius,radius)*1.3f,cruise*16,cruise*45);
        const float edge=std::max(0.f,mapWidth*.5f-radius),top=std::max(0.f,halfLength-radius);
        const unsigned first=next()%12;float best=-1e30f;Vec2 selected=position;
        for(unsigned n=0;n<12;++n){
            const float angle=float((first+n)%12)*6.28318530718f/12;
            Vec2 candidate{std::clamp(position.x+std::sin(angle)*distance,-edge,edge),std::clamp(position.y+std::cos(angle)*distance,-top,top)};
            float safety=1e30f;
            threats.near(candidate,cruise*21,radius,player,[&](const auto& t){safety=std::min(safety,std::hypot(candidate.x-t.impact.x,candidate.y-t.impact.y)-std::max(t.radius,radius));});
            if(safety>cruise*2&&std::hypot(candidate.x-position.x,candidate.y-position.y)>distance*.5f){selected=candidate;break;}
            // Randomized first safe direction is per actor; penalize edges
            // that leave no room to move. No changes to hit radii or weapon stats.
            safety=std::min(safety,distance*2)+std::hypot(candidate.x-position.x,candidate.y-position.y)*.05f;
            if(safety>best+.001f){best=safety;selected=candidate;}
        }
        if(std::hypot(selected.x-position.x,selected.y-position.y)<cruise)return false;
        dodgeDestination=selected;dodging=true;dodgeEnd=now+.45;++dodgeCount;braking=false;
        prepareDodgeCurve(now,position,cruise,std::max(mapWidth/512.f,cruise*1.2f));
        return true;
    }
    std::uint32_t next(){random^=random<<13;random^=random>>17;random^=random<<5;return random;}
    Vec2 destination(double,Vec2 position,Vec2 target,float range,float mapWidth,float mapHalfLength,float radius){
        const float edge=std::max(0.f,mapWidth*.5f-radius);
        const float top=std::max(0.f,mapHalfLength-radius);
        const float dx=position.x-target.x,dy=position.y-target.y;
        const float distance=std::hypot(dx,dy);
        range=std::max(0.f,range);anchor=target;
        // An acquired target is already in the assigned unit range. Brake at
        // that point instead of generating another orbit leg through the target.
        if(!engaged){holdingRange=distance<=range;engaged=true;}
        else if(holdingRange&&distance>range)holdingRange=false;
        else if(!holdingRange&&distance<=range*.9f)holdingRange=true;
        if(holdingRange)return waypoint=position;
        // Reapproach toward a standoff point, not the target centre. The inner
        // stop and outer resume distances prevent boundary oscillation.
        const float standoff=std::min(distance,range*.8f);
        waypoint={std::clamp(target.x+(distance>0?dx/distance*standoff:0.f),-edge,edge),
                  std::clamp(target.y+(distance>0?dy/distance*standoff:0.f),-top,top)};
        return waypoint;
    }
    // New aircraft only. Velocity changes by bounded acceleration at the
    // existing fixed 60 Hz simulation step. Opposite lateral orders brake to
    // zero for one simulation step before acceleration resumes.
    void advance(AircraftMotion& motion){
        if(motion.state!=1){bank*=.88f;velocityChange={};flightVelocity=motion.direction;return;}
        const auto previousVelocity=motion.direction;
        if(dodging&&dodgeTicks>0){
            const float tick=std::clamp(float((dodgeNow-dodgeStart)/stepSeconds+1),0.f,dodgeTicks);
            float distance,velocity;
            if(tick<=dodgeRiseTicks){
                const float acceleration=(dodgePeakSpeed-dodgeInitialSpeed)/dodgeRiseTicks;
                distance=dodgeInitialSpeed*tick+.5f*acceleration*tick*tick;
                velocity=dodgeInitialSpeed+acceleration*tick;
            }else{
                const float remaining=dodgeTicks-tick;
                const float deceleration=dodgePeakSpeed/(dodgeTicks-dodgeRiseTicks);
                distance=dodgeLength-.5f*deceleration*remaining*remaining;velocity=deceleration*remaining;
            }
            distance=std::clamp(distance,0.f,dodgeLength);
            motion.position={dodgeOrigin.x+dodgeAxis.x*distance,dodgeOrigin.y+dodgeAxis.y*distance};
            motion.direction={dodgeAxis.x*velocity,dodgeAxis.y*velocity};motion.speed=velocity;
            if(tick>=dodgeTicks){motion.position=dodgeDestination;motion.direction={};motion.speed=0;}
            updateBank(motion,previousVelocity,motion.moveSpeed*2.5f/4.f,motion.moveSpeed*2.5f);
            if(!engaged&&velocity>0)face(motion,std::atan2(dodgeAxis.y,dodgeAxis.x));
            return;
        }
        const Vec2 delta{motion.destination.x-motion.position.x,motion.destination.y-motion.position.y};
        const float distance=std::hypot(delta.x,delta.y),cruise=motion.moveSpeed*(dodging?2.5f:1.f);
        const float acceleration=cruise/(dodging?6.f:30.f),deceleration=cruise/(dodging?4.f:24.f);
        const float wantedSpeed=std::min(cruise,std::sqrt(std::max(0.f,2*acceleration*distance)));
        const Vec2 desired=distance>.001f?Vec2{delta.x/distance*wantedSpeed,delta.y/distance*wantedSpeed}:Vec2{};
        if(!dodging&&!braking&&motion.direction.x*desired.x<0)braking=true;
        const Vec2 goal=braking?Vec2{}:desired;
        const float step=braking?deceleration:acceleration;
        Vec2 change{goal.x-motion.direction.x,goal.y-motion.direction.y};const float length=std::hypot(change.x,change.y);
        if(length<=step)motion.direction=goal;
        else{motion.direction.x+=change.x/length*step;motion.direction.y+=change.y/length*step;}
        updateBank(motion,previousVelocity,deceleration,cruise);
        motion.speed=std::hypot(motion.direction.x,motion.direction.y);
        if(braking&&motion.speed==0){braking=false;return;}
        if(!engaged&&!braking&&wantedSpeed>0){
            const float desiredAngle=std::atan2(desired.y,desired.x);
            face(motion,desiredAngle);
        }
        motion.position.x+=motion.direction.x;motion.position.y+=motion.direction.y;
        if(distance<.001f&&motion.speed==0)motion.state=0;
    }

};
}
