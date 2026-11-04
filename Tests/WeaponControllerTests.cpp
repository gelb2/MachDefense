#include "../Sources/Core/WeaponController.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    md::WeaponDefinition weapon;
    weapon.shotCount=3;weapon.shotDelay=.25f;weapon.reloadDelay=1;weapon.shotRange=200;weapon.aimRate=0;
    weapon.type=md::WeaponType::gun;weapon.fireSound="Gun.wav";
    md::WeaponController controller;md::DarwinRandom random(1);
    md::WeaponFrame frame;frame.target=md::WeaponTarget{{100,0},100};
    std::vector<md::WeaponFire> events;auto fire=[&](const md::WeaponFire& event){events.push_back(event);};
    frame.time=.999;assert(controller.tick(weapon,frame,random,fire).shots==0);
    frame.time=1;auto result=controller.tick(weapon,frame,random,fire);
    assert(result.shots==1 && controller.shotsLeft()==2 && result.advanceMuzzle && result.playFireSound);
    assert(events.back().directDamage && events.back().destination.x==100 && events.back().destination.y==0);
    assert(result.recoil==-2.25f);
    frame.time=1.249;assert(controller.tick(weapon,frame,random,fire).shots==0);
    frame.time=1.25;assert(controller.tick(weapon,frame,random,fire).shots==1);
    frame.time=5;assert(controller.tick(weapon,frame,random,fire).shots==1); // No delayed burst catch-up.
    frame.time=5.999;assert(controller.tick(weapon,frame,random,fire).shots==0);
    frame.time=6;assert(controller.tick(weapon,frame,random,fire).shots==1);
    controller.setReload(10);frame.time=10.5;assert(controller.tick(weapon,frame,random,fire).shots==0);
    frame.time=11;frame.statusFlags=2;assert(controller.tick(weapon,frame,random,fire).shots==0);
    frame.statusFlags=0;frame.weaponReady=false;assert(controller.tick(weapon,frame,random,fire).shots==0);
    frame.weaponReady=true;frame.target->position.x=200;assert(controller.tick(weapon,frame,random,fire).shots==0);
    frame.target->position.x=199;assert(controller.tick(weapon,frame,random,fire).shots==1);
    md::WeaponController simultaneous;weapon.shotDelay=0;frame.time=1;
    auto batch=simultaneous.tick(weapon,frame,random,fire);assert(batch.shots==3 && simultaneous.shotsLeft()==0);
    frame.target.reset();frame.manualFire=false;frame.time=100;assert(simultaneous.tick(weapon,frame,random,fire).shots==0);
    frame.manualFire=true;weapon.type=md::WeaponType::missile;
    auto manual=simultaneous.tick(weapon,frame,random,fire);assert(manual.shots==3);
    auto p=events.back();float length=std::hypot(p.destination.x-p.origin.x,p.destination.y-p.origin.y);
    assert(length>=160 && length<=200 && !p.directDamage && !p.hasTarget);
    md::WeaponController tracker;md::WeaponFrame tracking;
    tracking.target=md::WeaponTarget{{0,100},100};tracking.weaponReady=true;
    assert(!tracker.track(tracking));assert(std::abs(tracker.angle()-float(std::acos(-1)/20))<1e-6f);
    for(int i=0;i<100;++i) tracking.weaponReady=tracker.track(tracking);
    assert(tracking.weaponReady);
    tracking.machineType=0;tracking.weaponReady=false;assert(!tracker.track(tracking));
    md::WeaponController idle;tracking.target.reset();tracking.time=0;tracking.bodyAngle=0;
    idle.track(tracking);assert(idle.angle()==.0625f);
    std::cout<<"Weapon cadence, reload, range, aim, manual fire and recoil checks passed\n";
}
