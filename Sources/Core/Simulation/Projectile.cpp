#include "Core/Simulation/Projectile.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace md {
namespace {
float lengthSquared(Vec2 a) { return a.x*a.x+a.y*a.y; }
Vec2 difference(Vec2 a,Vec2 b) { return {a.x-b.x,a.y-b.y}; }
float angleOf(Vec2 p) { return static_cast<float>(std::atan2(static_cast<double>(p.y),static_cast<double>(p.x))); }
void rotate(Vec2& p,float angle) {
    const float s=std::sin(angle),c=std::cos(angle),x=p.x,y=p.y;
    p={x*c-y*s,x*s+y*c};
}
float wrapped(float current,float desired) {
    constexpr float pi=3.1415927410125732421875f,tau=6.283185482025146484375f;
    if(desired-current>pi) current+=tau;
    else if(desired-current < -pi) current-=tau;
    return current;
}
void easeLaunch(float& value) { // QVector::easyOut:To:Div:Min: 0x4746c
    if(value!=0) value=std::abs(value)>.5f?value+(0-value)/15.0f:0;
}
}
Projectile Projectile::fire(const WeaponDefinition& weapon,Vec2 origin,Vec2 destination,float angle,
                            double time,DarwinRandom& random,Vec2 launch) {
    Projectile p;p.position=origin;p.destination=destination;p.angle=angle;p.launchVelocity=launch;
    p.shotLengthSquared=lengthSquared(difference(destination,origin));
    p.previousLengthSquared=p.shotLengthSquared*10.0f;p.startTime=static_cast<float>(time);
    const float r=static_cast<float>(random.next())*0x1p-31f;
    p.speed=(weapon.speed+(r*weapon.speed)*.2f)+weapon.speed*(-.1f);
    if(weapon.type==WeaponType::missile) {
        p.speed*=.05f;p.acceleration={std::cos(angle)*p.speed,std::sin(angle)*p.speed};
        if(launch.x==0&&launch.y==0) p.velocity=p.acceleration;
    } else p.velocity={std::cos(angle)*p.speed,std::sin(angle)*p.speed};
    return p;
}
bool Projectile::tick(const WeaponDefinition& weapon,bool paused,bool shown) {
    if(paused||!shown) return false;
    if(launchVelocity.x==0&&launchVelocity.y==0) {
        if(weapon.type==WeaponType::missile) {
            velocity.x+=acceleration.x;velocity.y+=acceleration.y;
            if(lockOn) {
                const float desired=angleOf(difference(destination,position));
                const float accelerationAngle=wrapped(angleOf(acceleration),desired);
                rotate(acceleration,(desired-accelerationAngle)/15.0f);
                angle=wrapped(angle,desired);rotate(velocity,(desired-angle)/20.0f);
                angle=angleOf(velocity);
            }
        } else if(weapon.type==WeaponType::railgun) angle+=.1f;
        else angle=angleOf(velocity);
        step={velocity.x/.0166f,velocity.y/.0166f};
        position.x+=step.x;position.y+=step.y;
    } else {
        easeLaunch(launchVelocity.x);easeLaunch(launchVelocity.y);
        position.x+=launchVelocity.x;position.y+=launchVelocity.y;
    }
    bool collide=false;
    if(!removed&&launchVelocity.x==0&&launchVelocity.y==0) {
        const float distance=lengthSquared(difference(destination,position));
        collide=distance<shotLengthSquared/3.0f &&
                (distance>previousLengthSquared||distance<lengthSquared(step));
        if(drop&&!weapon.bulletImage.empty()) {
            const float ratio=distance/shotLengthSquared;
            imageScale={std::max(.1f,ratio+.3f),std::max(.1f,ratio*.7f+.6f)};
        }
        previousLengthSquared=distance;
    }
    if(weapon.effectType==2) effectVisible=!effectVisible;
    return collide;
}
std::optional<Sprite> Projectile::attachedEffect(const WeaponDefinition& weapon,Vec2 imageSize,Vec2 screen,float scale) const {
    if(removed||weapon.effectImage.empty()||imageSize.x<=0||imageSize.y<=0)return {};
    if(weapon.effectType!=1&&weapon.effectType!=2&&weapon.effectType!=8)return {};
    if(weapon.effectType==2&&!effectVisible)return {};
    const float width=imageSize.x/float(std::max(1,weapon.effectColumns)),height=imageSize.y/float(std::max(1,weapon.effectRows));
    Sprite s;s.image=weapon.effectImage;s.sourcePx=Rect{0,0,width,height};s.destination={screen.x-width*scale*.5f,screen.y-height*scale*.5f,width*scale,height*scale};
    s.rotationDegrees=-angle*180.f/3.14159265358979323846f;s.blend=Blend::additive;return s;
}
std::optional<ProjectileEffect> Projectile::drawEffect(const WeaponDefinition& weapon,double time,DarwinRandom& random,
                                                       const std::function<bool()>& acquireParticle) {
    if(weapon.effectType>=5&&weapon.effectType<=8) {
        // setFire creates a smokeTrail only for types 5, 6 and 7.
        if(weapon.effectType==8||launchVelocity.x!=0||launchVelocity.y!=0) return std::nullopt;
        ProjectileEffect effect;effect.position=position;effect.trailPoint=true;return effect;
    }
    if(weapon.effectType!=3&&weapon.effectType!=4) return std::nullopt;
    const float delay=toPlayer?.06f:.02f;
    if(time<=static_cast<double>(effectRegenerationTime+delay)) return std::nullopt;
    effectRegenerationTime=static_cast<float>(time);
    if(!acquireParticle()) return std::nullopt;
    auto r=[&](){return static_cast<float>(random.next())*0x1p-31f;};
    auto randomAngle=[&](){const float a=r()*3.1415927410125732421875f;return a+a;};
    ProjectileEffect e;e.position=position;
    if(weapon.effectType==3) {
        e.additive=true;e.life=.4f;e.position.x+=r()*12.0f-6.0f;e.position.y+=r()*12.0f-6.0f;
        e.scale=r()*.4f+.6f;e.scaleVelocity=-(r()*.008f+.008f);
        e.angle=randomAngle();e.rotationVelocity=r()*.05f+.1f;
    } else if(toPlayer) {
        e.life=.5f;e.scale=r()*.2f+.1f;e.scaleVelocity=r()*.5f+.5f;
        e.angle=randomAngle();e.rotationVelocity=r()*3.0f-1.5f;
    } else {
        e.life=.6f;e.scale=r()*.4f+.2f;e.scaleVelocity=r()*.015f+.005f;
        e.angle=randomAngle();e.rotationVelocity=r()*.5f-.25f;
    }
    return e;
}
std::optional<SpecialDamageApplication> projectileSpecialDamage(const WeaponDefinition& weapon,double time) {
    if(weapon.type!=WeaponType::railgun&&weapon.type!=WeaponType::emp) return std::nullopt;
    return SpecialDamageApplication{weapon.type,weapon.special2,1.0f,static_cast<float>(time),
                                    static_cast<float>(time+static_cast<double>(weapon.special1))};
}
bool SpecialDamageController::add(const SpecialDamageApplication& entry) {
    if(entry.type!=WeaponType::railgun&&entry.type!=WeaponType::emp) return false;
    const auto existing=std::find_if(entries_.begin(),entries_.end(),[&](const auto& v){return v.type==entry.type;});
    if(existing==entries_.end()) entries_.push_back(entry);
    else {existing->endTime=entry.endTime;existing->damageTime=entry.damageTime;}
    return entry.type==WeaponType::emp;
}
StatusTick SpecialDamageController::tick(double time,DarwinRandom& random,
        const std::function<bool()>& acquireParticle,const std::function<void(const StatusEvent&)>& onEvent) {
    const float now=static_cast<float>(time);StatusTick result;result.flags=flags_;
    if(!(nextCheck_<now)) return result;
    result.checked=true;flags_=0;
    for(auto& entry:entries_) {
        if(entry.type==WeaponType::railgun) flags_|=1;
        else if(entry.type==WeaponType::emp) flags_|=2;
        if(now>entry.endTime) continue; // flags are set before original expiry check.
        const bool damage=(flags_&1)&&(entry.damageTime+entry.delay<now);
        const bool emp=!damage&&(flags_&2)&&(entry.damageTime+.5f<now);
        if(!damage&&!emp) continue;
        StatusEvent event;event.entryType=entry.type;event.dealsDamage=damage;
        event.damage=damage?static_cast<int>(entry.damage):0;event.sound=damage?14:-1;
        event.particleLife=damage?.3f:.5f;event.particleAlpha=damage?1.f:.5f;
        event.hasParticle=acquireParticle();
        if(event.hasParticle) {
            const float angle=(static_cast<float>(random.next())*0x1p-31f)*3.1415927410125732421875f;
            event.particleAngle=angle+angle;
        }
        result.events.push_back(event);
        if(onEvent) onEvent(event); // Original onDamage can consume shared RNG here.
        entry.damageTime=now;
    }
    std::erase_if(entries_,[&](const auto& entry){return now>entry.endTime;});
    nextCheck_=now+.1f;result.flags=flags_;return result;
}
void ParticleState::reset(float time,DarwinRandom& random) {
    tile=0;alpha=1;maxAlpha=1;position={};velocity={};gravity=0;scale={1,1};angle=0;
    rotationVelocity=0;scaleVelocity=0;easyOutDivisor=0;velocityAdd=0;life=0;
    tileAnimation=false;animationFrames=0;startTime=time;blend=Blend::additive;
    waterFrequency=(static_cast<float>(random.next())*0x1p-31f)*20.f+50.f;
    waterAnimation=false;fadeOut=true;scaleModulation={1,1};selfRemove=false;
    // Original reset leaves sinRotate/sinRotValue/easyOutScale unchanged.
}
void ParticleState::tick(float now,float cameraHalfHeight) {
    if(!shown) return;
    Vec2 step=velocity;step.y+=gravity*(-.0166f);
    position.x+=step.x;position.y+=step.y;angle+=rotationVelocity;
    scale.x+=scaleVelocity;scale.y+=scaleVelocity;
    if(velocityAdd!=0) {step.x+=step.x*velocityAdd;step.y+=step.y*velocityAdd;}
    velocity=step;
    if(life==0) return;
    const float elapsed=now-startTime;
    if(!(elapsed<life)) {shown=false;if(selfRemove)removed=true;return;}
    if(tileAnimation) tile=static_cast<int>((elapsed/life)*static_cast<float>(animationFrames)+.5f);
    if(fadeOut) alpha=(1.f-elapsed/life)*maxAlpha;
    if(waterAnimation) {
        const float phase=elapsed*waterFrequency;
        scaleModulation={std::cos(phase)*.3f+1.f,std::sin(phase*1.5f)*.3f+1.f};
        angle=std::sin(phase);
        if(position.y>cameraHalfHeight+40.f) {shown=false;if(selfRemove)removed=true;}
    }
    if(sinRotate!=0) angle=std::sin(elapsed*sinRotate)*sinRotationAmplitude;
    if(easyOutDivisor==0) {scale.x+=scaleVelocity;scale.y+=scaleVelocity;}
    else {scale.x+=(easyOutScale-scale.x)/easyOutDivisor;scale.y+=(easyOutScale-scale.y)/easyOutDivisor;}
}
std::optional<Sprite> ParticleState::sprite(std::string_view image,Vec2 size,unsigned columns,unsigned rows,
                                          Vec2 screenPosition,float assetScale) const {
    if(!shown||image.empty()||size.x<=0||size.y<=0||columns==0||rows==0) return std::nullopt;
    const unsigned selected=std::min(static_cast<unsigned>(std::max(0,tile)),columns*rows-1);
    const float width=size.x/static_cast<float>(columns),height=size.y/static_cast<float>(rows);
    const float w=width*scale.x*scaleModulation.x*assetScale,h=height*scale.y*scaleModulation.y*assetScale;
    Sprite s;s.image=image;s.sourcePx=Rect{float(selected%columns)*width,float(selected/columns)*height,width,height};
    s.destination={screenPosition.x-w*.5f,screenPosition.y-h*.5f,w,h};
    s.rotationDegrees=-angle*180.f/3.14159265358979323846f;s.color={1,1,1,alpha};s.blend=blend;return s;
}
ParticlePool::ParticlePool(float time,DarwinRandom& random) { for(auto& p:particles_) p.reset(time,random); }
ParticleState* ParticlePool::acquire(float time,DarwinRandom& random) {
    for(auto& p:particles_) if(!p.shown) {p.reset(time,random);return &p;}return nullptr;
}
std::vector<ProjectileHit> projectileCollide(const WeaponDefinition& weapon,Vec2 position,bool toPlayer,
        double time,std::span<ProjectileTarget> enemies,std::span<ProjectileTarget> players) {
    std::vector<ProjectileHit> hits;
    const auto special=projectileSpecialDamage(weapon,time);
    auto apply=[&](std::span<ProjectileTarget> targets,std::size_t i,bool player,int damage,bool secondary) {
        const auto hp=static_cast<std::int64_t>(targets[i].hp)-damage;
        targets[i].hp=static_cast<int>(std::clamp<std::int64_t>(hp,0,INT32_MAX));
        hits.push_back({player,i,damage,secondary,special});
    };
    if(weapon.damageRange==0) {
        // Original 0xa568 always selects listEnemy in this branch, even toPlayer.
        for(std::size_t i=0;i<enemies.size();++i) {
            if(enemies[i].hp<=0) continue;
            if(lengthSquared(difference(position,enemies[i].position))>=enemies[i].radius*enemies[i].radius) continue;
            apply(enemies,i,false,weapon.damage,false);
            if(weapon.type==WeaponType::laser) { // set2ndDamage 0x9f80
                for(std::size_t j=0;j<enemies.size();++j) {
                    if(j==i||enemies[j].hp<=0) continue;
                    if(lengthSquared(difference(position,enemies[j].position))<weapon.special1*weapon.special1)
                        apply(enemies,j,false,static_cast<int>(static_cast<float>(weapon.damage)*.5f),true);
                }
            }
        }
    } else {
        auto targets=toPlayer?players:enemies;const float range=weapon.damageRange;
        for(std::size_t i=0;i<targets.size();++i) {
            if(targets[i].hp<=0) continue;
            const Vec2 delta=difference(position,targets[i].position);
            if(std::abs(delta.x)>=range||std::abs(delta.y)>=range) continue;
            const float falloff=1.0f-std::sqrt(lengthSquared(delta))/range;
            if(falloff>0) apply(targets,i,toPlayer,static_cast<int>(static_cast<float>(weapon.damage)*std::min(1.0f,falloff+.5f)),false);
        }
    }
    return hits;
}
}
