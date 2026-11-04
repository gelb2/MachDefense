#include "World.hpp"
#include <cmath>
namespace md {
// GobHvMach::onCrash 0x1422c, processCrash 0x1151c, tick 0x110f0.
void World::emitCrashExplosion(Vec2 position){
    if(auto* p=particles_->acquire(float(time()),random_)){
        p->blend=Blend::alpha;p->life=1;p->position=position;p->setTileAnimation(16);
        setParticleImage(p,"Efx_ExplodeNPC.png",4,4);p->start(float(time()));
    }
}
void World::emitDamageSmoke(const WorldActor& a,bool crashed){
    auto* p=particles_->acquire(float(time()),random_);if(!p)return;
    auto random=[&]{return float(random_.next())*0x1p-31f;};
    p->blend=Blend::alpha;p->life=crashed?1.f:.7f;
    const float scale=random()*(crashed?.4f:.2f)+(crashed?.4f:.2f);p->scale={scale,scale};
    p->scaleVelocity=random()*(crashed?.005f:.003f)+.001f;
    p->angle=random()*0x1.921fb4p+2f;p->rotationVelocity=random()*.02f;
    // Alive damage smoke has unscaled velocity in the original; wreck smoke
    // applies deviceScale to both axes.
    p->velocity.x=-.5f-random()*.1f;p->velocity.y=random()*.1f+.2f;
    if(crashed){p->velocity.x*=geometry_.deviceScale;p->velocity.y*=geometry_.deviceScale;}
    const float radius=a.radius*(crashed?.3f:.5f);
    p->position={a.position.x+(random()*(radius+radius)-radius),a.position.y+(random()*(radius+radius)-radius)};
    p->setTileAnimation(16);setParticleImage(p,"Efx_Smoke.png",4,4);p->start(float(time()));
}
void World::emitSecondaryImpact(const WeaponDefinition& weapon,Vec2 from,Vec2 to){
    // GobBullet::set2ndDamage 0x9f80: after damage/status, then arc and impact.
    if(auto* p=particles_->acquire(float(time()),random_)){
        const Vec2 delta{from.x-to.x,from.y-to.y};p->blend=Blend::alpha;p->life=8.f*.04f;
        p->position={from.x-delta.x*.5f,from.y-delta.y*.5f};p->setTileAnimation(8);
        p->angle=float(std::atan2(double(delta.y),double(delta.x)));
        p->scale={std::sqrt(delta.x*delta.x+delta.y*delta.y)*.0078125f,geometry_.deviceScale};
        setParticleImage(p,"Efx_Lightning.png",2,4);p->start(float(time()));
    }
    auto secondary=weapon;secondary.explodeScale*=.8f;emitWeaponImpact(secondary,to,0,.04f);
}
void World::updateCrash(WorldActor& a){
    if(random_.next()%20==0)emitDamageSmoke(a,true);
    if(time()>double(a.crashTime)){emitCrashExplosion(a.position);host_.playSound("SFX_ExplMach.wav");a.removed=true;}
}
}
