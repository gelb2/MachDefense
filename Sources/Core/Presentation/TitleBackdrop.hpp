#pragma once
#include "Core/Contracts/FrameLayout.hpp"

namespace md {
// Keep the shipped title intact. Only the area outside its rectangle gets
// extension art; a short outer feather joins it without editing the source.
inline void appendTitleBackdrop(Frame& frame, Sprite title, const FrameLayout& layout) {
    const auto c=layout.center();
    auto piece=[&](std::string image,Rect src,Rect dst){
        if(dst.width<=0||dst.height<=0)return;
        Sprite s;s.image=std::move(image);s.sourcePx=src;s.destination=dst;
        s.blend=Blend::opaque;frame.sprites.push_back(std::move(s));
    };
    // Each candidate was painted around a registered 320x480 original crop.
    // Sample only the newly painted margins. Smaller windows crop those margins.
    constexpr float wideW=1586,wideH=992,wideScale=480/wideH;
    constexpr float left=wideW*7/24,right=wideW*17/24;
    const float leftWidth=std::min(left,c.x/wideScale);
    const float rightWidth=std::min(wideW-right,(layout.width-c.x-320)/wideScale);
    piece("Generated/Title/Wide.png",{left-leftWidth,0,leftWidth,wideH},{0,c.y,c.x,480});
    piece("Generated/Title/Wide.png",{right,0,rightWidth,wideH},{c.x+320,c.y,layout.width-c.x-320,480});
    constexpr float tallW=852,tallH=1846,tallScale=320/tallW;
    constexpr float upper=(tallH-480/tallScale)/2,lower=tallH-upper;
    const float topHeight=std::min(upper,c.y/tallScale);
    const float bottomHeight=std::min(upper,(layout.height-c.y-480)/tallScale);
    piece("Generated/Title/Tall.png",{0,upper-topHeight,tallW,topHeight},{0,0,layout.width,c.y});
    piece("Generated/Title/Tall.png",{0,lower,tallW,bottomHeight},{0,c.y+480,layout.width,layout.height-c.y-480});
    constexpr float feather=8;
    const float x[]={c.x-feather,c.x,c.x+320,c.x+320+feather};
    const float y[]={c.y-feather,c.y,c.y+480,c.y+480+feather};
    // Half-texel clamping avoids sampling the transparent POT padding.
    const float u[]={.5f/640,.5f/640,1-.5f/640,1-.5f/640};
    const float v[]={.5f/960,.5f/960,1-.5f/960,1-.5f/960};
    for(int row=0;row<3;++row)for(int col=0;col<3;++col){
        if(row==1&&col==1)continue;
        TextureMesh edge;edge.image=title.image;edge.beforeSpriteIndex=frame.sprites.size();
        for(const auto corner:{std::pair{0,0},std::pair{0,1},std::pair{1,0},std::pair{1,1}}){
            const auto ix=col+corner.first,iy=row+corner.second;
            const float a=(ix==0||ix==3||iy==0||iy==3)?0.f:1.f;
            edge.vertices.push_back({{x[ix],y[iy]},{u[ix],v[iy]},{1,1,1,a}});
        }
        frame.meshes.push_back(std::move(edge));
    }
    title.destination.x=c.x;title.destination.y=c.y;
    title.sourcePx=Rect{.5f,.5f,639,959};
    frame.sprites.push_back(std::move(title));
}
}
