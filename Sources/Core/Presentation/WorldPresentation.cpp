#include "Core/Simulation/AircraftVisual.hpp"
#include "Core/Simulation/World.hpp"
#include "Core/Simulation/Formation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>

#include "Core/Simulation/WorldSupport.hpp"
namespace md {
using namespace world_detail;
void World::setViewport(float w,float h,float top){
    if(!std::isfinite(w)||!std::isfinite(h)||w<=0||h<=0)return;
    const bool bottom=onBottom_,upper=onTop_;
    viewWidth_=w;viewHeight_=h;safeTop_=top;
    // Fit the original 256-unit battlefield up to the 64-unit production HUD.
    // This changes presentation only; collision, weapons and save use the
    // original BattleGeometry. Phone-width layouts keep scale 1.
    presentationScale_=geometry_.deviceScale==.5f?std::max(1.f,(w-64.f)/256.f):1.f;
    cameraTouch_.reset();radarTouch_.reset();cameraVelocity_=0;
    if(bottom)camera_=height()*.5f-bottomMargin()-geometry_.halfLength();
    else if(upper)camera_=geometry_.halfLength()-height()*.5f;
    clampCamera();actualCamera_=camera_;
}
void World::setUIMargin(float value){
    uiMargin_=value;
    const float lower=height()*.5f-bottomMargin()-geometry_.halfLength();
    if(onBottom_)camera_=lower;
    else if(camera_<lower)onBottom_=true;
}
void World::clampCamera(){
    onBottom_=onTop_=true;float upper=geometry_.halfLength()-height()*.5f;
    if(camera_<=upper)onTop_=false;else camera_=upper;
    float lower=height()*.5f-bottomMargin()-geometry_.halfLength();
    if(lower>upper){camera_=(lower+upper)*.5f;return;}
    if(camera_<lower)camera_=lower;else onBottom_=false;
}
void World::updateCamera(){
    if(!cameraTouch_&&cameraVelocity_!=0){cameraVelocity_=std::abs(cameraVelocity_)>.01f?cameraVelocity_+(0-cameraVelocity_)/10.f:0;camera_+=cameraVelocity_;clampCamera();}
    if(onBottom_){cameraVelocity_=0;actualCamera_=camera_;}
    else if(actualCamera_!=camera_)actualCamera_=std::abs(actualCamera_-camera_)>.5f?actualCamera_+(camera_-actualCamera_)/5.f:camera_;
}
Vec2 World::worldToView(Vec2 p)const{return {p.x-cameraX()+width()*.5f,height()*.5f-(p.y-actualCamera_)};}
Vec2 World::worldToScreen(Vec2 p)const{const auto v=worldToView(p);return {v.x*presentationScale_,v.y*presentationScale_};}
Vec2 World::screenToWorld(Vec2 p)const{return {cameraX()-width()*.5f+p.x/presentationScale_,actualCamera_+height()*.5f-p.y/presentationScale_};}
std::optional<Vec2> World::touch(const Touch& t){
    if(!started_||paused_||state_!=WorldState::running)return {};
    const float x=t.position.x,y=height()*presentationScale_-t.position.y;
    const bool phone=geometry_.deviceScale==.5f;Vec2 radar=phone?Vec2{18,height()*presentationScale_-82-safeTop_}:Vec2{55,824};Vec2 half=phone?Vec2{16,64}:Vec2{32,128};
    if(t.phase==TouchPhase::began){
        if(std::abs(x-radar.x)<half.x&&std::abs(y-radar.y)<half.y){radarTouch_=t.id;camera_=(y-radar.y)*8;clampCamera();}
        else if(x>=0&&x<(phone?256.f*presentationScale_:640.f)&&y>uiMargin_&&!cameraTouch_){cameraTouch_=t.id;dragY_=y;onlyTap_=true;}
    }else if(radarTouch_==t.id){camera_=(y-radar.y)*8;clampCamera();if(t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled)radarTouch_.reset();}
    else if(cameraTouch_==t.id){
        if(onlyTap_){if(std::abs(y-dragY_)>16)onlyTap_=false;}
        else {cameraVelocity_=(dragY_-y)/presentationScale_;camera_+=cameraVelocity_;clampCamera();dragY_=y;}
        if(t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled){cameraTouch_.reset();if(onlyTap_&&t.phase==TouchPhase::ended)return screenToWorld(t.position);if(std::abs(cameraVelocity_)<8)cameraVelocity_=0;}
    }return {};
}
void World::appendSprites(Frame& frame)const{
    if(!started_)return;
    const auto firstSprite=frame.sprites.size(),firstMesh=frame.meshes.size();
    const float scale=geometry_.deviceScale;
    for(int part=0;part<2;++part){Sprite s;s.image=(scale==.5f?"iPon_":"")+background_+(part==0?"_01.jpg":"_02.jpg");Vec2 center=worldToView({0,float(part==0?-512:512)*scale});s.destination={center.x-256*scale,center.y-512*scale,512*scale,1024*scale};s.blend=Blend::opaque;frame.sprites.push_back(std::move(s));}
    // QobManager::drawVisual visits layers 0..13. Part shadows use layer 0;
    // child parts use the machine layer + 1 (GobHvMach::setLayer 0x14430).
    std::array<Frame,10> layers;
    for(const auto& a:actors_)if(!a->removed){
        const auto poses=models_.pose(a->assembly,scale,a->angle+a->artHeadingOffset,a->angle,a->radarAngle);
        const int baseLayer=a->hp<=0?1:(a->flightHeight>0?7:(a->base?(a->player?7:5):(a->player?3:1)));
        for(const auto& pose:poses){Frame part;models_.appendSprites(part,std::span<const PartPose>(&pose,1),worldToView(a->position),[&](std::string_view n){return resources_.imageSize(n);});
            if(part.sprites.size()==2){
                if(a->flightHeight>0){
                    // The silhouette follows the same moving part on the ground.
                    auto& shadow=part.sprites[0];shadow.destination.x+=a->flightHeight*scale;shadow.destination.y+=a->flightHeight*scale;shadow.color.a=.55f;
                    shadow.nearest=part.sprites[1].nearest=true;

                }
                const int layer=baseLayer+(a->hp>0&&pose.hasParent?1:0);
                if(a->flightTactics&&a->hp<=0){
                    // Grounded wreck: no altitude offset; only a faint contact shadow.
                    auto& sh=part.sprites[0];const auto& body=part.sprites[1];sh.destination=body.destination;
                    sh.destination.x+=scale;sh.destination.y+=scale;sh.color.a=.18f;
                }
                const float bank=a->flightTactics&&a->hp>0?a->flightTactics->bank:0;
                if(a->flightTactics&&std::abs(bank)>.001f){
                    const auto origin=worldToView(a->position);
                    // Use the preserved body texture throughout a roll. Switching
                    // to a separately generated pose changed the approved design.
                    for(unsigned i=0;i<2;++i){auto& dst=layers[i?layer:0];auto pivot=origin;
                        if(!i){pivot.x+=a->flightHeight*scale+4*pose.deviceScale;pivot.y+=a->flightHeight*scale+4*pose.deviceScale;}
                        auto mesh=aircraftBankMesh(part.sprites[i],pivot,a->angle,bank,resources_.imageSize(part.sprites[i].image));
                        mesh.beforeSpriteIndex=dst.sprites.size();dst.meshes.push_back(std::move(mesh));
                    }
                }else{layers[0].sprites.push_back(std::move(part.sprites[0]));layers[layer].sprites.push_back(std::move(part.sprites[1]));}
            }
        }
    }
    for(const auto& item:loot_)item.appendSprites(layers[0],worldToView(item.position),[&](std::string_view n){return resources_.imageSize(n);});
    auto particles=particles_->particles();for(std::size_t i=0;i<particles.size();++i){const auto& p=particles[i];const auto& image=particleImages_[i];if(!p.shown||image.image.empty())continue;const float imageScale=image.assetScale?image.assetScale:scale;if(auto s=p.sprite(image.image,resources_.imageSize(image.image),image.columns,image.rows,worldToView({p.position.x-image.origin.x*p.scale.x*imageScale,p.position.y-image.origin.y*p.scale.y*imageScale}),imageScale))layers[5].sprites.push_back(std::move(*s));}
    struct MovingVisual {std::uint64_t order;const WorldTrail* trail=nullptr;const WorldProjectile* projectile=nullptr;};
    std::vector<MovingVisual> moving;for(const auto& trail:trails_)moving.push_back({trail.renderOrder,&trail,nullptr});for(const auto& p:projectiles_)moving.push_back({p.renderOrder,nullptr,&p});
    std::sort(moving.begin(),moving.end(),[](const auto& a,const auto& b){return a.order<b.order;});
    for(const auto& visual:moving){
        if(visual.trail){visual.trail->motion->appendMesh(layers[visual.trail->layer],[&](Vec2 p){return worldToView(p);});continue;}
        const auto& p=*visual.projectile;const auto* w=weapons_.find(p.weapon);if(!w)continue;
        if(!w->bulletImage.empty()){
        const auto size=resources_.imageSize(w->bulletImage);const unsigned columns=w->bulletImage=="Blt_Bullet.png"?4:1,rows=w->bulletImage=="Blt_Bullet.png"?8:1;
        const float cellWidth=size.x/columns,cellHeight=size.y/rows;
        Sprite s;s.image=w->bulletImage;s.sourcePx=Rect{float(unsigned(w->bulletTile)%columns)*cellWidth,float(unsigned(w->bulletTile)/columns)*cellHeight,cellWidth,cellHeight};
        const Vec2 center=worldToView(p.motion.position);const float wpx=cellWidth*scale*p.motion.imageScale.x,hpx=cellHeight*scale*p.motion.imageScale.y;
        s.destination={center.x-wpx*.5f,center.y-hpx*.5f,wpx,hpx};s.rotationDegrees=-p.motion.angle*180.f/float(3.141592653589793);layers[p.layer].sprites.push_back(std::move(s));
        }
        if(!w->effectImage.empty())if(auto effect=p.motion.attachedEffect(*w,resources_.imageSize(w->effectImage),worldToView(p.motion.position),scale))layers[p.layer].sprites.push_back(std::move(*effect));
    }
    for(const auto& strike:airStrikes_)strike.motion.appendSprite(layers[7],worldToView(strike.motion.position),resources_.imageSize("Airplane.png"),scale);
    for(auto& layer:layers){const auto offset=frame.sprites.size();for(auto& mesh:layer.meshes){mesh.beforeSpriteIndex+=offset;frame.meshes.push_back(std::move(mesh));}for(auto& sprite:layer.sprites)frame.sprites.push_back(std::move(sprite));}

    // Transform every world layer together, including shadows and trail meshes.
    if(presentationScale_!=1){
        const float z=presentationScale_;
        auto scaleRect=[&](Rect& r){r.x*=z;r.y*=z;r.width*=z;r.height*=z;};
        for(auto i=firstSprite;i<frame.sprites.size();++i){auto& s=frame.sprites[i];scaleRect(s.destination);if(s.clip)scaleRect(*s.clip);}
        for(auto i=firstMesh;i<frame.meshes.size();++i)for(auto& v:frame.meshes[i].vertices){v.position.x*=z;v.position.y*=z;}
    }

}

}
