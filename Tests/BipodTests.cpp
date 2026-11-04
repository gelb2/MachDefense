#include "Bipod.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
namespace { void near(float a,float b,float tolerance=.00001f){if(!(std::abs(a-b)<tolerance)){std::cerr<<"actual "<<a<<" expected "<<b<<"\n";assert(false);}} }
int main(){
    md::BipodMotion walk;walk.setStepSize(10);assert(walk.speedPoint>0);
    walk.setDestination({100,0});walk.tick();assert(walk.phase==3&&walk.rightFootActive);near(walk.position.x,0);
    // Original 0x12480/0x1281e: step +1/8; filtered advance +1/6.
    walk.tick();near(walk.stepPosition,1.25f);near(walk.bodyAdvance,1.25f/6);
    near(walk.position.x,1.25f/6);near(walk.feet[1].addPosX,1.25f-1.25f/6);near(walk.feet[0].addPosX,-1.25f/6);
    near(walk.bodyAddPosY,-.125f);assert(walk.feet[1].scale>1&&walk.bodyScale>1);
    bool sawLeft=false;for(int i=0;i<1000;++i){walk.tick();sawLeft|=!walk.rightFootActive;if(walk.state==0)break;}
    assert(sawLeft&&walk.state==0&&std::abs(walk.position.x-100)<5);
    const float previous=walk.bodyAdvance,oldStep=walk.stepPosition;walk.tick();
    near(walk.stepPosition,oldStep+(0-oldStep)/6);near(walk.bodyAdvance,previous+(walk.stepPosition-previous)/6);

    md::BipodMotion aiming;aiming.tick(md::Vec2{100,0});assert(aiming.weaponReady);
    near(aiming.leftWeaponAngle,-.125f);near(aiming.rightWeaponAngle,.125f);near(aiming.baseAngle,0);
    aiming.tick(md::Vec2{200,0});near(aiming.leftWeaponAngle,0);near(aiming.rightWeaponAngle,0);
    aiming.bodyAngle=0;aiming.tick(md::Vec2{0,100});assert(!aiming.weaponReady);near(aiming.bodyAngle,float(3.141592653589793/20));
    for(int i=0;i<40;++i)aiming.tick(md::Vec2{0,100});assert(aiming.weaponReady);
    md::BipodMotion planted;planted.plantedTurn=true;planted.setDestination({-100,0});planted.tick();assert(planted.phase==4);near(planted.angle,float(2*3.141592653589793)-.5f);
    for(int i=0;i<100;++i)planted.tick();near(planted.position.x,0);near(planted.position.y,0);
    md::BipodMotion step;step.setStepSize(0);assert(step.speedPoint==0);step.setStepSize(10);const auto p=step.speedPoint;
    step.setStepSize(20);assert(step.speedPoint>p);

    // With no candidate in the 160*scale look-ahead, preserve caller's target.
    auto clear=md::checkPlayerMoveAhead({0,0},{0,30},2,.5f,{});near(clear.y,30);
    const md::AlliedObstacle outside{{30,20},1,100};
    auto projected=md::checkPlayerMoveAhead({0,0},{0,30},2,.5f,std::span(&outside,1));near(projected.y,80);
    const md::AlliedObstacle blocker{{0,30},8,100};
    auto avoided=md::checkPlayerMoveAhead({0,0},{0,80},2,.5f,std::span(&blocker,1));
    // 0 and +/-0.2 are obstructed; +0.4 is the first clear direction.
    near(avoided.x,-std::sin(.4f)*80);near(avoided.y,std::cos(.4f)*80);
    const md::AlliedObstacle wall{{0,30},1000,100};
    auto retained=md::checkPlayerMoveAhead({0,0},{0,40},2,.5f,std::span(&wall,1));near(retained.x,0);near(retained.y,40);
    const md::AlliedObstacle dead{{0,30},1000,0};
    auto ignored=md::checkPlayerMoveAhead({0,0},{0,40},2,.5f,std::span(&dead,1));near(ignored.y,40);

    md::PlayerSteering ai;md::PlayerMovementFrame f;f.time=.25;f.baseBoundary=1000;f.targetRange=100;
    md::DarwinRandom rng(1),reference(1);float u=float(reference.next())*0x1p-31f;
    auto result=ai.tick(f,rng);assert(result);near(result->x,u*16-8);near(result->y,80);
    const float delay=float(f.time+double(float(reference.next())*0x1p-31f*2)+1);near(ai.stopDelay,delay);assert(rng.next()==reference.next());
    assert(!ai.tick(f,rng));f.time=10;f.hasTarget=f.moveToTarget=true;assert(!ai.tick(f,rng));
    ai.isOrder=true;f.position={10,10};f.destination={13,14};auto ordered=ai.tick(f,rng);assert(!ordered&&!ai.isOrder);near(ai.topPlayerPosition,10);
    md::PlayerSteering boundary;f={};f.time=10;f.position={55,10};f.baseBoundary=100;f.targetRange=100;
    result=boundary.tick(f,rng);assert(result&&result->x<=55);near(result->y,20);
    std::cout<<"Bipod phases, alternating feet, aim/convergence, player steering and ally avoidance passed\n";
}
