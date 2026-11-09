#include "Core/Simulation/World.hpp"
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
    if(a.flightTactics){a.flightHeight=0;a.flightTactics->bank=0;a.aircraft.direction={};a.aircraft.speed=0;}
    if(random_.next()%20==0)emitDamageSmoke(a,true);
    if(time()>double(a.crashTime)){emitCrashExplosion(a.position);host_.playSound("SFX_ExplMach.wav");a.removed=true;}
}void World::emitWeaponImpact(const WeaponDefinition& weapon,Vec2 position,float angle,float frameDuration){
    if(weapon.explodeImage.empty())return;
    unsigned columns=4,rows=4;
    if(weapon.explodeImage=="Efx_Explode01.png"){columns=4;rows=2;}
    else if(weapon.explodeImage=="Efx_Explode02.png"){columns=6;rows=4;}
    else if(weapon.explodeImage=="Efx_ExplodeGround.png"){columns=4;rows=5;}
    else if(weapon.explodeImage=="Efx_Damage.png"){columns=6;rows=1;}
    else if(weapon.explodeImage=="Efx_DamageHMG.png"){columns=8;rows=1;}
    if(auto* p=particles_->acquire(float(time()),random_)){
        p->blend=Blend::alpha;p->life=float(columns*rows)*frameDuration;p->position=position;p->angle=angle;
        p->setTileAnimation(int(columns*rows));
        const float effectScale=weapon.type==WeaponType::sniper?1.f:weapon.explodeScale;p->scale={effectScale,effectScale};
        setParticleImage(p,weapon.explodeImage,columns,rows);p->start(float(time()));
    }
}
void World::setParticleImage(ParticleState* p,std::string image,unsigned columns,unsigned rows){
    const auto i=std::size_t(p-particles_->particles().data());Vec2 origin;if(image=="Efx_Explode02.png")origin.y=-44;if(image=="Efx_DamageHMG.png")origin.x=-24;
    const float assetScale=(image=="Efx_BulletTrail.png"||image=="Efx_Lightning.png")?1.f:0.f;
    particleImages_.at(i)={std::move(image),columns,rows,origin,assetScale};
}
void World::prepareDraw(){
    if(!started_||paused_)return;
    for(auto& bullet:projectiles_){const auto* weapon=weapons_.find(bullet.weapon);if(!weapon)continue;ParticleState* particle=nullptr;
        const auto effect=bullet.motion.drawEffect(*weapon,time(),random_,[&]{particle=particles_->acquire(float(time()),random_);return particle!=nullptr;});
        if(effect&&effect->trailPoint&&bullet.trail)bullet.trail->add(effect->position,time(),random_);
        if(effect&&particle&&!effect->trailPoint){auto& p=*particle;p.position=effect->position;p.life=effect->life;p.scale={effect->scale,effect->scale};p.scaleVelocity=effect->scaleVelocity;p.angle=effect->angle;p.rotationVelocity=effect->rotationVelocity;p.blend=effect->additive?Blend::additive:Blend::alpha;setParticleImage(&p,weapon->effectImage,unsigned(weapon->effectColumns),unsigned(weapon->effectRows));p.start(float(time()));}
    }
}

}
