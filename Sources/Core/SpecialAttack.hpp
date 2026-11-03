#pragma once
#include "Projectile.hpp"
#include <string>
#include <string_view>

namespace md {
// GobHvM_Player::setSpAttack:To: 0xe6a8. Owner/layer are assigned by World.
std::vector<Projectile> crossfireProjectiles(const WeaponDefinition&,Vec2 origin,Vec2 target,
                                             float bodyAngle,double time,DarwinRandom&);
std::string specialWeaponName(std::string_view itemString,float itemLevel);
// GobAirStrike 0x8150/0x82bc/0x86e0. Each tick is one original update.
struct AirStrike {
    Vec2 position,velocity,target;
    float angle=0,startTime=0,fireTime=0,fireDelay=0,deviceScale=1;
    int remainingShots=0;
    bool removed=false;
    static AirStrike start(const WeaponDefinition&,Vec2 target,float itemDelay,double time,
                           float deviceScale,DarwinRandom&,
                           const std::function<ParticleState*()>& acquireMarker={},float animationTime=0);
    std::optional<Projectile> tick(const WeaponDefinition&,double time,DarwinRandom&);
    void appendSprite(Frame&,Vec2 screenPosition,Vec2 imageSize,float retinaScale=.5f) const;
};
// Original GobHvM_Bot::process_Etc 0xcd5c. D/R job methods themselves
// (0xce4c/0xd134) return immediately; no extra defense/repair AI is implied.
struct BotRecovery {
    float lastRefreshTime=0;
    bool tick(int& hp,int maximumHP,double time);
};
// Original GobHvM_Player::aiBot 0xf60c converts a delivery bot on arrival.
bool botReadyToTransform(Vec2 position,Vec2 destination,int machineState,std::string_view postBuild);
}
