#include "Trail.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace md {
namespace {
float length(Vec2 p){return std::sqrt(p.x*p.x+p.y*p.y);}
Vec2 minus(Vec2 a,Vec2 b){return {a.x-b.x,a.y-b.y};}
Vec2 normalized(Vec2 p){const float n=length(p);return n>0?Vec2{p.x/n,p.y/n}:Vec2{};}
float randomFloat(DarwinRandom& r){return float(r.next())*0x1p-31f;}
void vertices(TrailPoint& p){p.positive={p.position.x+p.normal.x,p.position.y+p.normal.y};p.negative={p.position.x-p.normal.x,p.position.y-p.normal.y};}
}
std::optional<Trail> Trail::forWeapon(const WeaponDefinition& w,float scale,Vec2 origin){
    if(w.effectType<5||w.effectType>7)return {};
    if(scale<=0)throw std::invalid_argument("Invalid trail scale");
    Trail t;t.image_=w.effectImage;t.origin_=origin;t.width_=4*scale;t.length_=16*scale;
    t.diffuse_=.05f*scale;t.delay_=.2f;t.fade_=.5f;t.wind_=20.f/scale;
    if(w.effectType==6){t.wind_=0;t.delay_=.3f;t.fade_=.3f;}
    if(w.effectType==7){t.wind_=40.f/scale;t.diffuse_=.1f*scale;t.width_=32*scale;t.length_=64*scale;t.fade_=.3f;}
    // The original allocation is undersized. Keep its rolling window without
    // reproducing the out-of-bounds scratch endpoint write.
    t.points_.resize(t.capacity_+2);return t;
}
void Trail::addValid(Vec2 point,double time,bool commit,DarwinRandom& random){
    if(last_>capacity_){for(unsigned i=0;i<capacity_;++i)points_[i]=points_[i+1];--last_;}
    auto& p=points_[last_];p.position=point;p.born=float(time);p.v=odd_?0.f:1.f;
    if(last_==0)p.normal={};
    else{
        const Vec2 delta=minus(point,points_[last_-1].position);const float distance=length(delta);
        // At an exact segment endpoint the original divides 0/0. Its scratch
        // point collapses; retaining the preceding normal keeps finite geometry.
        p.normal=distance>0?Vec2{delta.y/distance*width_*.5f,-delta.x/distance*width_*.5f}:points_[last_-1].normal;
        if(commit&&last_>1){
            auto& previous=points_[last_-1];Vec2 next=normalized(minus(point,previous.position));
            Vec2 prior=normalized(minus(points_[last_-2].position,previous.position));
            next.x*=width_;next.y*=width_;prior.x*=width_;prior.y*=width_;
            if(prior.x*next.y-prior.y*next.x>.1f){
                const Vec2 n=normalized({prior.x+next.x,prior.y+next.y});
                const float size=(width_+randomFloat(random)*6.f)*.5f;
                previous.normal={n.x*size,n.y*size};vertices(previous);
            }
        }else{points_[0].normal=p.normal;vertices(points_[0]);}
    }
    vertices(p);p.alpha=1;
    if(commit){++last_;odd_=!odd_;}
}
void Trail::add(Vec2 world,double time,DarwinRandom& random){
    if(removed_)return;const Vec2 p=minus(world,origin_);
    if(last_<1)addValid(p,time,true,random);
    else{
        Vec2 at=points_[last_-1].position;const Vec2 delta=minus(p,at);const float n=length(delta);
        if(n*n>=length_*length_){
            const float count=n/length_;const Vec2 step{delta.x/count,delta.y/count};
            for(float remaining=count;remaining>=1;remaining-=1){at.x+=step.x;at.y+=step.y;addValid(at,time,true,random);}
        }
    }
    addValid(p,time,false,random);
}
void Trail::tick(double time,DarwinRandom& random,bool paused){
    if(paused||removed_)return;bool alive=false;
    for(unsigned i=0;i<=last_;++i){auto& p=points_[i];const float age=float(time-double(p.born));
        if(age>delay_){const float elapsed=age-delay_;p.alpha=elapsed<fade_?1.f-elapsed/fade_:0;if(i==0)p.alpha=0;if(p.alpha==0)continue;}
        alive=true;
        if(diffuse_!=0){const float scale=randomFloat(random)*diffuse_;const Vec2 offset{p.normal.x*scale,p.normal.y*scale};p.positive.x+=offset.x;p.positive.y+=offset.y;p.negative.x-=offset.x;p.negative.y-=offset.y;}
        if(wind_!=0){const float scale=std::sin(p.born*30.f)/(wind_+randomFloat(random)*wind_*.5f);const Vec2 offset{p.normal.x*scale,p.normal.y*scale};p.positive.x+=offset.x;p.positive.y+=offset.y;p.negative.x+=offset.x;p.negative.y+=offset.y;}
    }
    if(last_>=1&&!alive)removed_=true;
}
void Trail::appendMesh(Frame& frame,const std::function<Vec2(Vec2)>& screen)const{
    if(removed_||last_<2)return;TextureMesh mesh;mesh.image=image_;mesh.blend=Blend::additive;mesh.beforeSpriteIndex=frame.sprites.size();
    for(unsigned i=0;i<=last_;++i){const auto& p=points_[i];
        mesh.vertices.push_back({screen({origin_.x+p.positive.x,origin_.y+p.positive.y}),{0,p.v},{1,1,1,p.alpha}});
        mesh.vertices.push_back({screen({origin_.x+p.negative.x,origin_.y+p.negative.y}),{1,p.v},{1,1,1,p.alpha}});
    }frame.meshes.push_back(std::move(mesh));
}
}
