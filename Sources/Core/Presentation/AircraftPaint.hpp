#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace md {
// Surface colour only. The original texture, alpha, detail placement and
// assembly stay intact. All parts of a family use the same channel curves;
// there are no separately coloured replacement panels or new markings.
inline constexpr std::array<unsigned,3> aircraftPaintMidpoint(std::string_view image) {
    if(image.starts_with("ALight")) return {124,130,117}; // muted grey olive
    if(image.starts_with("ATransport")) return {181,181,169}; // warm ivory
    if(image.starts_with("AHeavy")) return {132,141,168}; // neutral grey from warm source
    if(image.starts_with("ADuct")) return {112,124,166}; // Cheyenne cool steel grey
    return {128,128,128};
}
inline constexpr std::uint8_t aircraftPaintChannel(unsigned value,unsigned midpoint) {
    // Piecewise monotone curve preserves black and white; bright metal edges
    // remain distinct instead of clipping when the middle tones are raised.
    return static_cast<std::uint8_t>(value<=128
        ? (value*midpoint+64)/128
        : midpoint+((value-128)*(255-midpoint)+63)/127);
}
inline void applyAircraftPaint(std::string_view image,std::uint8_t* rgba,std::size_t pixels) {
    const auto mid=aircraftPaintMidpoint(image);
    if(mid==std::array<unsigned,3>{128,128,128})return;
    for(std::size_t i=0;i<pixels;++i) {
        auto* p=rgba+i*4;
        if(!p[3])continue;
        for(unsigned c=0;c<3;++c)p[c]=aircraftPaintChannel(p[c],mid[c]);
    }
}
}
