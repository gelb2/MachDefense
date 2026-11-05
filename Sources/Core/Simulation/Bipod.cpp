#include "Core/Simulation/Bipod.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace md {
namespace {
constexpr double pi=3.14159265358979323846;
constexpr float partTurn=0x1.921fb4p+2f;
float wrapPartNear(float current,float desired){
    const auto delta=desired-current;
    if(double(delta)>pi)current+=partTurn;else if(double(delta)<-pi)current-=partTurn;
    return current;
}
float aimPart(float current,float desired,float divisor,bool snap=false){
    current=wrapPartNear(current,desired);current+=(desired-current)/divisor;
    if(snap&&double(std::abs(current-desired))<.01)current=desired;
    return current;
}
float randomUnit(DarwinRandom& rng){return float(rng.next())*0x1p-31f;}
}
void BipodMotion::setDestination(Vec2 point){destination=point;state=1;if(phase==0)phase=1;}
void BipodMotion::footChange(){
    rightFootActive=!rightFootActive;stepPosition=feet[rightFootActive?1:0].addPosX;
    bodyAdvance=0;phase=plantedTurn?4:3;
}
void BipodMotion::setStepSize(float value){
    if(!std::isfinite(value))throw std::invalid_argument("Non-finite bipod step");
    stepSize=value;
    // The original ARM vcvt.s32 of 0/0 produces zero when stepSize is zero.
    if(value==0){speedPoint=0;return;}
    float accumulated=0,elapsed=0,at=0;
    do {
        // Ghidra omits this conditional: 0x13e28/0x13e34 snap within 0.5.
        const float next=std::abs(at-value)>.5f?at+(value-at)*.125f:value;
        accumulated+=next-at;elapsed+=.0166f;at=next;
    }while(at!=value);
    speedPoint=int(accumulated/elapsed);
}
void BipodMotion::tick(std::optional<Vec2> target,std::optional<CollisionSurface> collision){
    if(state==1){
        Vec2 delta{destination.x-position.x,destination.y-position.y};
        if(std::abs(delta.x)<=4&&std::abs(delta.y)<=4){footChange();state=0;}
        else if(phase==4){
            auto& foot=feet[rightFootActive?1:0];foot.angle=aimPart(foot.angle,angle,12,true);
            stepPosition+=(0-stepPosition)*.0625f;if(foot.angle==angle)phase=1;
        }else if(phase==3){
            auto& foot=feet[rightFootActive?1:0];foot.angle=aimPart(foot.angle,angle,10,true);
            if(stepPosition==stepSize||std::abs(stepPosition-stepSize)<=.5f)stepPosition=stepSize;
            else stepPosition+=(stepSize-stepPosition)*.125f;
            if(stepPosition==stepSize)phase=1;
            const float wave=std::sin(float(double(stepPosition/stepSize)*pi));
            foot.scale=wave*.2f+1;bodyScale=wave*.04f+1;
            bodyAddPosY=stepPosition*(rightFootActive?-.1f:.1f);
        }else if(phase==1){
            float desired=float(std::atan2(double(delta.y),double(delta.x)));
            if(collision&&delta.x*collision->normal.x+delta.y*collision->normal.y>0){
                desired=float(std::atan2(double(collision->direction.y),double(collision->direction.x)));
                if(delta.x*collision->direction.x+delta.y*collision->direction.y<0)desired=float(double(desired)+3.2415926550799092);
                else desired-=.1f;
            }
            const float difference=desired-angle;
            if(double(difference)>pi)angle=float(double(angle)+2*pi);
            else if(double(difference)<-pi)angle=float(double(angle)-2*pi);
            if(!plantedTurn||std::abs(angle-desired)>1)phase=2;
            if(phase==2){
                if(angle>desired)angle-=std::min(.5f,angle-desired);
                else angle+=std::min(.5f,desired-angle);
                footChange();
            }
        }
    }else stepPosition+=(0-stepPosition)/6.f;
    // 0x1281e: the previous filtered advance must be retained for velocity.
    const float previous=bodyAdvance;bodyAdvance+=(stepPosition-bodyAdvance)/6.f;
    feet[rightFootActive?1:0].addPosX=stepPosition-bodyAdvance;
    feet[rightFootActive?0:1].addPosX=-bodyAdvance;
    speed=bodyAdvance-previous;direction={std::cos(angle)*speed,std::sin(angle)*speed};
    position.x+=direction.x;position.y+=direction.y;
    if(target){
        const Vec2 delta{target->x-position.x,target->y-position.y};
        const float desired=float(std::atan2(double(delta.y),double(delta.x)));
        bodyAngle=aimPart(bodyAngle,desired,10);leftWeaponAngle=rightWeaponAngle=bodyAngle;
        weaponReady=double(std::abs(desired-bodyAngle))<.1;
        const float squared=delta.x*delta.x+delta.y*delta.y;
        if(squared<20000){const float converge=squared/-80000.f+.25f;leftWeaponAngle-=converge;rightWeaponAngle+=converge;}
        // The original base interpolation has no angle normalization here.
        baseAngle+=(angle-baseAngle)/10.f;
    }else{
        bodyAngle=aimPart(bodyAngle,angle,10);leftWeaponAngle=rightWeaponAngle=baseAngle=bodyAngle;
        // weaponReady remains owned by target acquisition when no target exists.
    }
}
Vec2 checkPlayerMoveAhead(Vec2 position,Vec2 requested,float radius,float scale,std::span<const AlliedObstacle> others){
    const float reach=160*scale;
    std::vector<const AlliedObstacle*> candidates;
    for(const auto& unit:others)if(unit.hp>0&&unit.position.y>position.y+radius&&unit.position.y<position.y+reach&&std::abs(unit.position.x-position.x)<reach)candidates.push_back(&unit);
    if(candidates.empty())return requested;
    const Vec2 delta{requested.x-position.x,requested.y-position.y};
    const float length=std::sqrt(delta.x*delta.x+delta.y*delta.y);
    const Vec2 original=length<.00001f?Vec2{}:Vec2{delta.x/length,delta.y/length};
    Vec2 direction=original;float sweep=0;bool negative=false;
    while(true){
        bool blocked=false;
        for(const auto* unit:candidates){
            const Vec2 v{unit->position.x-position.x,unit->position.y-position.y};
            const float ahead=direction.x*v.x+direction.y*v.y;
            const float across=direction.x*v.y-direction.y*v.x;
            if(ahead>radius&&ahead<reach&&std::abs(across)<unit->radius){blocked=true;break;}
        }
        if(!blocked)return {position.x+direction.x*reach,position.y+direction.y*reach};
        if(!negative)sweep+=.2f;
        const float turn=negative?-sweep:sweep;negative=!negative;
        const float c=std::cos(turn),s=std::sin(turn);
        direction={original.x*c-original.y*s,original.x*s+original.y*c};
        if(double(sweep)>pi/3)return requested;
    }
}
std::optional<Vec2> PlayerSteering::tick(const PlayerMovementFrame& f,DarwinRandom& rng,std::span<const AlliedObstacle> others){
    std::optional<Vec2> result;
    const float boundary=f.baseBoundary+f.targetRange*-.8f;
    if(f.time>double(stopDelay)&&!isOrder&&!(f.hasTarget&&f.moveToTarget)&&f.position.y<boundary){
        float width=16*f.deviceScale;
        float x=f.position.x+(randomUnit(rng)*(width+width)-width);
        x=std::clamp(x,-110*f.deviceScale,110*f.deviceScale);
        const float y=std::min(f.position.y+160*f.deviceScale,boundary);
        stopDelay=float(f.time+double(randomUnit(rng)*2)+1);
        result=checkPlayerMoveAhead(f.position,{x,y},f.radius,f.deviceScale,others);
    }
    if(isOrder&&std::abs(f.position.x-f.destination.x)<=4&&std::abs(f.position.y-f.destination.y)<=4){
        isOrder=false;stopDelay=float(f.time+double(randomUnit(rng))+1);
    }
    if(topPlayerPosition<f.position.y)topPlayerPosition=f.position.y;
    return result;
}
}
