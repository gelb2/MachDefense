#pragma once
#include "RenderTypes.hpp"
#include <algorithm>
#include <cmath>
namespace md {
// UI art keeps its original size. These translations apply to both graphics
// and input; world coordinates and BattleGeometry never use this structure.
struct FrameLayout {
    float width=320,height=480,top=0,bottom=0;
    void resize(float w,float h,float t=0,float b=0){
        if(!std::isfinite(w)||!std::isfinite(h)||w<=0||h<=0)return;
        t=std::clamp(t,0.f,h*.2f);b=std::clamp(b,0.f,h*.2f);
        const float scale=std::min(w/320.f,(h-t-b)/480.f);
        width=w/scale;height=h/scale;top=t/scale;bottom=b/scale;
    }
    Vec2 topRight()const{return {width-320,top};}
    Vec2 bottomRight()const{return {width-320,height-480-bottom};}
    Vec2 center()const{return {(width-320)/2,top+(height-top-bottom-480)/2};}
};
inline Touch translatedTouch(Touch t,Vec2 offset){t.position.x-=offset.x;t.position.y-=offset.y;return t;}
inline void translateFrame(Frame& f,std::size_t sprites,std::size_t meshes,Vec2 d){
    for(auto i=sprites;i<f.sprites.size();++i){auto& s=f.sprites[i];s.destination.x+=d.x;s.destination.y+=d.y;if(s.clip){s.clip->x+=d.x;s.clip->y+=d.y;}}
    for(auto i=meshes;i<f.meshes.size();++i)for(auto& v:f.meshes[i].vertices){v.position.x+=d.x;v.position.y+=d.y;}
}
}
