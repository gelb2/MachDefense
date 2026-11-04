#pragma once
#include "Bipod.hpp"
#include "Game.hpp"
#include "PlayerParts.hpp"
#include "Loot.hpp"
#include "Projectile.hpp"
#include "SpecialAttack.hpp"
#include "Trail.hpp"
#include "WeaponController.hpp"
#include <memory>

namespace md {
using ActorId=std::uint64_t;
enum class WorldState:int { running=0,cleared=1,failed=2 };
struct WorldWeapon {
    std::string name; std::vector<std::size_t> partPath;
    WeaponController controller; unsigned muzzle=1; float recoil=0;
};
struct WorldActor {
    ActorId id=0; std::string name,postBuild;
    bool player=false,base=false,removed=false,moveToTarget=false;
    int hp=0,maxHP=0,type=0,referenceType=0,experience=0,coin=0,coinRate=0;
    float radius=0,range=0,angle=0,radarAngle=0,attackCheck=0,targetCheck=0,crashTime=0;
    std::uint32_t statusFlags=0;
    Vec2 position,destination;
    ActorId target=0;
    BipodMotion bipod; VehicleMotion vehicle; AircraftMotion aircraft; TrainMotion train;
    SpecialDamageController specialDamage;
    float collisionCheck=0; std::optional<CollisionSurface> collision;
    EnemySteering enemyAI; PlayerSteering playerAI;
    std::vector<AssemblyPart> assembly; std::vector<WorldWeapon> weapons;
};
struct WorldProjectile { std::string weapon; Projectile motion; ActorId attacker=0; std::shared_ptr<Trail> trail; int layer=5;std::uint64_t renderOrder=0; };
struct WorldStatistics { unsigned enemyDestroyed=0,playerDestroyed=0,produced=0;int score=0; };
struct WorldEvent {
    enum class Kind { enemyDestroyed,playerDestroyed,stageCleared,stageFailed } kind;
    ActorId actor=0; int experience=0;
};
// GobWorld orchestration; save mutations are in memory. GameSession owns the
// original explicit save/summary/menu points and first-use help dialog.
class World {
public:
    World(ResourceAccess&,HostServices&,SaveData&,std::size_t slot,
          BattleGeometry=BattleGeometry::phone(),std::uint32_t seed=1);
    void start(unsigned stageIndex);
    void tick();
    void setPaused(bool value){paused_=value;}
    bool paused()const{return paused_;}
    WorldState state()const{return state_;}
    unsigned stage()const{return stage_;}
    double time()const{return clock_.time();}
    double startTime()const{return startTime_;}
    DarwinRandom& randomSource(){return random_;}
    const WorldStatistics& statistics()const{return statistics_;}
    float cells()const{return clock_.cells();}
    float cellCapacity()const{return capacity_;}
    int score()const{return statistics_.score;}
    const std::vector<LootState>& loot()const{return loot_;}
    std::size_t productionCount()const{return production_.size();}
    std::vector<ProductionProgress> productionRows()const{return production_.rows(time());}
    std::size_t productionOccupied(std::string_view baseName)const;
    bool enqueueBuild(std::string_view name,std::optional<Vec2> destination={});
    void order(unsigned orderIndex);
    // Called only by the validation application; these mutate memory, never files.
    void setValidationCells(float);
    bool setValidationHealth(ActorId,int);
    ActorId validationSpawn(std::string_view machineName,bool player,Vec2 position);
    bool activateSpecial(std::string_view ownedBaseName,Vec2 worldPoint);
    void setUIMargin(float);
    void setViewport(float width,float height,float safeTop=0);
    // Returns a world-space tap only after the original camera tap/drag test.
    std::optional<Vec2> touch(const Touch&);
    Vec2 worldToScreen(Vec2)const;
    Vec2 screenToWorld(Vec2)const;
    float cameraY()const{return actualCamera_;}
    float cameraTargetY()const{return camera_;}
    Vec2 cameraVisibleY()const{return {camera_-height()*.5f+bottomMargin(),camera_+height()*.5f};}
    void prepareDraw();
    void appendSprites(Frame&)const;
    const std::vector<std::unique_ptr<WorldActor>>& actors()const{return actors_;}
    const std::vector<WorldProjectile>& projectiles()const{return projectiles_;}
    std::vector<WorldEvent> takeEvents();
    WorldActor* actor(ActorId);
    const WorldActor* actor(ActorId)const;
    void damage(ActorId target,int damage,ActorId attacker=0);
    void refreshBaseUpgrades();
    bool upgradeCells();
    unsigned cellLevel()const{return cellLevel_;}
    int cellUpgradeCost()const{return static_cast<int>(capacity_*.6f);}
    void setFinishHandler(std::function<void(WorldState)> handler){finishHandler_=std::move(handler);}
private:
    ResourceAccess& resources_;HostServices& host_;SaveData& save_;std::size_t slot_;
    BattleGeometry geometry_; DarwinRandom random_;
    DataTable machinesTable_,itemsTable_,mapTable_;
    Chunk maps_;BattleModels models_;WeaponCatalog weapons_;BuildCatalog builds_;BaseUpgradeCatalog upgrades_;AttackCatalog attacks_;
    BattleClock clock_;ProductionQueue production_;std::optional<EnemyDirector> director_;
    std::vector<EnemySpawner> spawners_;StageSpawnPlan plan_;
    std::vector<TerrainSegment> terrain_;std::array<std::vector<std::size_t>,64> collisionGrid_;
    std::vector<std::unique_ptr<WorldActor>> actors_;
    struct WorldAirStrike {std::string weapon;AirStrike motion;};
    std::vector<WorldAirStrike> airStrikes_;
    struct WorldTrail {std::shared_ptr<Trail> motion;int layer;std::uint64_t renderOrder;};
    std::vector<WorldTrail> trails_;std::uint64_t nextRenderOrder_=0;
    void addProjectile(std::string,Projectile,ActorId,int layer=5);
    std::vector<WorldProjectile> projectiles_;std::vector<WorldEvent> events_;
    std::unordered_map<std::string,LootDefinition> lootDefinitions_;std::vector<LootState> loot_;
    std::unique_ptr<ParticlePool> particles_;
    struct ParticleImage {std::string image;unsigned columns=1,rows=1;Vec2 origin;float assetScale=0;};
    std::array<ParticleImage,ParticlePool::capacity> particleImages_;
    void setParticleImage(ParticleState*,std::string,unsigned,unsigned);
    void emitWeaponImpact(const WeaponDefinition&,Vec2,float angle,float frameDuration);
    void emitCrashExplosion(Vec2);
    void emitSecondaryImpact(const WeaponDefinition&,Vec2 from,Vec2 to);
    void updateCrash(WorldActor&);
    void emitDamageSmoke(const WorldActor&,bool crashed);
    ActorId nextId_=1,playerBase_=0,enemyBase_=0;
    double startTime_=0;WorldStatistics statistics_;
    std::function<void(WorldState)> finishHandler_;
    unsigned cellLevel_=0;
    unsigned stage_=0;WorldState state_=WorldState::running;
    bool started_=false,paused_=false;
    float capacity_=0,buildFactor_=1,baseBoundary_=0,topPlayerPosition_=-800;
    float viewWidth_=0,viewHeight_=0,safeTop_=0,presentationScale_=1;
    float camera_=0,actualCamera_=0,cameraVelocity_=0,dragY_=0,uiMargin_=0;
    bool onBottom_=true,onTop_=false,onlyTap_=false;
    std::optional<std::uint64_t> cameraTouch_,radarTouch_;
    std::string background_;
    WorldActor& addNamed(std::string_view,Vec2,bool player);
    WorldActor& addParts(const BuildDefinition&,Vec2);
    void configureWeapons(WorldActor&);
    void setDestination(WorldActor&,Vec2);
    void updateActor(WorldActor&);
    void updateWeapons(WorldActor&);
    void updateProjectiles();
    void buildTerrain();void collideTerrain(WorldActor&);void collideGroundBodies(WorldActor&);
    void updateCamera();void clampCamera();
    void finish(WorldState);
    float width()const{return viewWidth_?viewWidth_/presentationScale_:(geometry_.deviceScale==.5f?320.f:768.f);}
    float height()const{return viewHeight_?viewHeight_/presentationScale_:(geometry_.deviceScale==.5f?480.f:1024.f);}
    float cameraX()const{return geometry_.deviceScale==.5f?32.f+(width()-320.f)*.5f:0.f;}
    float bottomMargin()const{return uiMargin_/presentationScale_;}
    Vec2 worldToView(Vec2)const;
};
}
