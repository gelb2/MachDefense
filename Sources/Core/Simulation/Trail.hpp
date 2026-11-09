#pragma once
#include "Core/Simulation/Combat.hpp"
#include "Core/Contracts/DarwinRandom.hpp"
#include "Core/Contracts/RenderTypes.hpp"
#include <functional>
#include <algorithm>
#include <optional>
#include <span>
namespace md {
struct TrailPoint {Vec2 position,normal,positive,negative;float born=0,v=0,alpha=1;};
class Trail {
public:
    static std::optional<Trail> forWeapon(const WeaponDefinition&,float deviceScale,Vec2 origin);
    // Reuse prepared storage for visual-only emitters; existing projectile callers are unchanged.
    void restart(Vec2 origin){origin_=origin;last_=0;odd_=false;removed_=false;std::fill(points_.begin(),points_.end(),TrailPoint{});}
    void add(Vec2 worldPosition,double time,DarwinRandom&);
    void tick(double time,DarwinRandom&,bool paused=false);
    void appendMesh(Frame&,const std::function<Vec2(Vec2)>& worldToScreen)const;
    bool removed()const{return removed_;}
    std::span<const TrailPoint> points()const{return {points_.data(),last_+1};}
    unsigned lastPosition()const{return last_;}
    float width()const{return width_;}
    float blockLength()const{return length_;}
private:
    std::string image_;Vec2 origin_;float width_=0,length_=0,delay_=0,fade_=0,wind_=0,diffuse_=0;
    unsigned capacity_=64,last_=0;bool odd_=false,removed_=false;
    std::vector<TrailPoint> points_;
    void addValid(Vec2,double time,bool commit,DarwinRandom&);
};
}
