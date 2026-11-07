#pragma once
#include "Core/Data/Chunk.hpp"
#include "Core/Contracts/DarwinRandom.hpp"
#include "Core/Data/DataTable.hpp"
#include <functional>
#include "Core/Contracts/RenderTypes.hpp"
#include <optional>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace md {
class ResourceAccess;
// Original GobWorld/GobEnemySpawner/BuildItem rules. Combat controllers are
// reconstructed separately; this class never substitutes invented combat.
struct BattleGeometry {
    float deviceScale=.5f, mapLength=1024;
    [[nodiscard]] float halfLength() const { return mapLength*.5f; }
    static BattleGeometry phone() { return {}; }
    static BattleGeometry pad() { return {1,2048}; }
};
struct SpawnRule { float startTime=0; int cumulativeRate=0; std::string name; int count=1; float delay=0; };
struct SpawnEvent { std::string name; Vec2 worldPosition; };
class EnemySpawner {
public:
    EnemySpawner(SpawnRule rule,double worldTime,DarwinRandom& random);
    std::optional<SpawnEvent> tick(double time,int gameState,BattleGeometry geometry,DarwinRandom& random);
    [[nodiscard]] bool finished() const { return generated_>=rule_.count; }
    [[nodiscard]] int targetCount() const { return rule_.count; }
    [[nodiscard]] float nextSpawnTime() const { return nextTime_; }
private:
    SpawnRule rule_; int generated_=0; float nextTime_=0;
};
struct StageSpawnPlan {
    std::array<std::vector<SpawnRule>,5> difficulty;
    std::vector<SpawnRule> timed;
    std::vector<SpawnEvent> placed;
    std::vector<std::vector<Vec2>> collisionPolygons;
    std::vector<std::string> enemyNames;
    static StageSpawnPlan parse(const Chunk& map,const DataTable& spawnTable,BattleGeometry geometry);
    // Call after placed enemies are instantiated, preserving original RNG order.
    void appendGeneratedWaves(int stageIndex,DarwinRandom& random,const std::function<int(std::string_view)>& hp);
};
class EnemyDirector {
public:
    explicit EnemyDirector(std::array<std::vector<SpawnRule>,5> rules,int stageClearCount=0);
    std::optional<EnemySpawner> tick(double time,DarwinRandom& random);
    [[nodiscard]] float difficulty() const { return difficulty_; }
private:
    std::array<std::vector<SpawnRule>,5> rules_;
    float phaseA_=0,phaseB_=0,scale_=.3f,difficulty_=0,nextCheck_=0,nextSpawn_=0;
};
struct ProductionJob {
    std::string name; float startTime=0,duration=0;
    static ProductionJob begin(std::string name,double time,float originalDuration,float buildTimeFactor);
    [[nodiscard]] float progress(double time) const;
    [[nodiscard]] bool complete(double time) const { return progress(time)>1; }
};
class BattleClock {
public:
    void start(float cellCapacity,float cellRate);
    void configure(float cellCapacity,float cellRate) { capacity_=cellCapacity;rate_=cellRate; }
    void tick(bool shown,bool paused,int gameState);
    [[nodiscard]] double time() const { return time_; }
    [[nodiscard]] float cells() const { return cells_; }
    void setCells(float cells) { cells_=cells; }
    [[nodiscard]] float nextCellTime() const { return nextCellTime_; }
    static int enemyBaseHP(int stageIndex);
    static float initialDifficultyScale(int completedCount);
private:
    double time_=0; float cells_=0,capacity_=0,rate_=0,nextCellTime_=0;
};
struct MachineDefinition {
    std::string name; int hp=0,experience=0,type=0,referenceType=0,radius=0;
    float speed=0;
    static MachineDefinition fromTable(const DataTable& table,std::size_t row,BattleGeometry geometry);
};
struct CollisionSurface { Vec2 normal,direction; };
// GobHvMach::onCollideObject: (WeMade 1.0.3, 0x13a04).
// Apply once per same-side pair in original child-list order.
void separateGroundBodies(Vec2& first,float firstRadius,bool firstTurret,
                          Vec2& second,float secondRadius,bool secondTurret);
struct VehicleMotion {
    Vec2 position,destination,direction;
    float angle=0,speed=0,moveSpeed=.6f,rotationDivisor=30,scale=1;
    int state=0;
    void setDestination(Vec2 p) { destination=p;state=1; }
    void tick(DarwinRandom& random,std::optional<CollisionSurface> collision=std::nullopt);
};
struct AircraftMotion {
    Vec2 position,destination,direction;
    float angle=0,speed=0,moveSpeed=.6f,rotationDivisor=30;
    int state=0;
    void setDestination(Vec2 p) { destination=p;state=1; }
    void tick();
};
struct TrainMotion {
    Vec2 position,destination,direction; float speed=0,moveSpeed=.6f; int state=0;
    void setDestination(Vec2 p) { destination=p;state=1; }
    void tick();
};
// Original ColVector 0x43f9c / 0x440e0 / 0x44420.
struct TerrainSegment {
    Vec2 origin,second,direction,normal,cornerNormal; float length=0;
    static TerrainSegment between(Vec2 origin,Vec2 second);
    void setPrevious(const TerrainSegment*);
    [[nodiscard]] float penetration(Vec2 position,float radius=16) const;
};
struct EnemySteering {
    float stopDelay=0;
    bool firstAttack=false;
    static EnemySteering begin(double time,DarwinRandom& random);
    std::optional<Vec2> vehicle(double time,Vec2 position,float targetRange,float baseBoundary,
                                float deviceScale,bool hasTarget,bool moveToTarget,DarwinRandom& random);
    std::optional<Vec2> aircraft(double time,int movementState,DarwinRandom& random);
};
struct TargetCandidate { Vec2 position; int hp; };
std::optional<std::size_t> nearestLivingTarget(Vec2 from,float range,std::span<const TargetCandidate> targets);
int hpAfterDamage(int hp,int damage);
int originalStepSpeedPoint(float stepSize);
int originalVehicleSpeedPoint(float moveSpeed);
struct PartDefinition {
    std::string name; std::uint8_t type=0,tileCount=0; std::vector<Vec2> sockets;
    std::string image; unsigned splitX=1,splitY=1,tileNo=0;
    [[nodiscard]] Vec2 socket(unsigned index,float scale) const;
};
struct AssemblyPart {
    std::string name; std::uint8_t parentSocket=0,zOrder=0;
    std::vector<AssemblyPart> children;
    unsigned reverse=0; // Tile2D bit 1 flips x; bit 2 flips y.
    std::optional<float> angle;
    Vec2 addPosition; // GobMachParts addPos, rotated by this part angle.
    float visualScale=1;
};
struct PartPose {
    const PartDefinition* definition=nullptr;
    Vec2 localCenter;
    float angle=0,deviceScale=.5f;
    unsigned zOrder=0,reverse=0;
    float visualScale=1;
    bool hasParent=false;
    [[nodiscard]] std::optional<Vec2> muzzle(unsigned socketIndex=1,bool reverse=false) const;
    [[nodiscard]] unsigned nextMuzzle(unsigned current) const;
};
class BattleModels {
public:
    static BattleModels parse(const Chunk& parts,const Chunk& machines);
    static BattleModels load(ResourceAccess& resources);
    void append(const Chunk& parts,const Chunk& machines);
    void appendMachineVariant(std::string name,std::string_view source,std::string_view weaponSuffix);
    void configureTiles(const DataTable& partsList,const DataTable& tilesList);
    [[nodiscard]] std::vector<PartPose> pose(std::string_view machine,float deviceScale,float bodyAngle,float weaponAngle,float radarAngle) const;
    [[nodiscard]] std::vector<PartPose> pose(const std::vector<AssemblyPart>&,float deviceScale,float bodyAngle,float weaponAngle,float radarAngle) const;
    // Pose input uses original world angles (radians); screen origin is y-down.
    void appendSprites(Frame& frame,std::string_view machine,Vec2 screenOrigin,float deviceScale,
                       float bodyAngle,float weaponAngle,float radarAngle,
                       const std::function<Vec2(std::string_view)>& imageSize) const;
    void appendSprites(Frame&,std::span<const PartPose>,Vec2 screenOrigin,
                       const std::function<Vec2(std::string_view)>& imageSize) const;
    [[nodiscard]] const PartDefinition* part(std::string_view name) const;
    [[nodiscard]] const std::vector<AssemblyPart>* machine(std::string_view name) const;
    [[nodiscard]] std::size_t partCount() const { return parts_.size(); }
    [[nodiscard]] std::size_t machineCount() const { return machines_.size(); }
private:
    std::unordered_map<std::string,PartDefinition> parts_;
    std::unordered_map<std::string,std::vector<AssemblyPart>> machines_;
};
}
