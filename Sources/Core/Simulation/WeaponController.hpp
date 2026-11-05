#pragma once
#include "Core/Simulation/Combat.hpp"
#include "Core/Contracts/DarwinRandom.hpp"
#include "Core/Contracts/RenderTypes.hpp"
#include <functional>
#include <optional>

namespace md {
struct WeaponTarget { Vec2 position; int hp=0; };
struct WeaponFrame {
    double time=0;
    Vec2 ownerPosition,muzzlePosition;
    float bodyAngle=0,deviceScale=.5f;
    std::optional<WeaponTarget> target;
    bool ownerAndPartPresent=true,uiModel=false,manualFire=false,weaponReady=true;
    int ownerHP=1,machineType=1;
    std::uint32_t statusFlags=0;
};
struct WeaponFire {
    Vec2 origin,destination;
    float angle=0;
    bool directDamage=false;
    // A dead target can remain referenced until the unit's next target check.
    bool hasTarget=false;
};
struct WeaponTickResult {
    unsigned shots=0;
    bool advanceMuzzle=false,playFireSound=false;
    float recoil=0;
};
class WeaponController {
public:
    explicit WeaponController(float startTime=0):startTime_(startTime) {}
    void setReload(double time);
    // This method changes aim/readiness exactly as PartsController_WeaponAI.
    // Type 0 preserves readiness maintained by the bipod movement controller.
    bool track(const WeaponFrame& frame);
    WeaponTickResult tick(const WeaponDefinition& weapon,const WeaponFrame& frame,DarwinRandom& random,
                          const std::function<void(const WeaponFire&)>& fire);
    [[nodiscard]] float angle() const { return angle_; }
    void setAngle(float angle) { angle_=angle; }
    [[nodiscard]] int shotsLeft() const { return fireLeft_; }
    [[nodiscard]] float lastFireTime() const { return fireTime_; }
private:
    float startTime_=0,angle_=0,fireTime_=0,recoil_=0;
    int fireLeft_=0;
};
}
