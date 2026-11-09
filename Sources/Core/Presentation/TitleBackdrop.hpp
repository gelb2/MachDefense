#pragma once
#include "Core/Contracts/FrameLayout.hpp"

namespace md {
// These prepared textures contain the adopted report composition. Only the
// rectangles outside its original 640x960 title are sampled. All color matching
// is baked by prepare_title_art.mjs; no pixel work or upload occurs per frame.
inline void appendTitleBackdrop(Frame& frame, Sprite title, const FrameLayout& layout) {
    const auto c=layout.center();
    auto margin=[&](const char* image,Rect src,Rect dst,float anchorX,float anchorY){
        if(dst.width<=.001f||dst.height<=.001f)return;
        // Uniform cover crops scenery for other aspect ratios, never stretches it.
        const float scale=std::max(dst.width/src.width,dst.height/src.height);
        const float sw=dst.width/scale,sh=dst.height/scale;
        src.x+=(src.width-sw)*anchorX;src.y+=(src.height-sh)*anchorY;
        src.width=sw;src.height=sh;
        Sprite s;s.image=image;s.sourcePx=src;s.destination=dst;
        s.blend=Blend::opaque;frame.sprites.push_back(std::move(s));
    };
    // Wide: 3200x900, original rectangle [1300,0,600,900].
    // The adopted 1600x900 is preserved at x=800. Extra natural scenery
    // supports 32:9 while keeping the full 900px vertical registration.
    margin("Generated/Title/Wide.png",{0,0,1300,900},{0,0,c.x,layout.height},1,.5f);
    margin("Generated/Title/Wide.png",{1900,0,1300,900},{c.x+320,0,layout.width-c.x-320,layout.height},0,.5f);
    // Tall: 390x844, original rectangle [0,129.5,390,585].
    margin("Generated/Title/Tall.png",{0,0,390,129.5f},{c.x,0,320,c.y},.5f,1);
    margin("Generated/Title/Tall.png",{0,714.5f,390,129.5f},{c.x,c.y+480,320,layout.height-c.y-480},.5f,0);
    // Clamp only the outside half-texel border of the original. The interior
    // source-to-destination mapping stays 2:1, unlike shrinking the full UV box.
    // This prevents transparent POT padding from darkening fractional edges.
    const float dx[]={c.x,c.x+.25f,c.x+319.75f,c.x+320};
    const float dy[]={c.y,c.y+.25f,c.y+479.75f,c.y+480};
    const float sx[]={.5f,.5f,639.5f,639.5f};
    const float sy[]={.5f,.5f,959.5f,959.5f};
    for(int row=0;row<3;++row)for(int col=0;col<3;++col){
        auto piece=title;piece.sourcePx=Rect{sx[col],sy[row],sx[col+1]-sx[col],sy[row+1]-sy[row]};
        piece.destination={dx[col],dy[row],dx[col+1]-dx[col],dy[row+1]-dy[row]};
        frame.sprites.push_back(std::move(piece));
    }
}
}
