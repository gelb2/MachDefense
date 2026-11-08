#pragma once
#include "Core/Contracts/RenderTypes.hpp"
#include <algorithm>
#include <array>
#include <span>
#include <stdexcept>
namespace mdtest {
inline md::Vec2 pngSize(std::span<const std::uint8_t> bytes) {
    constexpr std::array<std::uint8_t,8> signature{137,80,78,71,13,10,26,10};
    if(bytes.size()<signature.size() || !std::equal(signature.begin(),signature.end(),bytes.begin()))
        throw std::runtime_error("Invalid PNG signature");
    auto word=[&](std::size_t offset) {
        if(offset>bytes.size() || bytes.size()-offset<4) throw std::runtime_error("Truncated PNG");
        return std::uint32_t(bytes[offset])<<24 | std::uint32_t(bytes[offset+1])<<16 |
               std::uint32_t(bytes[offset+2])<<8 | bytes[offset+3];
    };
    for(std::size_t offset=8;offset+12<=bytes.size();) {
        const auto length=word(offset);
        if(length>bytes.size()-offset-12) throw std::runtime_error("Truncated PNG chunk");
        if(word(offset+4)==0x49484452u) {
            if(length!=13) throw std::runtime_error("Invalid IHDR");
            return {float(word(offset+8)),float(word(offset+12))};
        }
        offset+=std::size_t(length)+12;
    }
    throw std::runtime_error("PNG has no IHDR");
}
inline md::Vec2 rasterSize(std::span<const std::uint8_t> bytes) {
    if(!bytes.empty() && bytes[0]==137) return pngSize(bytes);
    if(bytes.size()<4 || bytes[0]!=255 || bytes[1]!=216) throw std::runtime_error("Unknown image format");
    auto word=[&](std::size_t offset) {
        if(offset+2>bytes.size()) throw std::runtime_error("Truncated JPEG");
        return unsigned(bytes[offset])*256+bytes[offset+1];
    };
    for(std::size_t offset=2;offset+4<=bytes.size();) {
        if(bytes[offset++]!=255) continue;
        const auto marker=bytes[offset++];
        if(marker==0xd9 || marker==0xda) break;
        const auto length=word(offset);
        if(length<2 || length>bytes.size()-offset) throw std::runtime_error("Invalid JPEG segment");
        if(marker==0xc0 || marker==0xc1 || marker==0xc2) {
            if(length<8) throw std::runtime_error("Invalid JPEG frame");
            return {float(word(offset+5)),float(word(offset+3))};
        }
        offset+=length;
    }
    throw std::runtime_error("JPEG has no frame header");
}
}
