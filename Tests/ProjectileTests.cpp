#include "Projectile.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace md;
bool near(float a,float b,float epsilon=1e-5f) { return std::abs(a-b)<epsilon; }
int main() {
    WeaponDefinition gun;gun.type=WeaponType::bullet;gun.speed=.0166f;gun.damage=100;
    DarwinRandom random(1),expected(1);
    const float r=float(expected.next())*0x1p-31f;
    const float expectedSpeed=(gun.speed+(r*gun.speed)*.2f)+gun.speed*(-.1f);
    auto p=Projectile::fire(gun,{0,0},{20,0},0,2.25,random);
    assert(p.speed==expectedSpeed&&p.startTime==2.25f&&p.shotLengthSquared==400);
    assert(!p.tick(gun,true)&&p.position.x==0);
    assert(!p.tick(gun,false,false)&&p.position.x==0);
    assert(!p.tick(gun));assert(p.position.x==expectedSpeed/.0166f&&p.position.y==0);
    int steps=1;while(!p.tick(gun)&&++steps<100) {} assert(steps<25&&p.position.x>18);
    // The original launch animation decays each component, then starts flight.
    auto launch=Projectile::fire(gun,{0,0},{20,0},0,0,random,{15,.5});
    assert(!launch.tick(gun));assert(launch.position.x==14&&launch.position.y==0);
    WeaponDefinition missile=gun;missile.type=WeaponType::missile;
    auto m=Projectile::fire(missile,{0,0},{1000,0},0,0,random);
    const float initial=m.velocity.x;assert(initial==m.speed);
    m.tick(missile);assert(m.velocity.x==initial*2&&m.position.x==initial*2/.0166f);
    m.destination={1000,1000};m.lockOn=true;m.tick(missile);
    assert(m.acceleration.y>0&&m.velocity.y>0&&m.angle>0);
    auto deferred=Projectile::fire(missile,{0,0},{1000,0},0,0,random,{.5f,0});
    assert(deferred.velocity.x==0);deferred.tick(missile);assert(deferred.launchVelocity.x==0);
    deferred.tick(missile);assert(deferred.velocity.x==deferred.acceleration.x);
    WeaponDefinition plasma=gun;plasma.type=WeaponType::railgun;plasma.special1=2.5;plasma.special2=12;
    auto spinning=Projectile::fire(plasma,{0,0},{100,0},0,0,random);spinning.tick(plasma);assert(spinning.angle==.1f);
    auto effect=projectileSpecialDamage(plasma,3);assert(effect&&effect->endTime==5.5&&effect->damage==12&&effect->delay==1);
    assert(!projectileSpecialDamage(gun,0));
    std::vector<ProjectileTarget> enemies={{{0,0},1000,5},{{5,0},1000,5},{{20,0},1000,5}};
    std::vector<ProjectileTarget> players={{{0,0},1000,5}};
    auto hits=projectileCollide(gun,{0,0},true,0,enemies,players);
    assert(hits.size()==1&&!hits[0].player&&enemies[0].hp==900&&players[0].hp==1000);
    WeaponDefinition area=gun;area.damageRange=20;
    hits=projectileCollide(area,{0,0},false,0,enemies,players);
    assert(hits.size()==2&&enemies[0].hp==800&&enemies[1].hp==900&&enemies[2].hp==1000);
    enemies[1].position={15,0};hits=projectileCollide(area,{0,0},false,0,enemies,players);
    assert(hits.size()==2&&hits[1].damage==75);
    hits=projectileCollide(area,{0,0},true,0,enemies,players);assert(hits.size()==1&&hits[0].player);
    WeaponDefinition laser=gun;laser.type=WeaponType::laser;laser.special1=20;
    enemies={{{0,0},50,5},{{3,0},50,5},{{19,0},100,1},{{20,0},100,1}};
    hits=projectileCollide(laser,{0,0},false,0,enemies,players);
    assert(hits.size()==3&&hits[1].secondary&&hits[2].secondary);
    assert(enemies[0].hp==0&&enemies[1].hp==0&&enemies[2].hp==50&&enemies[3].hp==100);
    p=Projectile::fire(gun,{0,0},{100,0},0,0,random);p.drop=true;gun.bulletImage="bullet.png";gun.effectType=2;
    p.tick(gun);assert(p.imageScale.x>1&&p.imageScale.y>1&&!p.effectVisible);
    // Particle allocation determines original shared RNG consumption.
    gun.effectType=3;md::DarwinRandom effectRandom(1),expectedEffect(1);
    p=Projectile::fire(gun,{0,0},{100,0},0,0,random);
    int allocations=0;
    auto allocate=[&](){++allocations;return true;};
    assert(!p.drawEffect(gun,0,effectRandom,allocate)&&allocations==0);
    auto particle=p.drawEffect(gun,.1,effectRandom,allocate);assert(particle&&particle->additive&&allocations==1);
    for(int i=0;i<6;++i)expectedEffect.next();assert(effectRandom.next()==expectedEffect.next());
    assert(!p.drawEffect(gun,.11,effectRandom,allocate)&&allocations==1);
    auto failed=p.drawEffect(gun,.2,effectRandom,[](){return false;});assert(!failed&&p.effectRegenerationTime==.2f);
    assert(effectRandom.next()==expectedEffect.next());
    gun.effectType=4;particle=p.drawEffect(gun,.3,effectRandom,allocate);assert(particle&&!particle->additive&&particle->life==.6f);
    for(int i=0;i<4;++i)expectedEffect.next();assert(effectRandom.next()==expectedEffect.next());
    gun.effectType=5;particle=p.drawEffect(gun,.3,effectRandom,allocate);assert(particle&&particle->trailPoint);
    p.launchVelocity={1,0};assert(!p.drawEffect(gun,.3,effectRandom,allocate));
    SpecialDamageController status;
    SpecialDamageApplication plasmaStatus{WeaponType::railgun,12,1,0,5};
    assert(!status.add(plasmaStatus));assert(status.orderedEntries().size()==1);
    auto sr=status.tick(0,effectRandom,allocate);assert(!sr.checked&&sr.flags==0);
    sr=status.tick(.1,effectRandom,allocate);assert(sr.checked&&sr.flags==1&&sr.events.empty());
    sr=status.tick(1,effectRandom,allocate);assert(sr.events.empty()); // strict delay bound
    int applied=0;
    sr=status.tick(1.2,effectRandom,[](){return false;},[&](const auto& e){applied+=e.damage;});
    assert(sr.events.size()==1&&sr.events[0].dealsDamage&&!sr.events[0].hasParticle&&applied==12);
    auto stronger=plasmaStatus;stronger.damage=99;stronger.damageTime=1.3;stronger.endTime=3;
    status.add(stronger);assert(status.orderedEntries()[0].damage==12&&status.orderedEntries()[0].endTime==3);
    sr=status.tick(3.1,effectRandom,allocate);assert(sr.flags==1&&sr.events.empty()&&status.orderedEntries().empty());
    sr=status.tick(3.3,effectRandom,allocate);assert(sr.flags==0);
    SpecialDamageApplication empStatus{WeaponType::emp,40,1,0,5};
    SpecialDamageController emp;assert(emp.add(empStatus));sr=emp.tick(.5,effectRandom,allocate);assert(sr.events.empty());
    sr=emp.tick(.7,effectRandom,allocate);assert(sr.flags==2&&sr.events.size()==1&&!sr.events[0].dealsDamage&&sr.events[0].particleAlpha==.5f);
    // Original cumulative flags expose the NSDictionary enumeration order.
    SpecialDamageController plasmaFirst;plasmaFirst.add(plasmaStatus);plasmaFirst.add(empStatus);
    SpecialDamageController empFirst;empFirst.add(empStatus);empFirst.add(plasmaStatus);
    const auto a=plasmaFirst.tick(1.1,effectRandom,allocate),b=empFirst.tick(1.1,effectRandom,allocate);
    assert(a.flags==3&&b.flags==3&&a.events.size()==2&&b.events.size()==2);
    assert(a.events[0].dealsDamage&&a.events[1].dealsDamage);
    assert(!b.events[0].dealsDamage&&b.events[1].dealsDamage);
    DarwinRandom poolRandom(5),poolExpected(5);ParticlePool pool(0,poolRandom);
    for(int i=0;i<256;++i)poolExpected.next();assert(poolRandom.next()==poolExpected.next());
    auto* part=pool.acquire(0,poolRandom);assert(part);poolExpected.next();assert(poolRandom.next()==poolExpected.next());
    part->life=1;part->velocity={2,3};part->gravity=1;part->velocityAdd=-.05f;
    part->scaleVelocity=.1f;part->rotationVelocity=.2f;part->start(0);part->tick(.25f);
    assert(part->position.x==2&&near(part->position.y,3-.0166f));
    assert(near(part->scale.x,1.2f)&&part->angle==.2f&&part->alpha==.75f);
    assert(near(part->velocity.x,1.9f));
    auto sprite=part->sprite("effect.png",{128,128},4,4,{40,50});assert(sprite&&sprite->sourcePx->width==32&&sprite->blend==Blend::additive);
    part->setTileAnimation(8);part->tick(.5f);assert(part->tile==4&&part->alpha==.75f);
    part->tick(1);assert(!part->shown&&!part->sprite("effect.png",{128,128},4,4,{40,50}));
    for(auto& item:pool.particles())item.shown=true;
    assert(!pool.acquire(2,poolRandom));assert(poolRandom.next()==poolExpected.next());
    std::cout<<"Projectile original movement and collision checks passed\n";
}
