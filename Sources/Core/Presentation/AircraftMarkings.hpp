#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace md {
// Paint locations in the approved, unrotated source body. These small surface
// masks never alter alpha, sockets, texture dimensions, or original-unit art.
inline void applyAircraftMarking(std::string_view image,std::uint8_t* p,
                                 float u,float v) {
    if(p[3]<32)return;
    const unsigned light=(54u*p[0]+183u*p[1]+19u*p[2]+128u)/256u;
    if(light<42)return; // retain dark panel seams, vents and glazing
    int ink=0;
    const auto box=[&](float l,float t,float r,float b){return u>=l&&u<=r&&v>=t&&v<=b;};
    const auto hazard=[&]{return (int(std::floor((u+v)*105))&1)?1:2;};
    if(image=="ALightBody.png") {
        if(box(.27f,.536f,.39f,.555f)||box(.61f,.536f,.73f,.555f))ink=hazard();
        if(box(.20f,.444f,.28f,.455f)||box(.72f,.444f,.80f,.455f))ink=4;
    } else if(image=="ATransportBody.png") {
        if(box(.078f,.373f,.15f,.398f)||box(.82f,.373f,.89f,.398f))ink=hazard();
        // Small outlined triangles: no panel-wide replacement colour.
        const auto triangle=[&](float cx){const float y=(v-.389f)/.025f;
            const float edge=std::abs(u-cx)/.010f;
            return y>=0&&y<=1&&edge<=y&&(y>.76f||edge>y-.30f);};
        if(triangle(.34f)||triangle(.62f))ink=3;
    } else if(image=="AHeavyBody.png") {
        if(box(.305f,.26f,.355f,.295f)||box(.65f,.26f,.70f,.295f))ink=hazard();
        if(box(.38f,.053f,.43f,.072f)||box(.573f,.053f,.623f,.072f))ink=3;
    } else if(image=="ADuctBody.png") {
        if(box(.064f,.274f,.15f,.306f)||box(.855f,.274f,.94f,.306f))ink=hazard();
        if(box(.446f,.735f,.555f,.748f))ink=4;
    } else if(image=="ADuctGun.png"||image=="AHeavyCannon.png") {
        if(box(.14f,.16f,.86f,.22f))ink=3;
    }
    if(!ink)return;
    constexpr std::array<std::array<unsigned,3>,4> colours{{
        {187,139,62}, {52,57,49}, {143,57,42}, {207,203,181}}};
    const auto& c=colours[std::size_t(ink-1)];
    // Carry the source's local light and surface wear through the painted area.
    // No synthetic random scratches or large uniform flat fills.
    for(unsigned k=0;k<3;++k){
        const unsigned shaded=std::min(255u,c[k]*(light+112u)/240u);
        p[k]=std::uint8_t((p[k]+3u*shaded+2u)/4u);
    }
}
}
