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
    bool engaged=false, braking=false, enteringOrbit=false;
    float entryClearance=0;
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
        const float orbit=std::max(0.f,std::min(mapWidth*widthFraction*.5f,range*.72f));
        const float top=std::max(0.f,mapHalfLength-radius);
        const auto point=[&](unsigned s){
            const float angle=float(s)*6.28318530718f/12.f;
            return Vec2{std::clamp(target.x+std::sin(angle)*orbit,-edge,edge),
                        std::clamp(target.y+std::cos(angle)*orbit,-top,top)};
        };
        const float entryDistance=std::hypot(position.x-target.x,position.y-target.y);
        // Actors already inside the preferred circle need distinct outward
        // approaches; a path cannot start outside a circle it is already in.
        const bool starting=!engaged;
        if(starting)entryClearance=std::min(orbit*.8f,std::max(radius*2,entryDistance*.8f));
        const float clearance=(starting||enteringOrbit)?entryClearance:orbit*.8f;
        const auto clear=[&](Vec2 p){const float x=p.x-target.x,y=p.y-target.y;return x*x+y*y>=clearance*clearance;};
        const float turnDistance=std::max(mapWidth/128.f,radius*.35f);
        const float dx=position.x-waypoint.x,dy=position.y-waypoint.y;
        const bool arrived=dx*dx+dy*dy<=turnDistance*turnDistance;
        if(!starting&&(arrived||entryDistance<=orbit+turnDistance))enteringOrbit=false;
        if(starting||(!enteringOrbit&&time>=nextChange)||arrived){
            if(starting){
                sector=next()%12;sectorStep=(next()&1u)?1u:11u;engaged=true;enteringOrbit=true;
            }else{
                // A clipped point near a map edge can collapse onto the target.
                // Sweep back along the existing circle rather than cut across it.
                if(!clear(point((sector+sectorStep)%12)))sectorStep=12-sectorStep;
                sector=(sector+sectorStep)%12;
            }
            ++leg;
            nextChange=time+2.4+double(next()%1601)*.001;
            if(!starting&&leg%12==0)sectorStep=(next()&1u)?1u:11u;
        }
        anchor=target;waypoint=point(sector);
        // Keep safe random entries. Redirect a target-crossing entry or a
        // path made unsafe by target movement to the nearest usable clock point.
        const Vec2 path{waypoint.x-position.x,waypoint.y-position.y};
        const float length2=path.x*path.x+path.y*path.y;
        const float along=length2>0?std::clamp(((target.x-position.x)*path.x+(target.y-position.y)*path.y)/length2,0.f,1.f):0.f;
        if(!clear({position.x+along*path.x,position.y+along*path.y})){
            float best=1e30f;std::array<unsigned,12> safeEntries{};unsigned count=0;
            for(unsigned i=0;i<12;++i){
                const Vec2 p=point(i);const float x=p.x-position.x,y=p.y-position.y;
                if(!clear(p))continue;
                if(x*x+y*y<best){best=x*x+y*y;sector=i;waypoint=p;}
                const float length=x*x+y*y;
                const float t=length>0?std::clamp(((target.x-position.x)*x+(target.y-position.y)*y)/length,0.f,1.f):0.f;
                if(clear({position.x+t*x,position.y+t*y}))safeEntries[count++]=i;
            }
            // Retain per-aircraft entry variation among the safe approaches.
            // Reuse this actor's RNG state without taking another random draw.
            if(starting&&count){sector=safeEntries[random%count];waypoint=point(sector);}
        }
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
