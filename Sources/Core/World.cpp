#include "World.hpp"
#include "Formation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>

namespace md {
namespace {
DataTable table(ResourceAccess& resources,std::string_view name){auto b=resources.read("Tables/"+std::string(name)+".tsv");return DataTable::parse(std::string_view(reinterpret_cast<const char*>(b.data()),b.size()));}
Chunk chunk(ResourceAccess& resources,std::string_view name){return Chunk::parse(resources.read("RawData/"+std::string(name)+".mcb"));}
float distanceSquared(Vec2 a,Vec2 b){float x=a.x-b.x,y=a.y-b.y;return x*x+y*y;}
AssemblyPart& partAt(std::vector<AssemblyPart>& tree,const std::vector<std::size_t>& path){auto* node=&tree.at(path.at(0));for(std::size_t i=1;i<path.size();++i)node=&node->children.at(path[i]);return *node;}
float upgradeParameter(const BaseUpgradeCatalog& catalog,const SaveData& save,std::size_t slot,BaseUpgrade type){auto p=catalog.level(type,save.baseUpgrade(slot,unsigned(type))).parameter;if(!p)throw std::runtime_error("Missing original base upgrade parameter");return *p;}
}
World::World(ResourceAccess& r,HostServices& h,SaveData& s,std::size_t slot,BattleGeometry g,std::uint32_t seed):
 resources_(r),host_(h),save_(s),slot_(slot),geometry_(g),random_(seed),
 machinesTable_(table(r,"MachList")),itemsTable_(table(r,"ItemInfo")),mapTable_(table(r,"MapList")),maps_(chunk(r,"MapInfo")),
 models_(BattleModels::parse(chunk(r,"PartsInfo"),chunk(r,"MachInfo"))),
 weapons_(WeaponCatalog::load(table(r,"WeaponInfo"),g.deviceScale==.5f)),
 builds_(BuildCatalog::load(table(r,"MachBuild_Parts"),table(r,"MachBuild_Name"))),upgrades_(table(r,"BaseUpgrade")),attacks_(table(r,"SpAttackList")){
    uiMargin_=g.deviceScale==.5f?62.f:178.f;
    if(slot>=SaveData::slotCount||!s.active(slot))throw std::invalid_argument("World requires an original active save slot");
    models_.configureTiles(table(r,"PartsList"),table(r,"TileList"));
    particles_=std::make_unique<ParticlePool>(float(time()),random_);
    for(std::size_t i=0;i<itemsTable_.rowCount();++i){auto d=LootDefinition::fromTable(itemsTable_,i);if(d.type!=LootType::other)lootDefinitions_.emplace(d.code,std::move(d));}
}
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
WorldActor& World::addNamed(std::string_view name,Vec2 position,bool player){
    auto row=machinesTable_.findRow(name);if(!row)throw std::runtime_error("Unknown original machine "+std::string(name));
    const auto info=MachineDefinition::fromTable(machinesTable_,*row,geometry_);
    auto value=std::make_unique<WorldActor>();auto& a=*value;
    a.id=nextId_++;a.name=name;a.player=player;a.position=a.destination=position;
    if(!player)a.enemyAI=EnemySteering::begin(time(),random_);
    // makeMachFromName consumes an initial random direction before setDir.
    (void)random_.next();
    a.coin=int(machinesTable_.integer(*row,"Coin").value_or(0));a.coinRate=int(machinesTable_.integer(*row,"Coin%").value_or(0));
    a.hp=a.maxHP=info.hp;a.type=info.type;a.referenceType=info.referenceType;a.experience=info.experience;a.radius=float(info.radius);
    a.angle=player?0x1.921fb6p+0f:-0x1.921fb6p+0f;
    const auto* model=models_.machine(name);if(!model)throw std::runtime_error("Missing original machine assembly");a.assembly=*model;
    a.vehicle.moveSpeed=info.speed;a.aircraft.moveSpeed=info.speed;a.train.moveSpeed=info.speed;
    a.bipod.setStepSize(info.speed);a.bipod.position=a.vehicle.position=a.aircraft.position=a.train.position=position;
    a.bipod.destination=a.vehicle.destination=a.aircraft.destination=a.train.destination=position;
    a.bipod.angle=a.bipod.bodyAngle=a.bipod.baseAngle=a.vehicle.angle=a.aircraft.angle=a.angle;
    configureWeapons(a);actors_.push_back(std::move(value));return a;
}
WorldActor& World::addParts(const BuildDefinition& definition,Vec2 position){
    auto loadout=PlayerLoadout::fromBuild(definition,itemsTable_,geometry_);
    auto value=std::make_unique<WorldActor>();auto& a=*value;
    a.id=nextId_++;a.name=definition.name;a.player=true;a.hp=a.maxHP=loadout.hp;a.radius=loadout.radius;
    a.position=a.destination=position;a.angle=0x1.921fb6p+0f;a.assembly=std::move(loadout.assembly);
    a.bipod.position=a.bipod.destination=position;a.bipod.angle=a.bipod.bodyAngle=a.bipod.baseAngle=a.angle;a.bipod.setStepSize(loadout.stepSize);
    configureWeapons(a);actors_.push_back(std::move(value));return a;
}
void World::start(unsigned stage){
    if(stage>=80||stage>=maps_.children.size()||stage>=mapTable_.rowCount())throw std::out_of_range("Original stage index");
    stage_=stage;startTime_=time();statistics_={};actors_.clear();loot_.clear();projectiles_.clear();trails_.clear();airStrikes_.clear();spawners_.clear();production_.clear();events_.clear();
    for(auto& p:particles_->particles())p.shown=false;
    cellLevel_=0;state_=WorldState::running;paused_=false;started_=true;cameraTouch_.reset();radarTouch_.reset();cameraVelocity_=0;
    save_.writeWord(SaveData::slotStart+slot_*SaveData::slotStride+0x34,stage);
    capacity_=upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::cellStorage);
    buildFactor_=upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::buildTime);
    clock_.start(capacity_,upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::cellResearch));
    baseBoundary_=geometry_.halfLength()-50*geometry_.deviceScale;
    background_=mapTable_.value(stage,"ImageFile");
    char spawn[24];std::snprintf(spawn,sizeof spawn,"ESpawn_%02u",stage+1);
    plan_=StageSpawnPlan::parse(maps_.children.at(stage),table(resources_,spawn),geometry_);
    buildTerrain();
    for(const auto& event:plan_.placed)addNamed(event.name,event.worldPosition,false);
    plan_.appendGeneratedWaves(int(stage),random_,[&](std::string_view n){auto row=machinesTable_.findRow(n);if(!row)throw std::runtime_error("Unknown original wave machine");return MachineDefinition::fromTable(machinesTable_,*row,geometry_).hp;});
    for(const auto& rule:plan_.timed)spawners_.emplace_back(rule,time(),random_);
    director_.emplace(plan_.difficulty,save_.clearCount(slot_,stage));
    auto& enemy=addNamed("E_Base",{0,baseBoundary_},false);enemy.base=true;enemy.type=3;enemy.angle=0;enemy.hp=enemy.maxHP=BattleClock::enemyBaseHP(int(stage));enemyBase_=enemy.id;
    for(auto& w:enemy.weapons)w.controller.setAngle(0);
    auto base=std::make_unique<WorldActor>();auto& a=*base;auto loadout=PlayerLoadout::forBase(save_,slot_,upgrades_,geometry_);
    a.id=nextId_++;a.name="PlayerBase";a.player=a.base=true;a.type=3;a.hp=a.maxHP=loadout.hp;a.radius=loadout.radius;
    a.position=a.destination={0,84*geometry_.deviceScale-geometry_.halfLength()};a.angle=0x1.921fb6p+0f;a.assembly=std::move(loadout.assembly);
    configureWeapons(a);playerBase_=a.id;actors_.push_back(std::move(base));
    camera_=height()*.5f-bottomMargin()-geometry_.halfLength();clampCamera();actualCamera_=camera_;onBottom_=true;
}
WorldActor* World::actor(ActorId id){for(auto& a:actors_)if(a->id==id)return a.get();return nullptr;}
const WorldActor* World::actor(ActorId id)const{for(const auto& a:actors_)if(a->id==id)return a.get();return nullptr;}
void World::setDestination(WorldActor& a,Vec2 point){a.destination=point;a.bipod.setDestination(point);a.vehicle.setDestination(point);a.aircraft.setDestination(point);a.train.setDestination(point);}
bool World::enqueueBuild(std::string_view name,std::optional<Vec2> destination){
    if(!started_||paused_||state_!=WorldState::running)return false;
    const auto owned=save_.builds(slot_);const auto selected=std::find_if(owned.begin(),owned.end(),[&](const auto& b){return b.name==name;});if(selected==owned.end())return false;
    char suffix[16];std::snprintf(suffix,sizeof suffix,"_%02u",selected->level+1);const auto* definition=builds_.find(selected->name+suffix);if(!definition)return false;
    float cells=clock_.cells();bool accepted=production_.enqueue(*definition,time(),buildFactor_,cells,destination);clock_.setCells(cells);
    // BuildItem::init (0x30254) plays this only after a queue item is created.
    if(accepted)host_.playSound("BuildMach.wav");return accepted;
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
        for(const auto& a:actors_)if(a->player&&!a->removed&&a->hp>0&&a->type==0&&a->position.y<point.y){
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
void World::setViewport(float w,float h,float top){
    if(!std::isfinite(w)||!std::isfinite(h)||w<=0||h<=0)return;
    const bool bottom=onBottom_,upper=onTop_;
    viewWidth_=w;viewHeight_=h;safeTop_=top;
    // Fit the original 256-unit battlefield up to the 64-unit production HUD.
    // This changes presentation only; collision, weapons and save use the
    // original BattleGeometry. Phone-width layouts keep scale 1.
    presentationScale_=geometry_.deviceScale==.5f?std::max(1.f,(w-64.f)/256.f):1.f;
    cameraTouch_.reset();radarTouch_.reset();cameraVelocity_=0;
    if(bottom)camera_=height()*.5f-bottomMargin()-geometry_.halfLength();
    else if(upper)camera_=geometry_.halfLength()-height()*.5f;
    clampCamera();actualCamera_=camera_;
}
void World::setUIMargin(float value){
    uiMargin_=value;
    const float lower=height()*.5f-bottomMargin()-geometry_.halfLength();
    if(onBottom_)camera_=lower;
    else if(camera_<lower)onBottom_=true;
}
std::size_t World::productionOccupied(std::string_view baseName)const{
    const auto owned=save_.builds(slot_);const auto i=std::find_if(owned.begin(),owned.end(),[&](const auto& b){return b.name==baseName;});
    if(i==owned.end())return 0;char suffix[16];std::snprintf(suffix,sizeof suffix,"_%02u",i->level+1);return production_.occupied(i->name+suffix);
}
void World::tick(){
    if(!started_)return;
    clock_.tick(true,paused_,int(state_));updateCamera();if(paused_)return;
    if(state_==WorldState::running){
        topPlayerPosition_=-800;
        if(auto generated=director_->tick(time(),random_))spawners_.push_back(std::move(*generated));
        std::vector<SpawnEvent> incoming;
        for(auto& s:spawners_)if(auto e=s.tick(time(),int(state_),geometry_,random_))incoming.push_back(std::move(*e));
        std::erase_if(spawners_,[](const auto& s){return s.finished();});
        for(const auto& e:incoming)addNamed(e.name,e.worldPosition,false);
        production_.tick(time(),geometry_,random_,[&](const ProducedUnit& unit){
            ++statistics_.produced;
            // BuildItem::tick (WeMade 1.0.3, 0x30644) plays sfxID 17 here.
            host_.playSound("BuildMachComplete.wav");
            WorldActor* a;
            if(unit.definition.setType==0)a=&addParts(unit.definition,unit.position);
            else if(unit.definition.placementType==0)a=&addNamed(unit.definition.machine,unit.position,true);
            else {a=&addNamed(unit.definition.bot,unit.position,true);a->postBuild=unit.definition.machine;}
            if(unit.destination)setDestination(*a,*unit.destination);
        });
    }
    // New carriers may create their final unit during this pass; process those
    // children on the following update, keeping actor references stable.
    const auto count=actors_.size();for(std::size_t i=0;i<count;++i)updateActor(*actors_[i]);
    for(auto& strike:airStrikes_)if(const auto* weapon=weapons_.find(strike.weapon))
        if(auto bullet=strike.motion.tick(*weapon,time(),random_))addProjectile(strike.weapon,*bullet,0,0);
    std::erase_if(airStrikes_,[](const auto& strike){return strike.motion.removed;});
    for(auto& trail:trails_)trail.motion->tick(time(),random_);
    std::erase_if(trails_,[](const auto& trail){return trail.motion->removed();});
    updateProjectiles();
    for(auto& p:particles_->particles())p.tick(float(time()),geometry_.deviceScale==.5f?240.f:512.f);
    std::vector<LootCollector> collectors;for(const auto& a:actors_)if(a->player&&!a->removed)collectors.push_back({a->id,a->position,a->type,a->hp});
    for(auto& item:loot_)if(auto pickup=item.tick(time(),collectors,random_)){
        host_.playSound("Game_GetItem.wav");
        if(pickup->type==LootType::coin)save_.setCredits(slot_,save_.credits(slot_)+pickup->value);
        else if(pickup->type==LootType::mineral)clock_.setCells(pickup->applyCells(clock_.cells(),capacity_));
    }
    std::erase_if(loot_,[](const auto& l){return l.removed;});
}
void World::updateActor(WorldActor& a){
    if(a.removed)return;
    if(a.hp<=0){updateCrash(a);return;}
    a.radarAngle+=.2f;if(a.radarAngle>0x1.921fb6p+2f)a.radarAngle-=0x1.921fb6p+2f;
    auto* target=actor(a.target);if(target&&(target->hp<=0||target->removed)){a.target=0;a.moveToTarget=false;target=nullptr;}
    const bool aiEnabled=a.player||!(a.statusFlags&2);
    if(aiEnabled&&(!a.player||a.type==0||a.type==1||a.type==3)&&!target&&time()>a.attackCheck){
        float shortest=a.range*a.range;
        for(const auto& b:actors_)if(b->player!=a.player&&!b->removed&&b->hp>0){float d=distanceSquared(a.position,b->position);if(d<shortest){shortest=d;target=b.get();}}
        if(target){a.target=target->id;a.playerAI.isOrder=false;a.moveToTarget=a.player?target->position.y<baseBoundary_+.8f*a.range:target->position.y>-.8f*a.range-baseBoundary_;}
        a.attackCheck=float(time()+1);
    }
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
    if(target&&time()>double(a.targetCheck)+.5&&!(a.statusFlags&2)){
        if(a.moveToTarget)setDestination(a,distanceSquared(a.position,target->position)<(.8f*a.range)*(.8f*a.range)?a.position:target->position);
        a.targetCheck=float(time());
    }
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
        host_.playSound(sounds[random_.next()%sounds.size()]);
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
void World::finish(WorldState state){
    state_=state;events_.push_back({state==WorldState::cleared?WorldEvent::Kind::stageCleared:WorldEvent::Kind::stageFailed,0,0});
    if(finishHandler_)finishHandler_(state);
    std::vector<ActorId> ids;for(const auto& a:actors_)if(a->player==(state==WorldState::failed)&&a->hp>0)ids.push_back(a->id);
    for(auto id:ids){auto* a=actor(id);damage(id,a->hp);}
    if(state==WorldState::cleared)baseBoundary_=2048;
}
void World::refreshBaseUpgrades(){
    const float baseCapacity=upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::cellStorage);
    capacity_=baseCapacity+float(cellLevel_)*baseCapacity;buildFactor_=upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::buildTime);
    clock_.configure(capacity_,upgradeParameter(upgrades_,save_,slot_,BaseUpgrade::cellResearch));
    auto* a=actor(playerBase_);if(!a)return;auto loadout=PlayerLoadout::forBase(save_,slot_,upgrades_,geometry_);
    if(a->maxHP!=loadout.hp)a->hp=a->maxHP=loadout.hp;
    if(a->weapons.empty()||a->weapons.front().name!=loadout.weapon){
        // DlgUpgradeBase::setUpgrade (0x344cc) replaces only the two guns.
        // setStage (0x1627c) applies body/radar appearances on stage entry.
        auto replaceGuns=[&](auto&& self,std::vector<AssemblyPart>& parts)->void{
            for(auto& part:parts){if(weapons_.find(part.name))part.name=loadout.weapon;self(self,part.children);}
        };
        replaceGuns(replaceGuns,a->assembly);configureWeapons(*a);
    }
}
bool World::upgradeCells(){
    if(!started_||paused_||state_!=WorldState::running||cellLevel_>=5)return false;
    const int cost=cellUpgradeCost();if(clock_.cells()<float(cost))return false;
    clock_.setCells(clock_.cells()-float(cost));++cellLevel_;refreshBaseUpgrades();
    host_.playSound("UpgradeBase.wav");return true;
}
void World::setValidationCells(float value){
    if(!std::isfinite(value)||value<0)throw std::invalid_argument("Invalid validation cell amount");
    clock_.setCells(value);
}
bool World::setValidationHealth(ActorId id,int value){
    auto* a=actor(id);if(!a||a->removed)return false;
    if(value<=0){if(a->hp>0)damage(id,a->hp);}else a->hp=value;
    return true;
}
ActorId World::validationSpawn(std::string_view name,bool player,Vec2 position){
    if(!started_||!std::isfinite(position.x)||!std::isfinite(position.y))throw std::invalid_argument("Invalid validation spawn");
    return addNamed(name,position,player).id;
}
void World::order(unsigned index){
    if(index!=0||paused_||state_!=WorldState::running)return;
    for(auto& a:actors_)if(a->player&&!a->removed)if(auto destination=originalRetreatDestination(index,a->position,a->hp,a->type,topPlayerPosition_)){
        setDestination(*a,*destination);a->moveToTarget=false;a->playerAI.isOrder=true;
    }
}
void World::buildTerrain(){
    terrain_.clear();for(auto& cell:collisionGrid_)cell.clear();
    for(const auto& polygon:plan_.collisionPolygons){
        if(polygon.size()<2)continue;
        const auto first=terrain_.size();
        for(std::size_t i=0;i<polygon.size();++i){auto segment=TerrainSegment::between(polygon[i],polygon[(i+1)%polygon.size()]);if(i)segment.setPrevious(&terrain_.back());terrain_.push_back(segment);}
        // readColBox links each new edge to its preceding edge; the first
        // edge retains its initial corner normal.
        (void)first;
    }
    const float unit=128*geometry_.deviceScale,small=32*geometry_.deviceScale,large=small*3;
    for(std::size_t i=0;i<terrain_.size();++i){const auto& segment=terrain_[i];
        for(int x=0;x<4;++x)for(int y=0;y<16;++y){
            Vec2 origin{x*unit-geometry_.halfLength()*.25f,y*unit-geometry_.halfLength()};
            auto contains=[&](Vec2 p){return p.x>=origin.x-unit*.5f&&p.x<=origin.x+unit*1.5f&&p.y>=origin.y-unit*.5f&&p.y<=origin.y+unit*1.5f;};
            bool include=contains(segment.origin)||contains(segment.second);
            for(Vec2 offset:{Vec2{small,small},Vec2{large,small},Vec2{large,large},Vec2{small,large}})include|=segment.penetration({origin.x+offset.x,origin.y+offset.y},unit)!=0;
            if(include)collisionGrid_[std::size_t(y*4+x)].push_back(i);
        }
    }
}
void World::collideTerrain(WorldActor& a){
    a.collision.reset();const float unit=128*geometry_.deviceScale;
    const int x=int((a.position.x+geometry_.halfLength()*.25f)/unit),y=int((a.position.y+geometry_.halfLength())/unit);
    if(x<0||x>=4||y<0||y>=16)return;
    const auto& cell=collisionGrid_[std::size_t(y*4+x)];
    for(auto i=cell.rbegin();i!=cell.rend();++i){const auto& segment=terrain_[*i];const float penetration=segment.penetration(a.position,16);
        if(penetration>0){a.position.x-=segment.normal.x*penetration;a.position.y-=segment.normal.y*penetration;a.collision=CollisionSurface{segment.normal,segment.direction};}
    }
}
void World::collideGroundBodies(WorldActor& a){
    // Enemy/player overrides (0xdda8/0x10328) visit preceding living
    // same-side ground children, stopping at self so each pair runs once.
    if(a.base||a.type==2||(a.player&&a.type==5))return;
    for(const auto& other:actors_){
        if(other->player!=a.player||other->removed||other->hp<=0||other->base||other->type==2||(a.player&&other->type==5))continue;
        if(other.get()==&a)break;
        separateGroundBodies(a.position,a.radius,a.type==3,
                             other->position,other->radius,other->type==3);
    }
}
void World::clampCamera(){
    onBottom_=onTop_=true;float upper=geometry_.halfLength()-height()*.5f;
    if(camera_<=upper)onTop_=false;else camera_=upper;
    float lower=height()*.5f-bottomMargin()-geometry_.halfLength();
    if(lower>upper){camera_=(lower+upper)*.5f;return;}
    if(camera_<lower)camera_=lower;else onBottom_=false;
}
void World::updateCamera(){
    if(!cameraTouch_&&cameraVelocity_!=0){cameraVelocity_=std::abs(cameraVelocity_)>.01f?cameraVelocity_+(0-cameraVelocity_)/10.f:0;camera_+=cameraVelocity_;clampCamera();}
    if(onBottom_){cameraVelocity_=0;actualCamera_=camera_;}
    else if(actualCamera_!=camera_)actualCamera_=std::abs(actualCamera_-camera_)>.5f?actualCamera_+(camera_-actualCamera_)/5.f:camera_;
}
Vec2 World::worldToView(Vec2 p)const{return {p.x-cameraX()+width()*.5f,height()*.5f-(p.y-actualCamera_)};}
Vec2 World::worldToScreen(Vec2 p)const{const auto v=worldToView(p);return {v.x*presentationScale_,v.y*presentationScale_};}
Vec2 World::screenToWorld(Vec2 p)const{return {cameraX()-width()*.5f+p.x/presentationScale_,actualCamera_+height()*.5f-p.y/presentationScale_};}
std::optional<Vec2> World::touch(const Touch& t){
    if(!started_||paused_||state_!=WorldState::running)return {};
    const float x=t.position.x,y=height()*presentationScale_-t.position.y;
    const bool phone=geometry_.deviceScale==.5f;Vec2 radar=phone?Vec2{18,height()*presentationScale_-82-safeTop_}:Vec2{55,824};Vec2 half=phone?Vec2{16,64}:Vec2{32,128};
    if(t.phase==TouchPhase::began){
        if(std::abs(x-radar.x)<half.x&&std::abs(y-radar.y)<half.y){radarTouch_=t.id;camera_=(y-radar.y)*8;clampCamera();}
        else if(x>=0&&x<(phone?256.f*presentationScale_:640.f)&&y>uiMargin_&&!cameraTouch_){cameraTouch_=t.id;dragY_=y;onlyTap_=true;}
    }else if(radarTouch_==t.id){camera_=(y-radar.y)*8;clampCamera();if(t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled)radarTouch_.reset();}
    else if(cameraTouch_==t.id){
        if(onlyTap_){if(std::abs(y-dragY_)>16)onlyTap_=false;}
        else {cameraVelocity_=(dragY_-y)/presentationScale_;camera_+=cameraVelocity_;clampCamera();dragY_=y;}
        if(t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled){cameraTouch_.reset();if(onlyTap_&&t.phase==TouchPhase::ended)return screenToWorld(t.position);if(std::abs(cameraVelocity_)<8)cameraVelocity_=0;}
    }return {};
}
void World::emitWeaponImpact(const WeaponDefinition& weapon,Vec2 position,float angle,float frameDuration){
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
void World::appendSprites(Frame& frame)const{
    if(!started_)return;
    const auto firstSprite=frame.sprites.size(),firstMesh=frame.meshes.size();
    const float scale=geometry_.deviceScale;
    for(int part=0;part<2;++part){Sprite s;s.image=(scale==.5f?"iPon_":"")+background_+(part==0?"_01.jpg":"_02.jpg");Vec2 center=worldToView({0,float(part==0?-512:512)*scale});s.destination={center.x-256*scale,center.y-512*scale,512*scale,1024*scale};s.blend=Blend::opaque;frame.sprites.push_back(std::move(s));}
    // QobManager::drawVisual visits layers 0..13. Part shadows use layer 0;
    // child parts use the machine layer + 1 (GobHvMach::setLayer 0x14430).
    std::array<Frame,10> layers;
    for(const auto& a:actors_)if(!a->removed){
        const auto poses=models_.pose(a->assembly,scale,a->angle,a->angle,a->radarAngle);
        const int baseLayer=a->hp<=0?1:(a->base?(a->player?7:5):(a->player?3:1));
        for(const auto& pose:poses){Frame part;models_.appendSprites(part,std::span<const PartPose>(&pose,1),worldToView(a->position),[&](std::string_view n){return resources_.imageSize(n);});
            if(part.sprites.size()==2){layers[0].sprites.push_back(std::move(part.sprites[0]));layers[baseLayer+(a->hp>0&&pose.hasParent?1:0)].sprites.push_back(std::move(part.sprites[1]));}
        }
    }
    for(const auto& item:loot_)item.appendSprites(layers[0],worldToView(item.position),[&](std::string_view n){return resources_.imageSize(n);});
    auto particles=particles_->particles();for(std::size_t i=0;i<particles.size();++i){const auto& p=particles[i];const auto& image=particleImages_[i];if(!p.shown||image.image.empty())continue;const float imageScale=image.assetScale?image.assetScale:scale;if(auto s=p.sprite(image.image,resources_.imageSize(image.image),image.columns,image.rows,worldToView({p.position.x-image.origin.x*p.scale.x*imageScale,p.position.y-image.origin.y*p.scale.y*imageScale}),imageScale))layers[5].sprites.push_back(std::move(*s));}
    struct MovingVisual {std::uint64_t order;const WorldTrail* trail=nullptr;const WorldProjectile* projectile=nullptr;};
    std::vector<MovingVisual> moving;for(const auto& trail:trails_)moving.push_back({trail.renderOrder,&trail,nullptr});for(const auto& p:projectiles_)moving.push_back({p.renderOrder,nullptr,&p});
    std::sort(moving.begin(),moving.end(),[](const auto& a,const auto& b){return a.order<b.order;});
    for(const auto& visual:moving){
        if(visual.trail){visual.trail->motion->appendMesh(layers[visual.trail->layer],[&](Vec2 p){return worldToView(p);});continue;}
        const auto& p=*visual.projectile;const auto* w=weapons_.find(p.weapon);if(!w)continue;
        if(!w->bulletImage.empty()){
        const auto size=resources_.imageSize(w->bulletImage);const unsigned columns=w->bulletImage=="Blt_Bullet.png"?4:1,rows=w->bulletImage=="Blt_Bullet.png"?8:1;
        const float cellWidth=size.x/columns,cellHeight=size.y/rows;
        Sprite s;s.image=w->bulletImage;s.sourcePx=Rect{float(unsigned(w->bulletTile)%columns)*cellWidth,float(unsigned(w->bulletTile)/columns)*cellHeight,cellWidth,cellHeight};
        const Vec2 center=worldToView(p.motion.position);const float wpx=cellWidth*scale*p.motion.imageScale.x,hpx=cellHeight*scale*p.motion.imageScale.y;
        s.destination={center.x-wpx*.5f,center.y-hpx*.5f,wpx,hpx};s.rotationDegrees=-p.motion.angle*180.f/float(3.141592653589793);layers[p.layer].sprites.push_back(std::move(s));
        }
        if(!w->effectImage.empty())if(auto effect=p.motion.attachedEffect(*w,resources_.imageSize(w->effectImage),worldToView(p.motion.position),scale))layers[p.layer].sprites.push_back(std::move(*effect));
    }
    for(const auto& strike:airStrikes_)strike.motion.appendSprite(layers[7],worldToView(strike.motion.position),resources_.imageSize("Airplane.png"),scale);
    for(auto& layer:layers){const auto offset=frame.sprites.size();for(auto& mesh:layer.meshes){mesh.beforeSpriteIndex+=offset;frame.meshes.push_back(std::move(mesh));}for(auto& sprite:layer.sprites)frame.sprites.push_back(std::move(sprite));}

    // Transform every world layer together, including shadows and trail meshes.
    if(presentationScale_!=1){
        const float z=presentationScale_;
        auto scaleRect=[&](Rect& r){r.x*=z;r.y*=z;r.width*=z;r.height*=z;};
        for(auto i=firstSprite;i<frame.sprites.size();++i){auto& s=frame.sprites[i];scaleRect(s.destination);if(s.clip)scaleRect(*s.clip);}
        for(auto i=firstMesh;i<frame.meshes.size();++i)for(auto& v:frame.meshes[i].vertices){v.position.x*=z;v.position.y*=z;}
    }

}
std::vector<WorldEvent> World::takeEvents(){auto e=std::move(events_);events_.clear();return e;}
}
