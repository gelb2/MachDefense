#include "Core/Simulation/SpecialAttack.hpp"
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace md {
namespace {
float randomFloat(DarwinRandom& random) {return static_cast<float>(random.next())*0x1p-31f;}
}
std::vector<Projectile> crossfireProjectiles(const WeaponDefinition& weapon,Vec2 origin,Vec2 target,
                                             float bodyAngle,double time,DarwinRandom& random) {
    std::vector<Projectile> result;result.reserve(weapon.shotCount);
    for(unsigned i=0;i<weapon.shotCount;++i) {
        const float offset=randomFloat(random)*1.5f;
        const float angle=bodyAngle+((i&1)?-1.5f-offset:offset+1.5f);
        const float dx=randomFloat(random)*30.f-15.f,dy=randomFloat(random)*30.f-15.f;
        auto bullet=Projectile::fire(weapon,origin,{target.x+dx,target.y+dy},angle,time,random);
        bullet.lockOn=true;result.push_back(bullet);
    }
    return result;
}
std::string specialWeaponName(std::string_view prefix,float level) {
    char suffix[32];std::snprintf(suffix,sizeof(suffix),"_%02d",static_cast<int>(level));
    return std::string(prefix)+suffix;
}
std::optional<EnemyAirRaid> EnemyAirRaid::forStage(unsigned stage, std::size_t stageCount, int minimumMechHP) {
    constexpr unsigned firstRaidStage = 10;
    if (stage < firstRaidStage || stage >= stageCount || minimumMechHP < 2) return {};
    const auto cap = static_cast<std::int64_t>(minimumMechHP) * 125 / 100;
    const auto span = static_cast<std::int64_t>(stageCount - 1 - firstRaidStage);
    const auto progress = static_cast<std::int64_t>(stage - firstRaidStage);
    // Half the damage cap on the first raid stage, rising linearly to the full cap.
    const auto damage = span ? cap * (span + progress) / (2 * span) : cap;
    return EnemyAirRaid{static_cast<int>(damage)};
}
int EnemyAirRaid::damageForDrop(unsigned dropIndex, unsigned shotCount) const {
    const auto count = static_cast<int>(shotCount);
    return maximumDamage / count + (dropIndex < static_cast<unsigned>(maximumDamage % count) ? 1 : 0);
}
EnemyAirRaidSchedule EnemyAirRaidSchedule::fromTable(const DataTable& table, unsigned stageNumber) {
    const auto& columns=table.columns();
    const auto row=table.findRow(std::to_string(stageNumber));
    if (!row || columns.size()<6 || columns.size()%2!=0 || columns[0]!="Stage" || columns[1]!="CycleSeconds")
        throw std::runtime_error("Missing enemy raid schedule for stage "+std::to_string(stageNumber));

    EnemyAirRaidSchedule schedule;
    constexpr int minimumGap=20, maximumGap=45;
    constexpr double aircraftInterval=1.5;
    constexpr int maximumAircraft=3;
    std::int64_t previousWave=0;
    for (std::size_t i=2;i<columns.size();i+=2) {
        const std::string_view timeColumn=columns[i], countColumn=columns[i+1];
        if (!timeColumn.starts_with("At") || !countColumn.starts_with("Count") ||
            timeColumn.substr(2)!=countColumn.substr(5))
            throw std::runtime_error("Enemy raid schedule needs matching time and aircraft columns");
        const auto at=table.integer(*row,timeColumn);
        if (i==2 && (!at || *at!=EnemyAirRaid::startsAfter))
            throw std::runtime_error("Enemy raid schedule must start at 60 seconds");
        if (i>2 && (!at || *at<previousWave+minimumGap || *at>previousWave+maximumGap))
            throw std::runtime_error("Enemy raid schedule gap must be 20–45 seconds between waves");
        const auto count=table.integer(*row,countColumn);
        if (!count || *count<1 || *count>maximumAircraft)
            throw std::runtime_error("Enemy raid aircraft count must be 1–3");
        for (int aircraft=0;aircraft<*count;++aircraft)
            schedule.times_.push_back(static_cast<double>(*at)+aircraft*aircraftInterval);
        previousWave=*at;
    }
    const auto period=table.integer(*row,"CycleSeconds");
    const auto span=previousWave-EnemyAirRaid::startsAfter;
    if (!period || *period<span+minimumGap || *period>span+maximumGap)
        throw std::runtime_error("Enemy raid schedule needs a 20–45 second gap between cycles");
    schedule.cycleSeconds_=static_cast<double>(*period);
    return schedule;
}
bool EnemyAirRaidSchedule::takeDue(double elapsed) {
    if (elapsed<cycleStart_+times_[next_]) return false;
    if (++next_==times_.size()) {
        next_=0;
        cycleStart_+=cycleSeconds_;
    }
    return true;
}
AirStrike AirStrike::start(const WeaponDefinition& weapon,Vec2 target,float delay,double time,
                           float scale,DarwinRandom& random,const std::function<ParticleState*()>& acquire,float animationTime,bool reverseApproach) {
    AirStrike strike;strike.target=target;strike.deviceScale=scale;strike.remainingShots=weapon.shotCount;
    Vec2 direction{randomFloat(random)*.4f-.2f,reverseApproach?-1.f:1.f};
    const float length=std::sqrt(direction.x*direction.x+direction.y*direction.y);
    direction.x=direction.x/length*scale;direction.y=direction.y/length*scale;
    strike.position={target.x-direction.x*1200.f,target.y-direction.y*1200.f};
    strike.angle=static_cast<float>(std::atan2(static_cast<double>(direction.y),static_cast<double>(direction.x)));
    if(acquire)if(auto* marker=acquire()) {
        marker->blend=reverseApproach?Blend::alpha:Blend::additive;
        marker->life=reverseApproach?5.f:3.f;marker->tile=0;marker->position=target;
        marker->scale={15,15};marker->easyOutScale=1.5f;marker->easyOutDivisor=6;
        marker->rotationVelocity=.1f;marker->start(animationTime);
    }
    strike.velocity={direction.x*4.f,direction.y*4.f};
    strike.startTime=static_cast<float>(time+static_cast<double>(delay));
    strike.fireDelay=.3f/static_cast<float>(strike.remainingShots); // original division, even if zero
    return strike;
}
void AirStrike::advance() {
    position.x+=velocity.x;position.y+=velocity.y;
    velocity.x+=velocity.x*.01f;velocity.y+=velocity.y*.01f;
}
std::optional<Projectile> AirStrike::tick(const WeaponDefinition& weapon,double time,DarwinRandom& random) {
    if(removed)return {};
    advance();
    if(time<=static_cast<double>(startTime)||remainingShots<1) {
        if(time>static_cast<double>(startTime+4.f))removed=true;
        return {};
    }
    if(time<=static_cast<double>(fireTime))return {};
    const float a=randomFloat(random)*3.1415927410125732421875f;
    const float direction=a+a;
    const float first=randomFloat(random),second=randomFloat(random);
    const float radius=((first*weapon.aimRate*120.f+second*weapon.aimRate*120.f)-weapon.aimRate*120.f)*deviceScale;
    const Vec2 offset{std::cos(direction)*radius,std::sin(direction)*radius};
    auto bullet=Projectile::fire(weapon,{position.x+offset.x*.3f,position.y+offset.y*.3f},
        {target.x+offset.x,target.y+offset.y},angle,time,random);
    bullet.drop=true;--remainingShots;fireTime=static_cast<float>(time+static_cast<double>(fireDelay));
    return bullet;
}
void AirStrike::appendSprite(Frame& frame,Vec2 screen,Vec2 size,float retinaScale,std::string_view image) const {
    if(removed||size.x<=0||size.y<=0)return;
    const float w=size.x*1.3f*retinaScale,h=size.y*1.3f*retinaScale;
    Sprite sprite;sprite.image=image;sprite.destination={screen.x-w*.5f,screen.y-h*.5f,w,h};
    sprite.rotationDegrees=-angle*180.f/3.14159265358979323846f;frame.sprites.push_back(std::move(sprite));
}
bool BotRecovery::tick(int& hp,int maximumHP,double time) {
    if(hp<=0||hp>=maximumHP||time<=static_cast<double>(lastRefreshTime+5.f))return false;
    ++hp;lastRefreshTime=static_cast<float>(time);return true;
}
bool botReadyToTransform(Vec2 position,Vec2 destination,int state,std::string_view postBuild) {
    return std::abs(position.x-destination.x)<=4.f&&std::abs(position.y-destination.y)<=4.f&&state==0&&!postBuild.empty();
}
}
