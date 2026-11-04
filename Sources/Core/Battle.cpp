#include "Battle.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace md {
namespace {
float randomFloat(DarwinRandom& random) { return float(random.next())*0x1p-31f; }
AssemblyPart assembly(const Chunk& c) {
    if(c.type!=4 || c.data.size()<26) throw std::runtime_error("Invalid original assembly part");
    AssemblyPart p;p.name=c.text(0,24);p.parentSocket=c.byte(24);p.zOrder=c.byte(25);
    for(const auto& child:c.children) if(child.type==4) p.children.push_back(assembly(child));
    return p;
}
}
EnemySpawner::EnemySpawner(SpawnRule rule,double time,DarwinRandom& random):rule_(std::move(rule)) {
    if(!std::isfinite(rule_.startTime)||!std::isfinite(rule_.delay)||rule_.name.empty()) throw std::invalid_argument("Invalid spawn rule");
    if(rule_.startTime==-1) rule_.count=int(float(rule_.count)*.5f+(randomFloat(random)*float(rule_.count))*.5f+.5f);
    rule_.count=std::max(1,rule_.count);
    nextTime_=float(time+double(rule_.startTime));
}
std::optional<SpawnEvent> EnemySpawner::tick(double time,int state,BattleGeometry geometry,DarwinRandom& random) {
    if(finished()||state!=0||!(double(nextTime_)<time)) return std::nullopt;
    // Preserve original float operation order: scale enters before randomness.
    float halfWidth=220.f*geometry.deviceScale;
    SpawnEvent event{rule_.name,{randomFloat(random)*(halfWidth+halfWidth)-halfWidth,geometry.halfLength()+32.f*geometry.deviceScale}};
    ++generated_;
    float halfDelay=rule_.delay*.5f;
    nextTime_=float(time+double(halfDelay)+double(randomFloat(random)*halfDelay));
    return event;
}
ProductionJob ProductionJob::begin(std::string name,double time,float duration,float factor) {
    return {std::move(name),float(time),duration*factor};
}
float ProductionJob::progress(double time) const { return float((time-double(startTime))/double(duration)); }
void BattleClock::start(float capacity,float rate) {
    capacity_=capacity;rate_=rate;cells_=capacity*.4f;
}
void BattleClock::tick(bool shown,bool paused,int state) {
    if(!shown||paused) return;
    time_+=double(.0166f);
    if(state==0 && double(nextCellTime_)<=time_) {
        // Original increment can exceed capacity; subsequent ticks leave it as is.
        if(cells_<capacity_) cells_+=rate_;
        nextCellTime_=float(time_+double(.08f));
    }
}
int BattleClock::enemyBaseHP(int stage) {
    if(stage<0 || stage>(std::numeric_limits<int>::max()-40000)/10000) throw std::out_of_range("Invalid stage index");
    return 40000+stage*10000;
}
float BattleClock::initialDifficultyScale(int completed) { return .3f + (completed>4 ? .7f : .7f*.25f*float(completed)); }
BattleModels BattleModels::parse(const Chunk& parts,const Chunk& machines) {
    BattleModels result;
    std::function<void(const Chunk&)> readPart=[&](const Chunk& c) {
        if(c.type==1&&!c.data.empty()) {
            if(c.data.size()<27) throw std::runtime_error("Truncated original part definition");
            PartDefinition p; p.name=c.text(0,24);p.type=c.byte(24);p.tileCount=c.byte(25);
            p.image=p.tileCount ? p.name+".png" : "PartsDummy.png";p.splitX=std::max(1u,unsigned(p.tileCount));
            auto count=c.byte(26);
            if(c.data.size()<27+std::size_t(count)*8) throw std::runtime_error("Truncated original part sockets");
            for(unsigned i=0;i<count;++i) p.sockets.push_back({c.real32(27+i*8),c.real32(31+i*8)});
            result.parts_.insert_or_assign(p.name,std::move(p));
        }
        for(const auto& child:c.children) readPart(child);
    };
    readPart(parts);
    std::function<void(const Chunk&)> readMachine=[&](const Chunk& c) {
        if(c.type==3&&!c.data.empty()) {
            std::vector<AssemblyPart> model;
            for(const auto& child:c.children) if(child.type==4) model.push_back(assembly(child));
            result.machines_.insert_or_assign(c.text(0,24),std::move(model));
        }
        for(const auto& child:c.children) readMachine(child);
    };
    readMachine(machines);return result;
}
const PartDefinition* BattleModels::part(std::string_view name) const { auto i=parts_.find(std::string(name));return i==parts_.end()?nullptr:&i->second; }
const std::vector<AssemblyPart>* BattleModels::machine(std::string_view name) const { auto i=machines_.find(std::string(name));return i==machines_.end()?nullptr:&i->second; }
Vec2 PartDefinition::socket(unsigned index,float scale) const {
    if(index>=sockets.size()) return {};
    return {std::trunc(sockets[index].x)*scale,std::trunc(sockets[index].y)*scale};
}
void BattleModels::configureTiles(const DataTable& partsList,const DataTable& tilesList) {
    for(std::size_t row=0;row<partsList.rowCount();++row) {
        auto i=parts_.find(std::string(partsList.value(row,"PartsName")));
        if(i==parts_.end()) continue;
        auto tileName=partsList.value(row,"TileName");if(tileName.empty()||tileName=="-") continue;
        std::string image=std::string(tileName)+".png";
        auto tileRow=tilesList.findRow(image);if(!tileRow) throw std::runtime_error("Shared part tile missing from TileList");
        auto x=tilesList.integer(*tileRow,"X"),y=tilesList.integer(*tileRow,"Y"),tile=partsList.integer(row,"No");
        if(!x||!y||!tile||*x<1||*y<1||*tile<0) throw std::runtime_error("Invalid original tile grid");
        i->second.image=image;i->second.splitX=unsigned(*x);i->second.splitY=unsigned(*y);i->second.tileNo=unsigned(*tile);
    }
}

std::vector<PartPose> BattleModels::pose(std::string_view name,float scale,float bodyAngle,float weaponAngle,float radarAngle) const {
    auto tree=machine(name);if(!tree) throw std::runtime_error("Unknown original machine model");
    return pose(*tree,scale,bodyAngle,weaponAngle,radarAngle);
}
std::vector<PartPose> BattleModels::pose(const std::vector<AssemblyPart>& tree,float scale,float bodyAngle,float weaponAngle,float radarAngle) const {
    std::vector<PartPose> poses;
    auto rotate=[](Vec2 v,float a){float c=std::cos(a),s=std::sin(a);return Vec2{v.x*c-v.y*s,v.x*s+v.y*c};};
    std::function<void(const AssemblyPart&,Vec2,float,bool)> append=[&](const AssemblyPart& a,Vec2 basePos,float parentAngle,bool hasParent){
        auto p=part(a.name);if(!p) throw std::runtime_error("Assembly references unknown part");
        float angle=a.angle.value_or(p->type==4?weaponAngle:(p->type==7?radarAngle:parentAngle));
        Vec2 anchor=p->socket(0,scale),drawingAnchor=anchor;
        if(a.reverse) drawingAnchor.y=-drawingAnchor.y;
        Vec2 baseRot=rotate(basePos,parentAngle),anchorRot=rotate(drawingAnchor,angle),add=rotate(a.addPosition,angle);
        poses.push_back({p,{baseRot.x+add.x-anchorRot.x,baseRot.y+add.y-anchorRot.y},angle,scale,a.zOrder,a.reverse,a.visualScale,hasParent});
        for(const auto& child:a.children) {
            Vec2 socket=p->socket(child.parentSocket,scale);
            append(child,{basePos.x+socket.x-anchor.x,basePos.y+socket.y-anchor.y},angle,true);
        }
    };
    for(const auto& a:tree) append(a,{},bodyAngle,false);
    std::stable_sort(poses.begin(),poses.end(),[](const auto& a,const auto& b){return a.zOrder<b.zOrder;});
    return poses;
}
std::optional<Vec2> PartPose::muzzle(unsigned index,bool reverse) const {
    if(!definition || definition->sockets.size()<2 || index>=definition->sockets.size()) return std::nullopt;
    Vec2 base=definition->socket(0,deviceScale),socket=definition->socket(index,deviceScale);
    Vec2 relative{socket.x-base.x,(reverse||this->reverse)?base.y-socket.y:socket.y-base.y};
    float c=std::cos(angle),s=std::sin(angle);
    return Vec2{localCenter.x+relative.x*c-relative.y*s,localCenter.y+relative.x*s+relative.y*c};
}
unsigned PartPose::nextMuzzle(unsigned current) const {
    return !definition || current+1>=definition->sockets.size()?1:current+1;
}
void BattleModels::appendSprites(Frame& frame,std::string_view name,Vec2 origin,float scale,float bodyAngle,float weaponAngle,float radarAngle,const std::function<Vec2(std::string_view)>& imageSize) const {
    const auto poses=pose(name,scale,bodyAngle,weaponAngle,radarAngle);
    appendSprites(frame,poses,origin,imageSize);
}
void BattleModels::appendSprites(Frame& frame,std::span<const PartPose> poses,Vec2 origin,const std::function<Vec2(std::string_view)>& imageSize) const {
    std::vector<Sprite> images,shadows;
    for(const auto& pose:poses) {
        const float scale=pose.deviceScale*pose.visualScale;
        const auto* p=pose.definition;Vec2 size=imageSize(p->image);
        // Missing dummy textures return nil in ResMgr_Tile; their sockets still attach children.
        if(size.x<=0 || size.y<=0) continue;
        Vec2 center{origin.x+pose.localCenter.x,origin.y-pose.localCenter.y};
        float w=size.x/p->splitX,h=size.y/p->splitY;
        Sprite image;image.image=p->image;image.sourcePx=Rect{float(p->tileNo%p->splitX)*w,float(p->tileNo/p->splitX)*h,w,h};
        image.destination={center.x-w*scale*.5f,center.y-h*scale*.5f,w*scale,h*scale};
        image.rotationDegrees=-pose.angle*180.f/3.14159265358979323846f;
        image.flipX=(pose.reverse&1)!=0;image.flipY=(pose.reverse&2)!=0;
        Sprite shadow=image;float shadowLength=(p->type==2?2.f:4.f)*scale;
        shadow.destination.x+=shadowLength;shadow.destination.y+=shadowLength;shadow.color={0,0,0,1};
        images.push_back(std::move(image));shadows.push_back(std::move(shadow));
    }
    for(auto& sprite:shadows) frame.sprites.push_back(std::move(sprite));
    for(auto& sprite:images) frame.sprites.push_back(std::move(sprite));
}

int originalStepSpeedPoint(float stepSize) { // setStepSize 0x13de0, ARM conditional instructions.
    if(stepSize==0)return 0;
    float position=0,total=0,elapsed=0;
    do {
        const float next=std::abs(static_cast<double>(position-stepSize))>.5
            ?position+(stepSize-position)*.125f:stepSize;
        elapsed+=.0166f;total+=next-position;position=next;
    } while(position!=stepSize);
    return static_cast<int>(total/elapsed);
}
int originalVehicleSpeedPoint(float moveSpeed) {return static_cast<int>(moveSpeed/.0166f);}
MachineDefinition MachineDefinition::fromTable(const DataTable& table,std::size_t row,BattleGeometry geometry) {
    auto integer=[&](std::string_view key){auto v=table.integer(row,key);if(!v) throw std::runtime_error("Invalid MachList integer");return int(*v);};
    MachineDefinition m;m.name=table.value(row,"MachName");m.hp=integer("HP")*2;m.experience=integer("EXP");
    auto speed=table.number(row,"Spd");if(!speed) throw std::runtime_error("Invalid MachList speed");
    m.speed=*speed*geometry.deviceScale;m.radius=int(float(integer("Rad"))*geometry.deviceScale);
    auto type=table.value(row,"Type");
    if(type=="TANK") m.type=1;else if(type=="PLANE") m.type=2;else if(type=="TURRET") m.type=3;else if(type=="BOT") m.type=5;
    auto ref=table.value(row,"Refer");
    if(ref=="BOT_D") m.referenceType=1;else if(ref=="BOT_P") m.referenceType=2;else if(ref=="BOT_R") m.referenceType=3;
    return m;
}
void VehicleMotion::tick(DarwinRandom& random,std::optional<CollisionSurface> collision) {
    if(state!=1) return;
    Vec2 delta{destination.x-position.x,destination.y-position.y};
    if(std::abs(delta.x)<=4 && std::abs(delta.y)<=4) {state=0;speed=0;return;}
    float desired=float(std::atan2(double(delta.y),double(delta.x)));
    if(collision && delta.x*collision->normal.x+delta.y*collision->normal.y>0) {
        desired=float(std::atan2(double(collision->direction.y),double(collision->direction.x)));
        if(delta.x*collision->direction.x+delta.y*collision->direction.y<0)
            desired=float(double(desired)+3.2415926550799092);
        else desired-=.1f;
    }
    float difference=desired-angle;
    constexpr float pi=0x1.921fb6p+1f,turn=0x1.921fb4p+2f;
    if(difference>pi) angle+=turn;else if(difference < -pi) angle-=turn;
    angle+=(desired-angle)/rotationDivisor;
    float distanceSquared=delta.x*delta.x+delta.y*delta.y;
    speed+=(distanceSquared>900 ? (moveSpeed-speed)/100.f : (.5f-speed)/60.f);
    direction={std::cos(angle)*speed,std::sin(angle)*speed};
    position.x+=direction.x;position.y+=direction.y;
    if(random.next()%5==0) {
        position.x+=randomFloat(random)-.5f;
        position.y+=randomFloat(random)-.5f;
        scale=(randomFloat(random)*.02f-.01f)+1.f;
    }
}
std::optional<std::size_t> nearestLivingTarget(Vec2 from,float range,std::span<const TargetCandidate> candidates) {
    float shortest=range*range;std::optional<std::size_t> result;
    for(std::size_t i=0;i<candidates.size();++i) {
        if(candidates[i].hp<=0) continue;
        float dx=from.x-candidates[i].position.x,dy=from.y-candidates[i].position.y;
        float distance=dx*dx+dy*dy;
        if(distance<shortest) {shortest=distance;result=i;}
    }
    return result;
}
int hpAfterDamage(int hp,int damage) {
    auto result=std::int64_t(hp)-std::int64_t(damage);
    return int(std::clamp(result,std::int64_t(0),std::int64_t(std::numeric_limits<int>::max())));
}

StageSpawnPlan StageSpawnPlan::parse(const Chunk& map,const DataTable& table,BattleGeometry geometry) {
    StageSpawnPlan plan;
    auto shortAt=[](const Chunk& c,std::size_t p) {return std::bit_cast<std::int16_t>(std::uint16_t(std::uint16_t(c.byte(p))|std::uint16_t(c.byte(p+1))<<8));};
    for(const auto& c:map.children) {
        if(c.type==4) {
            plan.placed.push_back({c.text(2,16),{float(shortAt(c,18))*geometry.deviceScale,float(shortAt(c,20))*geometry.deviceScale}});
        } else if(c.type==3) {
            int count=shortAt(c,0);std::vector<Vec2> polygon;
            for(int i=0;i<count;++i) polygon.push_back({c.real32(2+8*i)*geometry.deviceScale,c.real32(6+8*i)*geometry.deviceScale});
            if(!polygon.empty()) plan.collisionPolygons.push_back(std::move(polygon));
        }
    }
    auto integer=[&](std::size_t row,std::string_view key){auto v=table.integer(row,key);if(!v && table.value(row,key).empty()) return 0;if(!v) throw std::runtime_error("Invalid original ESpawn integer");return int(*v);};
    for(std::size_t row=0;row<table.rowCount();++row) {
        SpawnRule rule;rule.name=table.value(row,"MachName");rule.count=integer(row,"Cnt");rule.delay=float(integer(row,"Dly"));
        int dif=integer(row,"Dif");
        if(dif>=1 && dif<=5) {
            auto& bucket=plan.difficulty[dif-1];
            if(bucket.size()<32) {
                rule.startTime=-1;rule.cumulativeRate=(bucket.empty()?0:bucket.back().cumulativeRate)+integer(row,"Rte");
                bucket.push_back(rule);
            }
        } else {
            rule.startTime=float(dif/100)*60.f+float(dif%100);plan.timed.push_back(rule);
        }
        if(std::find(plan.enemyNames.begin(),plan.enemyNames.end(),rule.name)==plan.enemyNames.end()) plan.enemyNames.push_back(rule.name);
    }
    std::sort(plan.enemyNames.begin(),plan.enemyNames.end());return plan;
}
void StageSpawnPlan::appendGeneratedWaves(int stage,DarwinRandom& random,const std::function<int(std::string_view)>& hp) {
    if(stage<0||stage>=80||enemyNames.empty()) throw std::invalid_argument("Invalid original wave stage");
    int selected=std::min(4,int(enemyNames.size()));
    float interval=std::max(5.f,60.f-float(stage)),base=90,span=30,accumulated=0;
    int scale=stage*10+2000;
    int budget=stage*((stage*10+7000)-std::max(scale,5000))+6000;
    int increment=scale>5000?3000:int(float(scale)*.6f);
    for(int wave=0;wave<7;++wave) {
        float time=base+randomFloat(random)*span;base+=interval;accumulated+=time;
        int allowance=int(float(budget)/float(selected)),remaining=budget,available=0;
        for(int i=int(enemyNames.size())-1;i>=int(enemyNames.size())-selected;--i) {
            available=std::min(remaining,available+allowance);int health=hp(enemyNames[i]);
            if(health>0 && health<=available) {
                SpawnRule rule;rule.name=enemyNames[i];rule.count=available/health;
                float r=randomFloat(random);rule.delay=(r+r)+2.f;
                rule.startTime=accumulated+randomFloat(random)*5.f;
                timed.push_back(rule);remaining-=health*rule.count;available-=health*rule.count;
            }
        }
        span+=10;budget+=increment;
    }
}
EnemyDirector::EnemyDirector(std::array<std::vector<SpawnRule>,5> rules,int completed):rules_(std::move(rules)),scale_(BattleClock::initialDifficultyScale(completed)) {}
std::optional<EnemySpawner> EnemyDirector::tick(double time,DarwinRandom& random) {
    if(double(nextCheck_)<time) {
        nextCheck_=float(time+double(.4f));
        phaseA_+=randomFloat(random)*((1.2f-difficulty_)*.02f);
        phaseB_+=randomFloat(random)*((1.2f-difficulty_)*.01f);
        if(scale_<1) scale_=std::min(1.f,scale_+randomFloat(random)*.002f);
        difficulty_=(((1.f-std::cos(phaseA_))*.5f+(1.f-std::cos(phaseB_))*.5f)*.5f)*scale_;
    }
    if(!(double(nextSpawn_)<time)) return std::nullopt;
    int level=std::clamp(int(difficulty_*5),0,4);
    while(level>0 && rules_[level].empty()) --level;
    auto& bucket=rules_[level];if(bucket.empty()||bucket.back().cumulativeRate<=0) throw std::runtime_error("Empty original difficulty bucket");
    int choice=int(random.next()%unsigned(bucket.back().cumulativeRate));
    auto found=std::find_if(bucket.begin(),bucket.end(),[&](const auto& r){return choice<r.cumulativeRate;});
    EnemySpawner spawner(*found,time,random);
    float oneMinus=1.f-difficulty_;
    float jitter=randomFloat(random)*(15.f+(30.f-15.f)*oneMinus);
    float interval=10.f+(20.f-10.f)*oneMinus;
    nextSpawn_=float(time+double(jitter)+double(interval));return spawner;
}
void AircraftMotion::tick() {
    if(state!=1) return;
    Vec2 delta{destination.x-position.x,destination.y-position.y};
    if(std::abs(delta.x)<=4 && std::abs(delta.y)<=4) {state=0;return;}
    float desired=float(std::atan2(double(delta.y),double(delta.x))),difference=desired-angle;
    constexpr float pi=0x1.921fb6p+1f,turn=0x1.921fb4p+2f;
    if(difference>pi) angle+=turn;else if(difference < -pi) angle-=turn;
    angle+=(desired-angle)/rotationDivisor;
    float distanceSquared=delta.x*delta.x+delta.y*delta.y;
    speed+=(distanceSquared>900 ? (moveSpeed-speed)/100.f : (.5f-speed)/60.f);
    direction={std::cos(angle)*speed,std::sin(angle)*speed};position.x+=direction.x;position.y+=direction.y;
}

void TrainMotion::tick() { // GobHvMach::processMove_Train 0x135dc
    if(state!=1) return;
    const Vec2 delta{destination.x-position.x,destination.y-position.y};
    if(std::abs(delta.x)<=4&&std::abs(delta.y)<=4) {state=0;speed=0;return;}
    const float squared=delta.x*delta.x+delta.y*delta.y;
    speed+=(squared>900?(moveSpeed-speed)/100.f:(.5f-speed)/60.f);
    const float length=std::sqrt(squared);
    direction={delta.x/length*speed,delta.y/length*speed};
    position.x+=direction.x;position.y+=direction.y;
}
TerrainSegment TerrainSegment::between(Vec2 origin,Vec2 second) {
    TerrainSegment line;line.origin=origin;line.second=second;
    line.direction={second.x-origin.x,second.y-origin.y};
    line.length=std::sqrt(line.direction.x*line.direction.x+line.direction.y*line.direction.y);
    if(line.length==0) throw std::runtime_error("Degenerate original terrain segment");
    line.direction.x/=line.length;line.direction.y/=line.length;
    line.normal={line.direction.y,-line.direction.x};line.cornerNormal=line.direction;
    return line;
}
void TerrainSegment::setPrevious(const TerrainSegment* previous) {
    cornerNormal=direction;
    if(previous) {cornerNormal.x+=previous->direction.x;cornerNormal.y+=previous->direction.y;}
    const float squared=cornerNormal.x*cornerNormal.x+cornerNormal.y*cornerNormal.y;
    const float size=std::sqrt(squared);
    if(size<1e-5f) {cornerNormal={};return;} // QVector::normalize 0x472d4.
    cornerNormal.x/=size;cornerNormal.y/=size;
}
float TerrainSegment::penetration(Vec2 position,float radius) const {
    const Vec2 delta{position.x-origin.x,position.y-origin.y};
    if(direction.x*delta.x+direction.y*delta.y>length+radius) return 0;
    if(cornerNormal.x*delta.x+cornerNormal.y*delta.y<0) return 0;
    const float projection=normal.x*delta.x+normal.y*delta.y;
    return std::max(0.f,static_cast<float>(static_cast<double>(radius)-std::abs(static_cast<double>(projection))));
}
EnemySteering EnemySteering::begin(double time,DarwinRandom& random) {
    EnemySteering result;result.firstAttack=random.next()%15==0;
    result.stopDelay=(float(time)+randomFloat(random))+.5f;return result;
}
std::optional<Vec2> EnemySteering::vehicle(double time,Vec2 position,float targetRange,float boundary,float scale,bool hasTarget,bool moveToTarget,DarwinRandom& random) {
    if(time<=double(stopDelay)||(hasTarget&&moveToTarget)) return std::nullopt;
    float x=position.x+(randomFloat(random)*32.f-16.f);
    x=std::clamp(x,-110.f*scale,110.f*scale);
    float y=position.y-160.f*scale,limit=targetRange*.8f-boundary;
    if(y<limit) {y=limit;stopDelay=float(time+100000.0);}
    else {float r=randomFloat(random);stopDelay=float(time+double(r+r)+1.0);}
    return Vec2{x,y};
}
std::optional<Vec2> EnemySteering::aircraft(double time,int state,DarwinRandom& random) {
    if(double(stopDelay)>=time && state!=0) return std::nullopt;
    stopDelay=float(time+double(randomFloat(random)*5.f)+5.0);
    float x=randomFloat(random)*1200.f-600.f,y=randomFloat(random)*1200.f-600.f;
    return Vec2{x,y};
}

}
