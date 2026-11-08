#pragma once
#include "Core/Simulation/Projectile.hpp"
#include "Core/Simulation/Trail.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace md {
// Visual-only bounded storage. Uses the original particle tick and trail mesh;
// never borrows combat particle slots or consumes the world's combat RNG.
class AircraftCountermeasures {
public:
    static constexpr unsigned flaresPerSide=7, flareCount=2*flaresPerSide, burstSize=flareCount+8, capacity=6*burstSize;
    explicit AircraftCountermeasures(float scale):scale_(scale){
        WeaponDefinition trail;trail.effectType=5;trail.effectImage="Efx_Trail01.png";
        for(auto& e:emitters_)e.trail=Trail::forWeapon(trail,scale,{});
    }
    void clear(){for(auto& e:emitters_)e.active=false;active_=0;}
    unsigned active()const{return active_;}
    unsigned bursts()const{return bursts_;}
    bool emit(float now,Vec2 at,float heading,float width){
        if(active_+burstSize>capacity)return false;
        unsigned n=0;
        for(auto& e:emitters_){if(e.active)continue;
            e.active=true;e.kind=n<flareCount?0:n<flareCount+4?1:n<flareCount+6?2:3;
            e.release=now+1.65f;e.turn=0;
            auto& p=e.particle;p.reset(now,random_);p.sinRotate=0;p.easyOutDivisor=0;
            const float side=n%2?1.f:-1.f;
            auto rotate=[&](Vec2 v){return Vec2{v.x*std::cos(heading)-v.y*std::sin(heading),v.x*std::sin(heading)+v.y*std::cos(heading)};};
            auto offset=rotate({-width*.28f,side*width*.27f});p.position={at.x+offset.x,at.y+offset.y};
            p.start(now+(n<flareCount?.025f*float(n/2):0));
            if(e.kind==0){
                const float tier=float(n/2);p.velocity=rotate({-width*(.004f+.0035f*tier),side*width*(.030f-.002f*tier)});
                e.turn=side*.006f;p.life=.88f;p.tile=1;p.scale={.48f,.32f};
                e.trail->restart(p.position);
            }else if(e.kind==1){
                p.velocity=rotate({-width*(.012f+.003f*float(n-flareCount)),side*width*.013f});
                p.life=.95f;p.tile=1;p.scale={.34f,.055f};p.rotationVelocity=side*.08f;p.angle=heading+side*.6f;
            }else if(e.kind==2){p.life=.11f;p.tile=2;p.scale={.9f,.65f};}
            else{p.life=.8f;p.tile=13;p.scale={.34f,.34f};p.scaleVelocity=.006f;p.blend=Blend::alpha;p.maxAlpha=.32f;p.velocity=rotate({-width*.003f,side*width*.003f});}
            if(++n==burstSize)break;
        }
        active_+=burstSize;++bursts_;return true;
    }
    void tick(float now){
        if(!active_)return;
        for(auto& e:emitters_)if(e.active){
            if(now>=e.release){e.active=false;--active_;continue;}
            auto& p=e.particle;if(now<p.startTime)continue;
            if(p.shown){
                if(e.turn!=0){auto v=p.velocity;const float c=std::cos(e.turn),s=std::sin(e.turn);p.velocity={v.x*c-v.y*s,v.x*s+v.y*c};}
                p.tick(now);
                if(e.kind==0&&p.shown)e.trail->add(p.position,now,random_);
            }
            if(e.kind==0)e.trail->tick(now,random_);
        }
    }
    void append(Frame& frame,float now,const std::function<Vec2(Vec2)>& screen,const std::function<Vec2(std::string_view)>& size)const{
        if(!active_)return;
        for(const auto& e:emitters_)if(e.active&&now>=e.particle.startTime){
            if(e.kind==0)e.trail->appendMesh(frame,screen);
            const char* image=e.kind==3?"Efx_Smoke.png":"Efx_Fire.png";
            if(auto sprite=e.particle.sprite(image,size(image),4,4,screen(e.particle.position),scale_)){
                // Existing fire atlas is white/silver. Keep only a faint ivory tint.
                if(e.kind==0||e.kind==2){sprite->color.r=1;sprite->color.g=.97f;sprite->color.b=.91f;}
                frame.sprites.push_back(std::move(*sprite));
            }
        }
    }
private:
    struct Emitter{ParticleState particle;std::optional<Trail> trail;float release=0,turn=0;unsigned kind=0;bool active=false;};
    std::array<Emitter,capacity> emitters_;
    DarwinRandom random_{0x73696c76};float scale_;unsigned active_=0,bursts_=0;
};
}
