#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include "Core/Presentation/AircraftMarkings.hpp"

namespace md {
// Surface colour only. The original texture, alpha, detail placement and
// assembly stay intact. All parts of a family use the same channel curves;
// there are no separately coloured replacement panels.
inline constexpr std::array<unsigned,3> aircraftPaintMidpoint(std::string_view image) {
    if(image.starts_with("ALight")) return {124,130,117}; // muted grey olive
    if(image.starts_with("ATransport")) return {181,181,169}; // warm ivory
    if(image.starts_with("AHeavy")) return {135,137,140}; // neutral steel grey
    if(image.starts_with("ADuct")) return {122,131,143}; // Cheyenne cool steel grey
    return {128,128,128};
}
inline constexpr std::uint8_t aircraftPaintChannel(unsigned value,unsigned midpoint) {
    // Piecewise monotone curve preserves black and white; bright metal edges
    // remain distinct instead of clipping when the middle tones are raised.
    return static_cast<std::uint8_t>(value<=128
        ? (value*midpoint+64)/128
        : midpoint+((value-128)*(255-midpoint)+63)/127);
}
inline void prepareAircraftPaint(std::string_view image,std::uint8_t* rgba,std::size_t pixels,
                               unsigned width=0,unsigned height=0) {
    const auto mid=aircraftPaintMidpoint(image);
    if(mid==std::array<unsigned,3>{128,128,128})return;
    for(std::size_t i=0;i<pixels;++i) {
        auto* p=rgba+i*4;
        if(!p[3])continue;
        const bool steel=image.starts_with("AHeavy")||image.starts_with("ADuct");
        const unsigned light=(54u*p[0]+183u*p[1]+19u*p[2]+128u)/256u;
        for(unsigned c=0;c<3;++c)
            p[c]=aircraftPaintChannel(steel?(p[c]+3u*light+2u)/4u:p[c],mid[c]);
        if(width&&height&&pixels==std::size_t(width)*height) {
            const float x=(float(i%width)+.5f)/width,y=(float(i/width)+.5f)/height;
            // The heavy source faces down and was packed -90 degrees. Other
            // bodies face up and were packed +90 degrees. Rotor art is unmarked.
            const bool heavy=image.starts_with("AHeavy");
            applyAircraftMarking(image,p,heavy?1.f-y:y,heavy?x:1.f-x);
        }
    }
}
// Approved colours/markings are now baked into helicopter-only PNGs by
// tools/bake_aircraft_paint.py. Retain the platform hook without repainting.
inline void applyAircraftPaint(std::string_view,std::uint8_t*,std::size_t,
                               unsigned=0,unsigned=0) {}
}
