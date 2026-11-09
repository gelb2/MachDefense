#include "Core/Simulation/AircraftTactics.hpp"
#include <array>
#include <cassert>
#include <iostream>

using namespace md;

int main(){
    // Existing four aircraft ranges, cruise speeds and body radii at phone scale.
    const std::array<std::array<float,4>,4> specs{{
        {.5f,150.f,1.f,6.f},{.425f,225.f,.5f,14.f},
        {.425f,140.f,1.05f,7.f},{.25f,255.f,.95f,7.f}}};
    unsigned cases=0,bad=0;
    for(const auto& s:specs)for(int mode=0;mode<7;++mode)
    for(unsigned seed=1;seed<=32;++seed)for(int direction=0;direction<8;++direction){
        AircraftTactics t;t.widthFraction=s[0];t.random=seed;
        AircraftMotion m;m.moveSpeed=s[2];
        Vec2 target=mode==1?Vec2{105,400}:mode==3?Vec2{-105,400}:mode==4?Vec2{105,-400}:mode==5?Vec2{-105,-400}:mode==6?Vec2{0,70}:Vec2{};
        const float angle=direction*6.28318530718f/8;
        m.position={std::clamp(target.x+std::sin(angle)*s[1]*.9f,-128+s[3],128-s[3]),
                    std::clamp(target.y+std::cos(angle)*s[1]*.9f,-512+s[3],512-s[3])};
        // This checks approaches from separated positions, not already-overlapping spawns.
        if(std::hypot(m.position.x-target.x,m.position.y-target.y)<2*s[3])continue;
        float nearest=1e9f;
        for(unsigned tick=0;tick<1800;++tick){
            if(mode==2)target.x=40*std::sin(tick*.001f);
            if(mode==6)target={70*std::sin(tick*.25f/70),70*std::cos(tick*.25f/70)};
            m.setDestination(t.destination(tick/60.,m.position,target,s[1],256,512,s[3]));
            t.advance(m);
            nearest=std::min(nearest,std::hypot(m.position.x-target.x,m.position.y-target.y));
            assert(std::isfinite(m.position.x)&&std::isfinite(m.position.y));
        }
        if(nearest<2*s[3]){++bad;std::cout<<"CONTACT mode="<<mode<<" speed="<<s[2]<<" seed="<<seed<<" dir="<<direction<<" min="<<nearest<<" radius="<<s[3]<<"\n";}++cases;
    }
    // A moving clock point must not prevent normal orbit phase changes forever.
    AircraftTactics t;t.widthFraction=.5f;t.random=1;
    AircraftMotion m;m.moveSpeed=1;m.position={95.45942f,95.45942f};
    for(unsigned tick=0;tick<12000;++tick){
        Vec2 target{70*std::sin(tick*.5f/70),70*std::cos(tick*.5f/70)};
        m.setDestination(t.destination(tick/60.,m.position,target,150,256,512,6));t.advance(m);
    }
    assert(t.leg>20&&!t.enteringOrbit);
    // Target loss, switch and completed evasion use the existing engagement reset.
    for(int reset=0;reset<3;++reset){
        if(reset==2)t.finishDodge(201.);else t.engaged=false;
        const auto before=t.leg;
        auto p=t.destination(202.,{0,-135},{0,0},150,256,512,6);
        assert(t.engaged&&t.enteringOrbit&&t.leg==before+1);
        assert(std::hypot(p.x,p.y)>50);
    }
    std::cout<<"contacts="<<bad<<" cases="<<cases<<" separated static/edge/moving approaches; moving orbit phases and three engagement resets passed\n";
}
