#pragma once

#include "Core/Contracts/RenderTypes.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

namespace md {

inline constexpr std::string_view crossfireLockStrokeImage = "CrossfireLockStroke.png";

// Adapted from the project-authored HeavyMach 1 HMAimOverlay.hpp and
// HMGameView::drawAimControls (MIT). Y increases downwards in MachDefense.
inline void appendCrossfireLockOverlay(Frame& frame, Rect target, float width, float height, float elapsed) {
    constexpr Color locked{.2f,1.f,.2f,1.f};
    constexpr Color shadow{.06f,.08f,.08f,1.f};
    constexpr float corner=8.f;
    constexpr float labelWidth=34.f; // Six letters, six units per letter, two trailing units.
    constexpr std::array<std::string_view,6> glyphs{
        "0668", "02288660", "200668", "063238", "20063568", "0115577660"
    };
    if (width<labelWidth+12.f || height<88.f) return;

    const float progress=std::clamp(elapsed/.15f,0.f,1.f);
    const float gap=4.f+12.f*(1.f-progress);
    const float left=target.x-gap,right=target.x+target.width+gap;
    const float top=target.y-gap,bottom=target.y+target.height+gap;
    const bool onRight=right+labelWidth+24.f<width;
    const Vec2 anchor{std::clamp(onRight?right:left,4.f,width-4.f),
                      std::clamp(top,20.f,height-4.f)};
    const float labelX=std::clamp(onRight?anchor.x+12.f:anchor.x-12.f-labelWidth,6.f,width-labelWidth-6.f);
    const float labelY=std::clamp(anchor.y-15.f,12.f,height-76.f);
    const Vec2 elbow{std::lerp(anchor.x,onRight?labelX:labelX+labelWidth,progress),
                     std::lerp(anchor.y,labelY+3.f,progress)};
    const Vec2 end{std::lerp(anchor.x,onRight?labelX+labelWidth:labelX,progress),elbow.y};

    TextureMesh mesh;
    mesh.image=crossfireLockStrokeImage;
    mesh.nearest=true;
    mesh.beforeSpriteIndex=frame.sprites.size();
    constexpr auto segments=10+2*(2+4+3+3+4+5);
    mesh.vertices.reserve(segments*6-2);
    const auto stroke=[&](Vec2 from,Vec2 to,Color color) {
        const float dx=to.x-from.x,dy=to.y-from.y;
        const float length=std::hypot(dx,dy);
        if (length==0.f) return;
        const Vec2 normal{-dy/length,dx/length}; // One unit on either side of the reference line.
        const Vec2 uv{.5f,.5f};
        const std::array<MeshVertex,4> quad{{
            {{from.x-normal.x,from.y-normal.y},uv,color},
            {{from.x+normal.x,from.y+normal.y},uv,color},
            {{to.x-normal.x,to.y-normal.y},uv,color},
            {{to.x+normal.x,to.y+normal.y},uv,color}
        }};
        if (!mesh.vertices.empty()) {
            // Degenerate triangles join the separate line quads in one draw call.
            const MeshVertex last=mesh.vertices.back();
            mesh.vertices.push_back(last);
            mesh.vertices.push_back(quad.front());
        }
        mesh.vertices.insert(mesh.vertices.end(),quad.begin(),quad.end());
    };
    stroke({left,bottom-corner},{left,bottom},locked);
    stroke({left,bottom},{left+corner,bottom},locked);
    stroke({right-corner,bottom},{right,bottom},locked);
    stroke({right,bottom},{right,bottom-corner},locked);
    stroke({left,top+corner},{left,top},locked);
    stroke({left,top},{left+corner,top},locked);
    stroke({right-corner,top},{right,top},locked);
    stroke({right,top},{right,top+corner},locked);
    stroke(anchor,elbow,locked);
    stroke(elbow,end,locked);

    const auto text=[&](float offset,Color color) {
        float x=labelX+offset;
        for (const auto glyph:glyphs) {
            const auto point=[&](char index) {
                const int p=index-'0';
                return Vec2{x+float(p%3)*2.f,labelY+offset-6.f+float(p/3)*3.f};
            };
            for (std::size_t i=0;i<glyph.size();i+=2)
                stroke(point(glyph[i]),point(glyph[i+1]),color);
            x+=6.f;
        }
    };
    text(.7f,shadow);
    text(0.f,elapsed>=.15f && elapsed<.24f ? Color{} : locked);
    frame.meshes.push_back(std::move(mesh));
}

}
