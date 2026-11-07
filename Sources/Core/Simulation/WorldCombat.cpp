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
void World::configureWeapons(WorldActor& actor){
    actor.weapons.clear();actor.range=0;
    std::function<void(std::vector<AssemblyPart>&,std::vector<std::size_t>)> walk=[&](auto& nodes,auto path){
        for(std::size_t i=0;i<nodes.size();++i){auto p=path;p.push_back(i);auto& part=nodes[i];
            if(const auto* weapon=weapons_.find(part.name)){
                WorldWeapon w;w.name=part.name;w.partPath=p;w.controller=WeaponController(float(time()));w.controller.setAngle(actor.angle);
                actor.weapons.push_back(std::move(w));actor.range=float(weapon->shotRange);
            }
            walk(part.children,p);
        }
    };walk(actor.assembly,{});
}
bool World::activateSpecial(std::string_view name,Vec2 point){
    if(!started_||paused_||state_!=WorldState::running)return false;
    auto owned=save_.attacks(slot_);auto selected=std::find_if(owned.begin(),owned.end(),[&](const auto& a){return a.name==name;});
    if(selected==owned.end()||selected->count==0)return false;
    const auto& attack=attacks_.definition(*selected);char code[16];std::snprintf(code,sizeof code,"%05d",attack.itemId);
    const auto row=itemsTable_.findRow(code);if(!row)return false;
    const auto prefix=itemsTable_.value(*row,"strParam");
    const auto weaponName=specialWeaponName(prefix,itemsTable_.number(*row,"Param1").value_or(0));
    const auto* weapon=weapons_.find(weaponName);if(!weapon)return false;
    auto acquireMarker=[&](){auto* p=particles_->acquire(float(time()),random_);if(p)setParticleImage(p,"Target.png",2,1);return p;};
    if(prefix.starts_with("AS_")){
        auto motion=AirStrike::start(*weapon,point,itemsTable_.number(*row,"Param2").value_or(0),time(),geometry_.deviceScale,random_,acquireMarker,float(time()));
        airStrikes_.push_back({weaponName,std::move(motion)});host_.playSound("SFX_AirStrike.wav");
    }else if(prefix.starts_with("CF_")){
        unsigned fired=0;
        for(const auto& a:actors_)if(a->player&&!a->removed&&a->hp>0&&(a->type==0||a->flightTactics)&&a->position.y<point.y){
            const float distance=distanceSquared(a->position,point);
            if(distance>10000.f&&distance<=float(weapon->shotRange*weapon->shotRange)){
                auto bullets=crossfireProjectiles(*weapon,a->position,point,a->angle,time(),random_);
                for(auto& bullet:bullets)addProjectile(weaponName,std::move(bullet),a->id,1);
                ++fired;
            }
        }
        if(auto* p=acquireMarker()){
            p->blend=Blend::additive;p->life=fired?3.f:.5f;p->tile=0;p->position=point;p->scale={15,15};
            p->easyOutScale=1.5f;p->easyOutDivisor=6;p->rotationVelocity=.1f;p->start(float(time()));
        }
        host_.playSound(fired?"SFX_AirStrike.wav":"Game_LowHP.wav");
        if(!fired)return false;
    }else return false;
    --selected->count;save_.setAttacks(slot_,owned);return true;
}
void World::updateWeapons(WorldActor& a){
    auto* target=actor(a.target);
    for(auto& gun:a.weapons){const auto* definition=weapons_.find(gun.name);if(!definition)continue;
        auto& part=partAt(a.assembly,gun.partPath);
        WeaponFrame frame;frame.time=time();frame.ownerPosition=a.position;frame.bodyAngle=a.angle;frame.deviceScale=geometry_.deviceScale;
        frame.ownerHP=a.hp;frame.machineType=a.type;frame.statusFlags=a.statusFlags;frame.weaponReady=a.type!=0||a.bipod.weaponReady;
        if(target)frame.target=WeaponTarget{target->position,target->hp};
        frame.weaponReady=gun.controller.track(frame);
        if(a.type==0){gun.controller.setAngle(part.reverse&2?a.bipod.rightWeaponAngle:a.bipod.leftWeaponAngle);}
        part.angle=gun.controller.angle();part.addPosition.x=gun.recoil;if(a.type==0)part.addPosition.y=a.bipod.bodyAddPosY;
        const auto poses=models_.pose(a.assembly,geometry_.deviceScale,a.angle,gun.controller.angle(),a.radarAngle);
        // Flattened pose order is traversal order before stable z sorting; match
        // the part's complete path by its ordinal among identical weapon names.
        std::size_t ordinal=0;for(const auto& other:a.weapons){if(&other==&gun)break;if(other.name==gun.name)++ordinal;}
        const PartPose* pose=nullptr;for(const auto& p:poses)if(p.definition&&p.definition->name==gun.name){if(ordinal==0){pose=&p;break;}--ordinal;}
        auto muzzle=pose?pose->muzzle(gun.muzzle):std::nullopt;
        frame.muzzlePosition={a.position.x+(muzzle?muzzle->x:0),a.position.y+(muzzle?muzzle->y:0)};
        const auto fired=gun.controller.tick(*definition,frame,random_,[&](const WeaponFire& shot){
            // Original muzzle flash is allocated before damage/projectile creation.
            if(auto* p=particles_->acquire(float(time()),random_)){
                p->life=.1f;p->tile=int(random_.next()%12);p->position=shot.origin;
                const float angle=float(random_.next())*0x1p-31f*0x1.921fb6p+1f;p->angle=angle+angle;
                setParticleImage(p,"Efx_Fire.png",4,4);p->start(float(time()));
            }
            if(shot.directDamage){
                emitWeaponImpact(*definition,shot.destination,shot.angle,.03f);
                if(auto* p=particles_->acquire(float(time()),random_)){
                    const Vec2 delta{shot.destination.x-shot.origin.x,shot.destination.y-shot.origin.y};
                    p->blend=Blend::additive;p->life=.1f;p->position={shot.origin.x+delta.x*.5f,shot.origin.y+delta.y*.5f};
                    p->angle=shot.angle;p->scale={std::sqrt(delta.x*delta.x+delta.y*delta.y)*.015625f,definition->type==WeaponType::gun?geometry_.deviceScale:1.f};
                    setParticleImage(p,"Efx_BulletTrail.png",1,1);p->start(float(time()));
                }
                if(auto* p=particles_->acquire(float(time()),random_)){
                    const float angleRandom=float(random_.next())*0x1p-31f;
                    const float speedRandom=float(random_.next())*0x1p-31f;
                    const bool left=a.player&&a.type==0&&!(part.reverse&2);
                    const float shellAngle=float(double(gun.controller.angle())+(left?1.5707963267948966:-1.5707963267948966)+double((angleRandom+angleRandom)-1.f));
                    float speed=(speedRandom+speedRandom)+1.5f;
                    const bool sniper=definition->type==WeaponType::sniper;
                    if(!sniper&&geometry_.deviceScale==.5f)speed*=.5f;
                    const float lifeRandom=float(random_.next())*0x1p-31f;
                    const float lifeBase=sniper?.7f:.3f;p->life=lifeRandom*lifeBase+lifeBase;
                    p->position={a.position.x+(pose?pose->localCenter.x:0),a.position.y+(pose?pose->localCenter.y:0)};
                    const float shellScale=sniper?.7f:.6f;p->scale={shellScale,shellScale};p->scaleVelocity=-.004f;p->rotationVelocity=.4f;
                    p->velocity={std::cos(shellAngle)*speed,std::sin(shellAngle)*speed};p->velocityAdd=-.05f;
                    setParticleImage(p,"BulletShell.png",2,1);p->start(float(time()));
                }
                if(target&&target->hp>0)damage(target->id,definition->damage,a.id);
            }
            else {WorldProjectile p{gun.name,Projectile::fire(*definition,shot.origin,shot.destination,shot.angle,time(),random_),a.id,{}};p.motion.toPlayer=!a.player;addProjectile(p.weapon,std::move(p.motion),a.id);}
        });
        if(fired.advanceMuzzle&&pose)gun.muzzle=pose->nextMuzzle(gun.muzzle);
        if(fired.playFireSound)host_.playSound(definition->fireSound);
        gun.recoil=fired.recoil;
    }
}
void World::addProjectile(std::string name,Projectile motion,ActorId attacker,int layer){
    WorldProjectile bullet{std::move(name),std::move(motion),attacker,{},layer,0};
    if(const auto* weapon=weapons_.find(bullet.weapon)){
        const bool launched=bullet.motion.launchVelocity.x!=0||bullet.motion.launchVelocity.y!=0;
        if(auto trail=Trail::forWeapon(*weapon,geometry_.deviceScale,launched?Vec2{}:bullet.motion.position)){
            bullet.trail=std::make_shared<Trail>(std::move(*trail));
            if(!launched)bullet.trail->add(bullet.motion.position,time(),random_);
            trails_.push_back({bullet.trail,layer,nextRenderOrder_++});
        }
    }
    bullet.renderOrder=nextRenderOrder_++;projectiles_.push_back(std::move(bullet));
}
void World::updateProjectiles(){
    for(auto& p:projectiles_){auto* weapon=weapons_.find(p.weapon);if(!weapon||p.motion.removed)continue;
        if(!p.motion.tick(*weapon))continue;
        if(p.trail&&(weapon->effectType==5||weapon->effectType==6))p.trail->add(p.motion.position,time(),random_);
        std::vector<ProjectileTarget> enemies,players;std::vector<ActorId> enemyIds,playerIds;
        for(const auto& a:actors_){auto& snapshots=a->player?players:enemies;auto& ids=a->player?playerIds:enemyIds;snapshots.push_back({a->position,a->removed?0:a->hp,a->radius});ids.push_back(a->id);}
        if(weapon->damageRange>0){
            if(weapon->type==WeaponType::nuclear||weapon->type==WeaponType::emp){
                if(auto* effect=particles_->acquire(float(time()),random_)){
                    effect->life=weapon->type==WeaponType::nuclear?.25f:.15f;effect->position=p.motion.position;
                    effect->scale={.1f,.1f};effect->scaleVelocity=.25f;effect->blend=Blend::additive;
                    setParticleImage(effect,weapon->type==WeaponType::nuclear?"Efx_Nuclear.png":"Efx_EMP.png",1,1);effect->start(float(time()));
                }
                if(weapon->type==WeaponType::emp)if(auto* effect=particles_->acquire(float(time()),random_)){
                    effect->life=16.f*.04f;effect->position=p.motion.position;effect->scale={2,2};effect->blend=Blend::additive;
                    effect->setTileAnimation(16);setParticleImage(effect,"Efx_ExplodeBoss.png",4,4);effect->start(float(time()));
                }
            }
            host_.playSound("SFX_ExplGround.wav");
        }
        const auto hits=projectileCollide(*weapon,p.motion.position,p.motion.toPlayer,time(),enemies,players);
        for(const auto& hit:hits){auto id=(hit.player?playerIds:enemyIds).at(hit.index);damage(id,hit.damage,p.attacker);if(hit.special){auto* a=actor(id);if(a&&a->hp>0&&a->specialDamage.add(*hit.special)){a->target=0;setDestination(*a,a->position);}}if(hit.secondary)if(const auto* a=actor(id))emitSecondaryImpact(*weapon,p.motion.position,a->position);}
        emitWeaponImpact(*weapon,p.motion.position,0,.04f);
        p.motion.removed=true;
    }
    std::erase_if(projectiles_,[](const auto& p){return p.motion.removed;});
}
void World::damage(ActorId id,int value,ActorId attacker){
    auto* a=actor(id);if(!a||a->hp<=0||a->removed)return;
    auto playHit=[&]{
        // GobHvMach::onDamage (0x13fd0): draw after onCrash has finished
        // consuming its explosion, drop and wreck-lifetime random values.
        constexpr std::array sounds{"SFX_HitBody01.wav","SFX_HitBody02.wav","SFX_HitBody03.wav"};
        const auto sound=sounds[random_.next()%sounds.size()];
        // 0x141a2..0x14216: float subtraction, double sqrt/divide, then float.
        // This unusual original expression is not a normalized stereo pan.
        const Vec2 position{float(std::sqrt(double(a->position.x-cameraX()))/10.0),
                            float(std::sqrt(double(a->position.y-actualCamera_))/10.0)};
        host_.playSoundAt(sound,position);
    };
    a->hp=hpAfterDamage(a->hp,value);
    if(a->hp>0){if(const auto* target=actor(a->target);target&&target->base)a->target=attacker;playHit();return;}
    host_.playSound("SFX_ExplMach.wav");emitCrashExplosion(a->position);
    if(!a->player){
        auto drops=enemyLootDrops(a->coin,a->coinRate,upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::creditResearch),a->position,time(),geometry_.deviceScale,random_,[&](std::string_view name)->const LootDefinition*{auto i=lootDefinitions_.find(std::string(name));return i==lootDefinitions_.end()?nullptr:&i->second;});
        for(auto& item:drops)loot_.push_back(std::move(item));
    }
    if(a->player&&a->type==0){
        std::function<void(std::vector<AssemblyPart>&)> removeWeapons=[&](auto& nodes){std::erase_if(nodes,[&](const auto& p){return weapons_.find(p.name)!=nullptr;});for(auto& node:nodes)removeWeapons(node.children);};removeWeapons(a->assembly);
    }else{if(a->assembly.size()>1)a->assembly.resize(1);if(!a->assembly.empty())a->assembly.front().children.clear();}
    for(auto& item:loot_)item.pickerDead(id);
    if(a->base&&state_==WorldState::running)finish(a->player?WorldState::failed:WorldState::cleared);
    ++(a->player?statistics_.playerDestroyed:statistics_.enemyDestroyed);
    events_.push_back({a->player?WorldEvent::Kind::playerDestroyed:WorldEvent::Kind::enemyDestroyed,a->id,a->experience});
    for(auto& b:actors_)if(b->target==id){b->target=0;b->moveToTarget=false;}
    const float life=static_cast<float>(random_.next())*0x1p-31f*2.f;
    a->crashTime=float(time()+3.0+double(life));
    if(!a->player){
        statistics_.score+=a->coin;
        const auto offset=SaveData::slotStart+slot_*SaveData::slotStride+0x2054;
        save_.writeWord(offset,save_.readWord(offset)+unsigned(a->coin));
    }
    playHit();
}

}
