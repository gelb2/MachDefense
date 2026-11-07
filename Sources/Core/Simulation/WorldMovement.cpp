#include "Core/Simulation/World.hpp"
#include "Core/Simulation/Formation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>

#include "Core/Simulation/WorldSupport.hpp"
namespace md {
using namespace world_detail;
void World::setDestination(WorldActor& a,Vec2 point){a.destination=point;a.bipod.setDestination(point);a.vehicle.setDestination(point);a.aircraft.setDestination(point);a.train.setDestination(point);}
void World::updateActor(WorldActor& a){
    if(a.removed)return;
    if(a.hp<=0){updateCrash(a);return;}
    a.radarAngle+=a.rotorStep;if(a.radarAngle>0x1.921fb6p+2f)a.radarAngle-=0x1.921fb6p+2f;
    auto* target=selectTarget(a);
    const bool aiEnabled=a.player||!(a.statusFlags&2);
    if(aiEnabled&&((a.player&&a.type==0)||a.type==1)){
        std::optional<Vec2> destination;
        if(a.player){std::vector<AlliedObstacle> others;for(const auto& b:actors_)if(b.get()!=&a&&b->player&&!b->removed)others.push_back({b->position,b->radius,b->hp});
            destination=a.playerAI.tick({time(),a.position,a.destination,a.range,baseBoundary_,geometry_.deviceScale,a.radius,target!=nullptr,a.moveToTarget},random_,others);
            topPlayerPosition_=std::max(topPlayerPosition_,a.playerAI.topPlayerPosition);
        }else destination=a.enemyAI.vehicle(time(),a.position,a.range,baseBoundary_,geometry_.deviceScale,target!=nullptr,a.moveToTarget,random_);
        if(destination)setDestination(a,*destination);
    }else if(aiEnabled&&a.type==2&&!a.player){if(auto d=a.enemyAI.aircraft(time(),a.aircraft.state,random_))setDestination(a,*d);}
    if(a.type==5&&a.player&&!a.postBuild.empty()&&a.aircraft.state==0&&std::abs(a.position.x-a.destination.x)<=4&&std::abs(a.position.y-a.destination.y)<=4){
        a.removed=true;a.hp=0;events_.push_back({WorldEvent::Kind::playerDestroyed,a.id,0});addNamed(a.postBuild,a.position,true);return;
    }
    if(a.type==0){a.bipod.position=a.position;a.bipod.tick(target?std::optional<Vec2>(target->position):std::nullopt,a.collision);a.position=a.bipod.position;a.angle=a.bipod.bodyAngle;
        // Player bipod sockets are fixed by setParts: feet 1/2 and body 3.
        if(a.player&&!a.assembly.empty()){auto& root=a.assembly.front();for(auto& part:root.children){
            if(part.parentSocket==1||part.parentSocket==2){const auto& foot=a.bipod.feet[part.parentSocket-1];part.angle=foot.angle;part.addPosition.x=foot.addPosX;part.visualScale=foot.scale;}
            else if(part.parentSocket==3){part.angle=a.bipod.bodyAngle;part.visualScale=a.bipod.bodyScale;part.addPosition.y=a.bipod.bodyAddPosY;}
        }}
    }else if(a.type==1){a.vehicle.position=a.position;a.vehicle.tick(random_,a.collision);a.position=a.vehicle.position;a.angle=a.vehicle.angle;}
    else if(a.type==2||a.type==5){a.aircraft.position=a.position;a.aircraft.tick();a.position=a.aircraft.position;a.angle=a.aircraft.angle;}
    else if(a.type==4){a.train.position=a.position;a.train.tick();a.position=a.train.position;}
    if(a.type!=2&&a.type!=5&&double(a.collisionCheck+.02f)<time()){collideTerrain(a);collideGroundBodies(a);a.collisionCheck=float(time());}
    updatePursuit(a,target);
    if(a.maxHP>0&&a.hp*100/a.maxHP<80&&random_.next()%unsigned(a.hp*200/a.maxHP+10)==0)emitDamageSmoke(a,false);
    ParticleState* statusParticle=nullptr;
    const auto status=a.specialDamage.tick(time(),random_,[&]{statusParticle=particles_->acquire(float(time()),random_);return statusParticle!=nullptr;},[&](const StatusEvent& event){
        if(event.dealsDamage){
            damage(a.id,event.damage);
            // processSpDamage 0x11040 passes raw OpenAL source 15, not SFX15.
            // The WeMade compatibility port loads Missile06.wav into that slot.
            host_.playSound("Missile06.wav");
        }
        if(event.hasParticle&&statusParticle){auto& p=*statusParticle;p.position=a.position;p.life=event.particleLife;p.alpha=p.maxAlpha=event.particleAlpha;p.angle=event.particleAngle;p.setTileAnimation(int(event.animationFrames));setParticleImage(&p,"Efx_Plasma.png",8,1);p.start(float(time()));}
    });a.statusFlags=status.flags;
    if(a.hp>0)updateWeapons(a);
}
void World::order(unsigned index){
    if(index!=0||paused_||state_!=WorldState::running)return;
    for(auto& a:actors_)if(a->player&&!a->removed)if(auto destination=originalRetreatDestination(index,a->position,a->hp,a->type,topPlayerPosition_)){
        setDestination(*a,*destination);a->moveToTarget=false;a->playerAI.isOrder=true;
    }
}
}
