#include "Core/Simulation/AircraftVisual.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include "Core/Simulation/World.hpp"
#include "Core/Simulation/Formation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>
#include <stdexcept>

#include "Core/Simulation/WorldSupport.hpp"
namespace md {
using namespace world_detail;
namespace {
constexpr std::string_view crossfireAttack = "Crossfire-Missile";
constexpr double crossfireReuseSeconds = 10.0;
constexpr double crossfireScanSeconds = 0.25;
constexpr std::string_view su33Missile = "E_Tank20_Wpn";
constexpr std::string_view su33LaserReference = "Turret_lsr_01";
constexpr double battleStep = double(.0166f);
constexpr unsigned su33Missiles = 2;

int distributeRaidDamage(int total, unsigned index, unsigned count) {
    return total/int(count)+(index<unsigned(total%int(count)));
}

Vec2 su33Muzzle(const AirStrike& motion, bool left) {
    // Wing sockets on the 256px right-facing sprite. The world y axis points up.
    const float spriteScale=1.3f*motion.deviceScale*.5f;
    const float forward=-16* spriteScale, lateral=(left ? 38.f : -38.f)*spriteScale;
    const float x=std::cos(motion.angle), y=std::sin(motion.angle);
    return {motion.position.x+x*forward-y*lateral, motion.position.y+y*forward+x*lateral};
}

double su33SafeGap(const AirStrike& motion, float distance, float clearance,
                   float helicopterSpeed, unsigned remaining, double normal) {
    const auto displacement=[&](double ticks) {
        return std::hypot(motion.velocity.x,motion.velocity.y)*std::expm1(ticks*std::log1p(.01))/.01+
               helicopterSpeed*ticks;
    };
    const double space=std::max(0.f,distance-clearance);
    unsigned low=0, high=unsigned(std::ceil(remaining*normal/battleStep))+2;
    if(displacement(high)<space)return normal;
    while(high-low>1) {
        const auto middle=low+(high-low)/2;
        if(displacement(middle)<space)low=middle;
        else high=middle;
    }
    return std::clamp(low*battleStep/remaining,battleStep,normal);
}

std::optional<float> crossfireDistance(const WorldActor& shooter,
                                       const WeaponDefinition& weapon, Vec2 point) {
    if (!shooter.player || shooter.removed || shooter.hp<=0 ||
        !(shooter.type==0 || shooter.flightTactics) || shooter.position.y>=point.y) return {};
    const float distance=distanceSquared(shooter.position,point);
    if (distance>10000.f && distance<=float(weapon.shotRange*weapon.shotRange)) return distance;
    return {};
}
}

void World::configureWeapons(WorldActor& actor){
    actor.weapons.clear();actor.range=0;
    std::function<void(std::vector<AssemblyPart>&,std::vector<std::size_t>)> walk=[&](auto& nodes,auto path){
        for(std::size_t i=0;i<nodes.size();++i){auto p=path;p.push_back(i);auto& part=nodes[i];
            if(const auto* weapon=weapons_.find(part.name)){
                WorldWeapon w;w.name=part.name;w.partPath=p;w.controller=WeaponController(float(time()));w.controller.setAngle(actor.angle);
                actor.weapons.push_back(std::move(w));actor.range=actor.flightTactics?std::max(actor.range,float(weapon->shotRange)):float(weapon->shotRange);
            }
            walk(part.children,p);
        }
    };walk(actor.assembly,{});
}
AirStrike World::beginAirStrike(const WeaponDefinition& weapon,Vec2 point,float delay,bool enemy) {
    auto marker = [&]() {
        auto* particle = particles_->acquire(float(time()),random_);
        if (particle) {
            if (enemy) setParticleImage(particle,enemyRaidReticleImage,1,1);
            else setParticleImage(particle,"Target.png",2,1);
        }
        return particle;
    };
    auto strike = AirStrike::start(weapon,point,delay,time(),geometry_.deviceScale,random_,marker,float(time()),enemy);
    host_.playSound("SFX_AirStrike.wav");
    return strike;
}
void World::updateEnemyAirRaid() {
    if (!scheduledEnemyRaid_ || !scheduledEnemyRaid_->schedule.takeDue(time()-startTime_)) return;
    const WorldActor* target = nullptr;
    const WorldActor* helicopter = nullptr;
    for (const auto& actor : actors_) {
        if (!actor->player || actor->hp <= 0 || actor->removed) continue;
        if (actor->type==0 && (!target || actor->position.y > target->position.y)) target=actor.get();
        if (actor->flightTactics && (!helicopter || actor->position.y > helicopter->position.y)) helicopter=actor.get();
    }
    if (target) {
        constexpr std::string_view bomb = "AS_BOMB_01";
        const auto* weapon = weapons_.find(bomb);
        if (!weapon || weapon->type != WeaponType::bomb || weapon->damageRange <= 0 || !weapon->shotCount)
            throw std::runtime_error("Enemy air raid requires the area-damage airstrike bomb");
        constexpr float warningSeconds = 1.5f;
        airStrikes_.push_back({std::string(bomb),beginAirStrike(*weapon,target->position,warningSeconds,true),scheduledEnemyRaid_->raid});
    }
    if (helicopter && su33Budget_>0) beginSu33Raid(*helicopter);
}

int World::su33StageBudget(unsigned stage) const {
    constexpr unsigned firstStage=10;
    if(stage<firstStage)return 0;
    int minimum=std::numeric_limits<int>::max();
    for(const auto& entry:alliedAircraftCatalog) {
        if(entry.mission>stage)continue;
        for(unsigned level=1;const auto* build=builds_.find(specialWeaponName(entry.name,level));++level) {
            const auto row=machinesTable_.findRow(build->machine);
            if(!row)throw std::runtime_error("Missing allied helicopter data for Su-33 damage");
            minimum=std::min(minimum,MachineDefinition::fromTable(machinesTable_,*row,geometry_).hp);
        }
    }
    if(minimum==std::numeric_limits<int>::max())return 0;
    const auto cap=std::int64_t(minimum)*180/100;
    const auto span=mapTable_.rowCount()-1-firstStage;
    return int(span ? cap*(span+stage-firstStage)/(2*span) : cap);
}

void World::beginSu33Raid(const WorldActor& target,bool trace) {
    const auto* laser=weapons_.find(su33LaserReference);
    if(!laser||laser->type!=WeaponType::gun||!laser->shotCount||laser->shotDelay<=0)
        throw std::runtime_error("Missing laser turret salvo reference for Su-33");
    const auto marker=[&] {
        auto* particle=particles_->acquire(float(time()),su33Random_);
        if(particle)setParticleImage(particle,enemyRaidReticleImage,1,1);
        return particle;
    };
    auto motion=AirStrike::start(*laser,target.position,0,time(),geometry_.deviceScale,
                                  su33Random_,marker,float(time()),true);
    su33Raids_.push_back({motion,target.id,target.position,time(),time()+.28,0,su33Budget_,0,0,trace});
    host_.playSound("SFX_AirStrike.wav");
    if(trace) {
        su33Trace_={};
        su33Trace_.active=true;
        su33Trace_.budget=su33Budget_;
    }
}

bool World::validationSu33Start(ActorId id) {
    const auto* target=actor(id);
    if(!started_||state_!=WorldState::running||!su33Budget_||!target||!target->player||
       !target->flightTactics||target->removed||target->hp<=0)return false;
    beginSu33Raid(*target,true);
    return true;
}

void World::fireSu33Laser(Vec2 muzzle,Vec2 destination,const WeaponDefinition& laser) {
    const Vec2 delta{destination.x-muzzle.x,destination.y-muzzle.y};
    const float angle=std::atan2(delta.y,delta.x);
    const auto acquired=[&]{return particles_->acquire(float(time()),su33Random_);};
    if(auto* flash=acquired()) {
        flash->life=.1f;
        flash->tile=int(su33Random_.next()%12);
        flash->position=muzzle;
        flash->angle=angle;
        setParticleImage(flash,"Efx_Fire.png",4,4);
        flash->start(float(time()));
    }
    if(auto* beam=acquired()) {
        const auto width=resources_.imageSize("Efx_Ion03.png").x;
        beam->blend=Blend::additive;
        beam->life=.085f;
        beam->position={muzzle.x+delta.x*.5f,muzzle.y+delta.y*.5f};
        beam->angle=angle;
        beam->scale={std::hypot(delta.x,delta.y)/std::max(1.f,width*geometry_.deviceScale),.24f};
        setParticleImage(beam,"Efx_Ion03.png",1,1);
        beam->start(float(time()));
    }
    emitWeaponImpact(laser,destination,angle,.03f);
    host_.playSound(laser.fireSound);
}

void World::tickSu33Raids() {
    if(su33Raids_.empty())return;
    const auto* missile=weapons_.find(su33Missile);
    const auto* laser=weapons_.find(su33LaserReference);
    if(!missile||missile->type!=WeaponType::bullet||missile->damageRange<=0||!laser)
        throw std::runtime_error("Missing Su-33 attack assets");
    const double normal=double(laser->shotDelay)/1.3;
    const float clearance=10.f/presentationScale_;
    for(auto& raid:su33Raids_) {
        auto* target=actor(raid.target);
        const bool alive=target&&target->hp>0&&!target->removed;
        if(alive)raid.aim=target->position;
        auto& motion=raid.motion;
        const Vec2 previous=motion.position;
        motion.advance();
        if(raid.lasers<laser->shotCount && alive) {
            const Vec2 segment{motion.position.x-previous.x,motion.position.y-previous.y};
            const float lengthSquared=distanceSquared(motion.position,previous);
            const float along=lengthSquared>0 ? std::clamp(
                ((raid.aim.x-previous.x)*segment.x+(raid.aim.y-previous.y)*segment.y)/lengthSquared,0.f,1.f) : 0.f;
            if(distanceSquared({previous.x+segment.x*along,previous.y+segment.y*along},raid.aim)<=clearance*clearance) {
                // A sudden turn or resize can invalidate the estimated firing window.
                const Vec2 offset{previous.x-raid.aim.x,previous.y-raid.aim.y};
                const float length=std::hypot(offset.x,offset.y);
                const float distance=std::max(length,clearance+1.f);
                motion.position=length>0 ? Vec2{raid.aim.x+offset.x*distance/length,raid.aim.y+offset.y*distance/length}
                    : Vec2{raid.aim.x-std::cos(motion.angle)*distance,raid.aim.y-std::sin(motion.angle)*distance};
                if(raid.traced)++su33Trace_.distanceAdjustments;
            }
        }
        const float distance=std::sqrt(distanceSquared(motion.position,raid.aim));
        if(raid.traced&&raid.lasers<laser->shotCount) {
            const float pixels=distance*presentationScale_;
            su33Trace_.closestWhileFiring=std::min(su33Trace_.closestWhileFiring,pixels);
            su33Trace_.earlyClose|=alive&&pixels<=10.f;
        }
        const int laserBudget=raid.budget/2, missileBudget=raid.budget-laserBudget;
        if(raid.missiles<su33Missiles && time()>=raid.enteredAt+.08*(raid.missiles+1)) {
            const auto muzzle=su33Muzzle(motion,raid.missiles==0);
            const float angle=std::atan2(raid.aim.y-muzzle.y,raid.aim.x-muzzle.x);
            auto shot=Projectile::fire(*missile,muzzle,raid.aim,angle,time(),su33Random_);
            shot.toPlayer=true;
            const int value=distributeRaidDamage(missileBudget,raid.missiles++,su33Missiles);
            addProjectile(missile->name,shot,0,5,value);
            host_.playSound(missile->fireSound);
            if(raid.traced){++su33Trace_.missiles;su33Trace_.scheduledDamage+=value;}
        }
        const unsigned remaining=laser->shotCount-raid.lasers;
        if(remaining>0) {
            const float helicopterSpeed=alive?std::max(target->aircraft.speed,target->aircraft.moveSpeed):0.f;
            const double gap=su33SafeGap(motion,distance,clearance,helicopterSpeed,remaining,normal);
            if(time()>=raid.nextLaserAt || (raid.lasers>0 && gap<normal && time()-raid.lastLaserAt+.000001>=gap)) {
                const bool left=raid.lasers%2==0;
                const int value=distributeRaidDamage(laserBudget,raid.lasers++,laser->shotCount);
                fireSu33Laser(su33Muzzle(motion,left),raid.aim,*laser);
                const int hp=alive ? target->hp : 0;
                if(alive)damage(raid.target,value);
                if(raid.traced) {
                    auto& trace=su33Trace_;
                    if(trace.laserShots==0)trace.firstLaserAt=time();
                    trace.lastLaserAt=time();
                    ++trace.laserShots;
                    ++(left?trace.leftShots:trace.rightShots);
                    trace.wingSequence+=left?'L':'R';
                    trace.scheduledDamage+=value;
                    if(alive)trace.appliedLaserDamage+=hp-target->hp;
                }
                raid.lastLaserAt=time();
                raid.nextLaserAt=std::min(raid.nextLaserAt,time())+normal;
            }
        }
        if(raid.lasers==laser->shotCount && raid.missiles==su33Missiles && time()>raid.enteredAt+4.0) {
            motion.removed=true;
            if(raid.traced){su33Trace_.active=false;su33Trace_.exited=true;}
        }
    }
    std::erase_if(su33Raids_,[](const auto& raid){return raid.motion.removed;});
}
World::SpecialWeapon World::specialWeapon(const OwnedAttack& owned)const {
    const auto& attack=attacks_.definition(owned);
    char code[16];
    std::snprintf(code,sizeof code,"%05d",attack.itemId);
    const auto row=itemsTable_.findRow(code);
    if (!row) return {};
    const auto prefix=itemsTable_.value(*row,"strParam");
    return {weapons_.find(specialWeaponName(prefix,itemsTable_.number(*row,"Param1").value_or(0))),
            itemsTable_.number(*row,"Param2").value_or(0)};
}
void World::fireCrossfire(const WeaponDefinition& weapon, const WorldActor& shooter, Vec2 point) {
    auto bullets=crossfireProjectiles(weapon,shooter.position,point,shooter.angle,time(),random_);
    for (auto& bullet:bullets) addProjectile(weapon.name,std::move(bullet),shooter.id,1);
}
bool World::activateSpecial(std::string_view name,Vec2 point){
    if(!started_||paused_||state_!=WorldState::running)return false;
    auto owned=save_.attacks(slot_);auto selected=std::find_if(owned.begin(),owned.end(),[&](const auto& a){return a.name==name;});
    if(selected==owned.end()||selected->count==0)return false;
    const auto [weapon,delay]=specialWeapon(*selected);if(!weapon)return false;
    auto acquireMarker=[&](){auto* p=particles_->acquire(float(time()),random_);if(p)setParticleImage(p,"Target.png",2,1);return p;};
    if(weapon->name.starts_with("AS_")){
        auto motion=beginAirStrike(*weapon,point,delay,false);
        airStrikes_.push_back({weapon->name,std::move(motion)});
    }else if(weapon->name.starts_with("CF_")){
        unsigned fired=0;
        for(const auto& a:actors_){
            if(crossfireDistance(*a,*weapon,point)){
                fireCrossfire(*weapon,*a,point);
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

void World::refreshCrossfireLockWeapon(std::span<const OwnedAttack> attacks) {
    const auto owned=std::find_if(attacks.begin(),attacks.end(),[](const auto& attack) {
        return attack.name==crossfireAttack;
    });
    const auto level = owned==attacks.end() ? std::nullopt : std::optional{owned->level};
    if (level==crossfireLockLevel_) return;
    crossfireLockLevel_=level;
    crossfireLockWeapon_=level ? specialWeapon(*owned).definition : nullptr;
    crossfireLockTarget_=0;
    nextCrossfireScan_=time();
}

WorldActor* World::crossfireShooter(Vec2 point)const {
    WorldActor* nearest=nullptr;
    float closest=0;
    for (const auto& ally:actors_) {
        if (ally->type!=0 || time()<ally->crossfireReadyAt) continue;
        if (const auto distance=crossfireDistance(*ally,*crossfireLockWeapon_,point);
            distance && (!nearest || *distance<closest)) {
            nearest=ally.get();
            closest=*distance;
        }
    }
    return nearest;
}

void World::updateCrossfireLock() {
    if (state_!=WorldState::running || !crossfireLockWeapon_ || validationArtScene_) return;
    if (const auto* locked=crossfireLockTarget_ ? actor(crossfireLockTarget_) : nullptr;
        locked && crossfireTargetVisible(*locked) && crossfireShooter(locked->position)) return;
    crossfireLockTarget_=0;
    if (time()<nextCrossfireScan_) return;
    nextCrossfireScan_=time()+crossfireScanSeconds;
    const bool ready=std::any_of(actors_.begin(),actors_.end(),[&](const auto& ally) {
        return ally->player && ally->type==0 && !ally->removed && ally->hp>0 &&
               time()>=ally->crossfireReadyAt;
    });
    if (!ready) return;
    for (const auto& enemy:actors_) {
        if (crossfireTargetVisible(*enemy) && crossfireShooter(enemy->position)) {
            crossfireLockTarget_=enemy->id;
            crossfireLockAcquiredAt_=time();
            return;
        }
    }
}

bool World::tapCrossfireLock(Vec2 point) {
    if (!started_ || paused_ || state_!=WorldState::running || !crossfireLockWeapon_ ||
        !crossfireTouchTarget_ || crossfireTouchTarget_!=crossfireLockTarget_ ||
        !crossfireLockContains(point)) return false;
    crossfireTouchTarget_=0;
    const auto* enemy=actor(crossfireLockTarget_);
    auto* shooter=crossfireShooter(enemy->position);
    if (!shooter) {
        updateCrossfireLock();
        return false;
    }
    fireCrossfire(*crossfireLockWeapon_,*shooter,enemy->position);
    shooter->crossfireReadyAt=time()+crossfireReuseSeconds;
    host_.playSound("SFX_AirStrike.wav");
    updateCrossfireLock();
    return true;
}
void World::updateWeapons(WorldActor& a){
    auto* target=actor(a.target);
    for(auto& gun:a.weapons){const auto* definition=weapons_.find(gun.name);if(!definition)continue;
        auto& part=partAt(a.assembly,gun.partPath);
        WeaponFrame frame;frame.time=time();frame.ownerPosition=a.position;frame.bodyAngle=a.angle;frame.deviceScale=geometry_.deviceScale;
        if(a.flightTactics){
            frame.maxAimStep=a.flightTactics->weaponStep;
            // Keep the added Cheyenne's zero-delay salvo on the controller's float clock.
            if(gun.name.starts_with("ADuctMissile")&&definition->shotDelay==0.f)
                frame.time=double(float(frame.time));
        }
        frame.ownerHP=a.hp;frame.machineType=a.type;frame.statusFlags=a.statusFlags;frame.weaponReady=a.type!=0||a.bipod.weaponReady;
        if(target)frame.target=WeaponTarget{target->position,target->hp};
        frame.weaponReady=gun.controller.track(frame);
        if(a.type==0){gun.controller.setAngle(part.reverse&2?a.bipod.rightWeaponAngle:a.bipod.leftWeaponAngle);}
        const bool embedded=gun.name.starts_with("AHeavyRocket")||gun.name.starts_with("AHeavyAirStrike");
        part.angle=embedded?a.angle:gun.controller.angle();part.addPosition.x=embedded?0:gun.recoil;if(a.type==0)part.addPosition.y=a.bipod.bodyAddPosY;
        const auto poses=models_.pose(a.assembly,geometry_.deviceScale,a.angle,gun.controller.angle(),a.radarAngle);
        // Flattened pose order is traversal order before stable z sorting; match
        // the part's complete path by its ordinal among identical weapon names.
        std::size_t ordinal=0;for(const auto& other:a.weapons){if(&other==&gun)break;if(other.name==gun.name)++ordinal;}
        const PartPose* pose=nullptr;for(const auto& p:poses)if(p.definition&&p.definition->name==gun.name){if(ordinal==0){pose=&p;break;}--ordinal;}
        auto muzzle=pose?pose->muzzle(gun.muzzle):std::nullopt;
        if(muzzle&&a.flightTactics)*muzzle=aircraftBankPoint(*muzzle,a.angle,a.flightTactics->bank);
        frame.muzzlePosition={a.position.x+(muzzle?muzzle->x:0),a.position.y+(muzzle?muzzle->y:0)};
        unsigned salvoIndex=0;
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
                    auto eject=pose?pose->localCenter:Vec2{};
                    if(a.flightTactics)eject=aircraftBankPoint(eject,a.angle,a.flightTactics->bank);
                    p->position={a.position.x+eject.x,a.position.y+eject.y};
                    const float shellScale=sniper?.7f:.6f;p->scale={shellScale,shellScale};p->scaleVelocity=-.004f;p->rotationVelocity=.4f;
                    p->velocity={std::cos(shellAngle)*speed,std::sin(shellAngle)*speed};p->velocityAdd=-.05f;
                    setParticleImage(p,"BulletShell.png",2,1);p->start(float(time()));
                }
                if(target&&target->hp>0)damage(target->id,definition->damage,a.id);
            }
            else {
                auto destination=shot.destination;
                if(target&&gun.name.starts_with("ADuctMissile")){
                    destination=target->position;
                    // Left pod: center + 3. Right pod: 3. One simultaneous seven-missile fan.
                    if(gun.name.starts_with("ADuctMissileR")||salvoIndex>0){
                        const float theta=float(random_.next())*0x1p-31f*6.28318530718f;
                        const float radius=float(definition->damageRange)*.5f;
                        destination.x+=std::cos(theta)*radius;destination.y+=std::sin(theta)*radius;
                    }
                }
                const float angle=gun.name.starts_with("ADuctMissile")?float(std::atan2(double(destination.y-shot.origin.y),double(destination.x-shot.origin.x))):shot.angle;
                WorldProjectile p{gun.name,Projectile::fire(*definition,shot.origin,destination,angle,time(),random_),a.id,{}};
                p.motion.toPlayer=!a.player;addProjectile(p.weapon,std::move(p.motion),a.id);
            }
            ++salvoIndex;
        });
        if(fired.advanceMuzzle&&pose)gun.muzzle=pose->nextMuzzle(gun.muzzle);
        if(fired.playFireSound)host_.playSound(definition->fireSound);
        gun.recoil=fired.recoil;
    }
}
void World::addProjectile(std::string name,Projectile motion,ActorId attacker,int layer,std::optional<int> damageOverride){
    WorldProjectile bullet{std::move(name),std::move(motion),attacker,{},layer,0,damageOverride};
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
        const auto hits=projectileCollide(*weapon,p.motion.position,p.motion.toPlayer,time(),enemies,players,p.damageOverride);
        for(const auto& hit:hits){auto id=(hit.player?playerIds:enemyIds).at(hit.index);damage(id,hit.damage,p.attacker);if(hit.special){auto* a=actor(id);if(a&&a->hp>0&&!(a->flightTactics&&a->flightTactics->invulnerable(time()))&&a->specialDamage.add(*hit.special)){a->target=0;setDestination(*a,a->position);}}if(hit.secondary)if(const auto* a=actor(id))emitSecondaryImpact(*weapon,p.motion.position,a->position);}
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
    // Incoming hits retain their visual/audio impact; only HP is protected.
    // Latest user rule includes hitscan. Original actors have no flightTactics.
    if(a->flightTactics&&a->flightTactics->invulnerable(time())){playHit();return;}
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
