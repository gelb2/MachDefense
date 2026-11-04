#include "Core/Simulation/Battle.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace md {
void BattleModels::appendSprites(Frame& frame,std::string_view name,Vec2 origin,float scale,float bodyAngle,float weaponAngle,float radarAngle,const std::function<Vec2(std::string_view)>& imageSize) const {
    const auto poses=pose(name,scale,bodyAngle,weaponAngle,radarAngle);
    appendSprites(frame,poses,origin,imageSize);
}
void BattleModels::appendSprites(Frame& frame,std::span<const PartPose> poses,Vec2 origin,const std::function<Vec2(std::string_view)>& imageSize) const {
    std::vector<Sprite> images,shadows;
    for(const auto& pose:poses) {
        const float scale=pose.deviceScale*pose.visualScale;
        const auto* p=pose.definition;Vec2 size=imageSize(p->image);
        // Missing dummy textures return nil in ResMgr_Tile; their sockets still attach children.
        if(size.x<=0 || size.y<=0) continue;
        Vec2 center{origin.x+pose.localCenter.x,origin.y-pose.localCenter.y};
        float w=size.x/p->splitX,h=size.y/p->splitY;
        Sprite image;image.image=p->image;image.sourcePx=Rect{float(p->tileNo%p->splitX)*w,float(p->tileNo/p->splitX)*h,w,h};
        image.destination={center.x-w*scale*.5f,center.y-h*scale*.5f,w*scale,h*scale};
        image.rotationDegrees=-pose.angle*180.f/3.14159265358979323846f;
        image.flipX=(pose.reverse&1)!=0;image.flipY=(pose.reverse&2)!=0;
        Sprite shadow=image;float shadowLength=(p->type==2?2.f:4.f)*scale;
        shadow.destination.x+=shadowLength;shadow.destination.y+=shadowLength;shadow.color={0,0,0,1};
        images.push_back(std::move(image));shadows.push_back(std::move(shadow));
    }
    for(auto& sprite:shadows) frame.sprites.push_back(std::move(sprite));
    for(auto& sprite:images) frame.sprites.push_back(std::move(sprite));
}

}
