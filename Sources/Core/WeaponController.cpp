#include "WeaponController.hpp"
#include <cmath>

namespace md {
namespace {
float randomFloat(DarwinRandom& random) { return float(random.next())*0x1p-31f; }
void wrapToward(float desired,float& angle) {
    float delta=desired-angle;
    if(double(delta)>3.141592653589793) angle+=0x1.921fb4p+2f;
    else if(delta < -0x1.921fb6p+1f) angle-=0x1.921fb4p+2f;
}
}
void WeaponController::setReload(double time) {fireLeft_=0;fireTime_=float(time);}
bool WeaponController::track(const WeaponFrame& frame) {
    bool ready=frame.weaponReady;
    if(!frame.ownerAndPartPresent || frame.uiModel || frame.ownerHP<1) return ready;
    if(frame.target && frame.target->hp>0) {
        auto target=frame.target->position;
        float desired=float(std::atan2(double(target.y-frame.ownerPosition.y),double(target.x-frame.ownerPosition.x)));
        wrapToward(desired,angle_);angle_+=(desired-angle_)/10.f;
        if(frame.machineType!=0) ready=std::abs(angle_-desired)<.1f;
    } else {
        wrapToward(frame.bodyAngle,angle_);
        float sway=std::cos(float((frame.time-double(startTime_))*.5))*.5f;
        angle_+=((frame.bodyAngle+sway)-angle_)*.125f;
    }
    return ready;
}
WeaponTickResult WeaponController::tick(const WeaponDefinition& weapon,const WeaponFrame& frame,DarwinRandom& random,const std::function<void(const WeaponFire&)>& fire) {
    WeaponTickResult result;
    auto finish=[&] {
        if(recoil_!=0) recoil_=std::abs(recoil_)>.01f ? recoil_+(0-recoil_)/10.f : 0;
        result.recoil=recoil_;return result;
    };
    bool livingTarget=frame.target && frame.target->hp>0;
    if(!frame.ownerAndPartPresent || (!livingTarget&&!frame.manualFire) || (frame.statusFlags&2)) return finish();
    float elapsed=float(frame.time-double(fireTime_));
    if(fireLeft_<1 && weapon.reloadDelay<=elapsed) fireLeft_=weapon.shotCount;
    if(fireLeft_<1 || elapsed<weapon.shotDelay || !frame.weaponReady) return finish();
    if(frame.target) {
        float x=frame.ownerPosition.x-frame.target->position.x,y=frame.ownerPosition.y-frame.target->position.y;
        if(!(x*x+y*y<float(int(weapon.shotRange)*int(weapon.shotRange)))) return finish();
    }
    result.advanceMuzzle=true;
    WeaponFire event;event.origin=frame.muzzlePosition;event.hasTarget=frame.target.has_value();
    event.directDamage=weapon.type==WeaponType::gun || weapon.type==WeaponType::sniper;
    if(frame.target) {
        Vec2 normal{std::sin(angle_),-std::cos(angle_)};
        Vec2 delta{frame.target->position.x-event.origin.x,frame.target->position.y-event.origin.y};
        float spread=((normal.x*delta.y-normal.y*delta.x)*.1f)*weapon.aimRate;
        float sideways=((normal.x*delta.x+normal.y*delta.y)*.5f)*weapon.aimRate;
        event.destination.x=(frame.target->position.x-normal.x*sideways)+(randomFloat(random)*(spread+spread)-spread);
        event.destination.y=(frame.target->position.y-normal.y*sideways)+(randomFloat(random)*(spread+spread)-spread);
    } else {
        float angle=angle_+(randomFloat(random)*.1f-.05f);
        float distance=float(weapon.shotRange)*.8f+(randomFloat(random)*float(weapon.shotRange))*.2f;
        event.destination={event.origin.x+std::cos(angle)*distance,event.origin.y+std::sin(angle)*distance};
    }
    event.angle=float(std::atan2(double(event.destination.y-event.origin.y),double(event.destination.x-event.origin.x)));
    while(fireLeft_>0 && !(frame.time<double(fireTime_+weapon.shotDelay))) {
        --fireLeft_;fireTime_=float(frame.time);++result.shots;
        // Original creates muzzle/ejection particles before a bullet. The caller
        // handles those effects and their RNG use in this callback.
        fire(event);
    }
    recoil_=-5.f*frame.deviceScale;result.playFireSound=!weapon.fireSound.empty();
    return finish();
}
}
