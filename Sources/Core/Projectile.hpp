#pragma once
#include "Combat.hpp"
#include "DarwinRandom.hpp"
#include "RenderTypes.hpp"
#include <optional>
#include <functional>
#include <array>
#include <span>
#include <vector>

namespace md {
struct ProjectileEffect {
    Vec2 position; float life=0,scale=1,scaleVelocity=0,angle=0,rotationVelocity=0;
    bool additive=false,trailPoint=false;
};
// GobBullet: setFire 0x8bbc, tick 0x9558. One call is one original game tick.
struct Projectile {
    Vec2 position,destination,velocity,acceleration,launchVelocity,step;
    float angle=0,speed=0,shotLengthSquared=0,previousLengthSquared=0,startTime=0,effectRegenerationTime=0;
    Vec2 imageScale{1,1};
    bool lockOn=false,drop=false,toPlayer=false,removed=false,effectVisible=true;
    static Projectile fire(const WeaponDefinition&,Vec2 origin,Vec2 destination,float angle,
                           double time,DarwinRandom&,Vec2 launchVelocity={});
    // True means the original proximity test called setCollide. The caller must
    // resolve damage at position and then remove the projectile.
    bool tick(const WeaponDefinition&,bool paused=false,bool shown=true);
    std::optional<Sprite> attachedEffect(const WeaponDefinition&,Vec2 imageSize,Vec2 screenPosition,float deviceScale) const;
    // GobBullet::draw 0xb290. Allocator is queried only when an effect is due.
    std::optional<ProjectileEffect> drawEffect(const WeaponDefinition&,double time,DarwinRandom&,
                                               const std::function<bool()>& acquireParticle);
};
struct ProjectileTarget { Vec2 position; int hp=0; float radius=0; };
struct SpecialDamageApplication {
    WeaponType type; float damage=0,delay=1,damageTime=0,endTime=0;
};
std::optional<SpecialDamageApplication> projectileSpecialDamage(const WeaponDefinition&,double time);
struct StatusEvent {
    WeaponType entryType=WeaponType::railgun;
    bool dealsDamage=false,hasParticle=false;
    int damage=0,sound=-1;
    float particleLife=0,particleAlpha=1,particleAngle=0;
    unsigned animationFrames=8;
};
struct StatusTick { bool checked=false; std::uint32_t flags=0; std::vector<StatusEvent> events; };
class SpecialDamageController {
public:
    // True requests original EMP side effects: clear target and set destination
    // to the unit's current position. Existing entry keeps its damage/delay.
    bool add(const SpecialDamageApplication&);
    StatusTick tick(double time,DarwinRandom&,const std::function<bool()>& acquireParticle,
                    const std::function<void(const StatusEvent&)>& onEvent={});
    [[nodiscard]] std::uint32_t flags() const { return flags_; }
    [[nodiscard]] float nextCheckTime() const { return nextCheck_; }
    // Original used NSDictionary allValues. The caller supplies/retains the
    // measured order here; no sorting or claimed Foundation order equivalence.
    std::vector<SpecialDamageApplication>& orderedEntries() { return entries_; }
    const std::vector<SpecialDamageApplication>& orderedEntries() const { return entries_; }
private:
    std::vector<SpecialDamageApplication> entries_;
    float nextCheck_=0;
    std::uint32_t flags_=0;
};
// QobParticle 0x427bc/0x42a88/0x42fa4. Time is the original float
// animation clock; game-world pause is handled by the owning object tree.
struct ParticleState {
    Vec2 position,velocity,scale{1,1},scaleModulation{1,1};
    float gravity=0,angle=0,rotationVelocity=0,scaleVelocity=0,velocityAdd=0;
    float life=0,startTime=0,alpha=1,maxAlpha=1,waterFrequency=0;
    float sinRotate=0,sinRotationAmplitude=0,easyOutScale=0,easyOutDivisor=0;
    int tile=0,animationFrames=0;
    bool shown=false,removed=false,tileAnimation=false,fadeOut=true,waterAnimation=false,selfRemove=false;
    Blend blend=Blend::additive;
    void reset(float animationTime,DarwinRandom&);
    void start(float animationTime) { startTime=animationTime;shown=true; }
    void setTileAnimation(int frames) { tileAnimation=true;animationFrames=frames;fadeOut=false; }
    void tick(float animationTime,float cameraHalfHeight=0);
    std::optional<Sprite> sprite(std::string_view image,Vec2 imageSize,unsigned columns,unsigned rows,
                                 Vec2 screenPosition,float assetScale=1) const;
};
class ParticlePool {
public:
    static constexpr std::size_t capacity=256;
    ParticlePool(float animationTime,DarwinRandom&);
    ParticleState* acquire(float animationTime,DarwinRandom&);
    std::span<ParticleState> particles() { return particles_; }
private:
    std::array<ParticleState,capacity> particles_;
};
struct ProjectileHit {
    bool player=false; std::size_t index=0; int damage=0; bool secondary=false;
    std::optional<SpecialDamageApplication> special;
};
// Ordered collision events; snapshots' hp are changed between laser chains.
// Apply each event once to real units (including original onDamage side effects).
std::vector<ProjectileHit> projectileCollide(const WeaponDefinition&,Vec2 position,bool toPlayer,
                                           double time,std::span<ProjectileTarget> enemies,
                                           std::span<ProjectileTarget> players);
}
