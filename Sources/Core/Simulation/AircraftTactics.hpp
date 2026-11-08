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
    unsigned leg=0, sector=0, sectorStep=1;
    double nextChange=0;
    Vec2 anchor{}, waypoint{};
    bool engaged=false, braking=false;
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
    Vec2 dodgeDestination{};
    unsigned dodgeCount=0;
    bool invulnerable(double now)const{return dodging&&now<dodgeEnd;}
    void finishDodge(double now){
        dodging=false;dodgeReady=now+.25+double(next()%251)*.001;
        engaged=false;braking=false;
    }
    bool updateDodge(double now,Vec2 position,bool player,float cruise,
                     float radius,float mapWidth,float halfLength,const AircraftThreatGrid& threats){
        if(dodging){
            const float arrival=std::max(mapWidth/512.f,cruise*1.2f);
            if(now>=dodgeEnd||std::hypot(position.x-dodgeDestination.x,position.y-dodgeDestination.y)<=arrival)finishDodge(now);
            return dodging;
        }
        if(now<dodgeReady)return false;
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
        return true;
    }
    std::uint32_t next(){random^=random<<13;random^=random>>17;random^=random<<5;return random;}
    Vec2 destination(double time,Vec2 position,Vec2 target,float range,float mapWidth,float mapHalfLength,float radius){
        const float edge=std::max(0.f,mapWidth*.5f-radius);
        // Twelve clock positions around the target. The sweep stays inside the
        // aircraft's existing movement envelope and its weapon acquisition range.
        const float orbit=std::max(0.f,std::min(mapWidth*widthFraction*.5f,range*.72f));
        const float top=std::max(0.f,mapHalfLength-radius);
        const bool starting=!engaged;
        const float turnDistance=std::max(mapWidth/128.f,radius*.35f);
        const float dx=position.x-waypoint.x,dy=position.y-waypoint.y;
        if(starting||time>=nextChange||dx*dx+dy*dy<=turnDistance*turnDistance){
            if(starting){sector=next()%12;sectorStep=(next()&1u)?1u:11u;engaged=true;}
            else sector=(sector+sectorStep)%12;
            ++leg;
            // Each actor owns its phase/direction/timing; simultaneous builds
            // do not share a left-first route or a world RNG draw.
            nextChange=time+2.4+double(next()%1601)*.001;
            if(!starting&&leg%12==0)sectorStep=(next()&1u)?1u:11u;
        }
        anchor=target;
        const float angle=float(sector)*6.28318530718f/12.f;
        waypoint={std::clamp(target.x+std::sin(angle)*orbit,-edge,edge),
                  std::clamp(target.y+std::cos(angle)*orbit,-top,top)};
        return waypoint;
    }
    // New aircraft only. Velocity changes by bounded acceleration at the
    // existing fixed 60 Hz simulation step. Opposite lateral orders brake to
    // zero for one simulation step before acceleration resumes.
    void advance(AircraftMotion& motion){
        if(motion.state!=1){bank*=.88f;velocityChange={};return;}
        const auto previousVelocity=motion.direction;
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
        velocityChange={motion.direction.x-previousVelocity.x,motion.direction.y-previousVelocity.y};
        const Vec2 left{-std::sin(motion.angle),std::cos(motion.angle)};
        const float lateral=(velocityChange.x*left.x+velocityChange.y*left.y)/std::max(.0001f,deceleration);
        const float drag=(motion.direction.x*left.x+motion.direction.y*left.y)/std::max(.0001f,cruise);
        const float requested=std::clamp(lateral*.38f+drag*.07f,-.4f,.4f);
        bank+=std::clamp((requested-bank)*.18f,-.035f,.035f);
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
