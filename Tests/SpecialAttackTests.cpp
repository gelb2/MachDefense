#include "Core/Simulation/SpecialAttack.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace md;
int main() {
    WeaponDefinition missile;missile.type=WeaponType::missile;missile.shotCount=4;missile.speed=20;missile.aimRate=.5;
    assert(specialWeaponName("CF_MISSILE",2.9f)=="CF_MISSILE_02");
    DarwinRandom random(7),reference(7);
    const auto shots=crossfireProjectiles(missile,{10,20},{100,200},.3f,0,random);
    assert(shots.size()==4);
    for(unsigned i=0;i<shots.size();++i) {
        const auto& s=shots[i];assert(s.lockOn&&!s.drop&&!s.toPlayer);
        assert(s.position.x==10&&s.position.y==20);
        assert(s.destination.x>=85&&s.destination.x<=115&&s.destination.y>=185&&s.destination.y<=215);
        assert((i&1)?s.angle<=-1.2f&&s.angle>=-2.7f:s.angle>=1.8f&&s.angle<=3.3f);
        for(unsigned j=0;j<4;++j)reference.next();
    }
    assert(random.next()==reference.next());
    WeaponDefinition bomb=missile;bomb.type=WeaponType::bomb;bomb.shotCount=3;
    DarwinRandom airRandom(13),airRef(13);ParticleState marker;bool acquired=false;
    auto strike=AirStrike::start(bomb,{200,300},1,10,.5f,airRandom,[&](){acquired=true;marker.reset(10,airRandom);return &marker;},10);
    assert(acquired&&marker.shown&&marker.life==3&&marker.scale.x==15&&marker.easyOutDivisor==6&&marker.tile==0);
    airRef.next();airRef.next();assert(airRandom.next()==airRef.next());
    assert(std::abs(std::hypot(strike.position.x-200,strike.position.y-300)-600)<.001);
    assert(std::abs(std::hypot(strike.velocity.x,strike.velocity.y)-2)<.001);
    assert(strike.startTime==11&&strike.remainingShots==3&&!strike.tick(bomb,11,airRandom));
    const auto before=strike.position,velocity=strike.velocity;
    auto first=strike.tick(bomb,11.01,airRandom);assert(first&&first->drop&&!first->lockOn&&strike.remainingShots==2);
    assert(strike.position.x==before.x+velocity.x&&strike.position.y==before.y+velocity.y);
    assert(strike.velocity.x==velocity.x+velocity.x*.01f);
    assert(first->angle==strike.angle&&std::abs(first->destination.x-200)<=30&&std::abs(first->destination.y-300)<=30);
    assert(!strike.tick(bomb,strike.fireTime,airRandom));
    assert(strike.tick(bomb,11.2,airRandom));assert(strike.tick(bomb,11.4,airRandom));
    assert(strike.remainingShots==0&&!strike.tick(bomb,15,airRandom)&&!strike.removed);
    strike.tick(bomb,15.001,airRandom);assert(strike.removed);
    AirStrike visible;Frame frame;visible.appendSprite(frame,{50,60},{100,200});assert(frame.sprites.size()==1);
    assert(frame.sprites[0].image=="Airplane.png"&&frame.sprites[0].destination.width==65);
    BotRecovery bot;int hp=9;assert(!bot.tick(hp,10,5));assert(bot.tick(hp,10,5.001)&&hp==10);
    assert(!bot.tick(hp,10,100));hp=0;assert(!bot.tick(hp,10,100));
    assert(botReadyToTransform({1,2},{5,6},0,"Turret"));
    assert(!botReadyToTransform({1,2},{5,6},1,"Turret"));assert(!botReadyToTransform({1,2},{5,6},0,""));
    assert(!botReadyToTransform({1,2},{5.001f,6},0,"Turret"));
    std::cout<<"Special attacks: crossfire, airstrike timing/RNG, marker and bot conditions passed\n";
}
