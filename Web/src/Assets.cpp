// WAV validation follows this repository's Android Assets.cpp; no game rules are copied.
#include "Assets.hpp"
#include <fstream>
#include <stdexcept>
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"
namespace md::web {
Bytes Assets::read(std::string_view path){
 if(path.empty()||path.front()=='/'||path.find("..")!=path.npos)throw std::runtime_error("Invalid resource path");
 std::ifstream f("/assets/"+std::string(path),std::ios::binary);if(!f)throw std::runtime_error("Missing asset: "+std::string(path));
 return {std::istreambuf_iterator<char>(f),{}};
}
Vec2 Assets::imageSize(std::string_view name){
 auto key=std::string(name);if(sizes.contains(key))return sizes[key];
 std::ifstream probe("/assets/"+imagePath(name));if(!probe)return sizes[key]={};
 auto b=read(imagePath(name));int w,h,c;if(!stbi_info_from_memory(b.data(),b.size(),&w,&h,&c))throw std::runtime_error("Invalid image: "+key);
 return sizes[key]={float(w),float(h)};
}
audio::PCM decodeWav(std::span<const std::uint8_t> b){
    auto u16=[&](std::size_t n){return unsigned(b[n])|(unsigned(b[n+1])<<8);};
    auto u32=[&](std::size_t n){return u16(n)|(u16(n+2)<<16);};
    if(b.size()<12||std::memcmp(b.data(),"RIFF",4)||std::memcmp(b.data()+8,"WAVE",4)||u32(4)<4||std::size_t(u32(4))+8>b.size())throw std::runtime_error("Invalid WAV header");
    audio::PCM result;bool fmt=false,data=false;auto end=std::size_t(u32(4))+8;
    for(std::size_t p=12;p<end;){
        if(end-p<8)throw std::runtime_error("Truncated WAV chunk");
        auto n=std::size_t(u32(p+4));auto start=p+8;if(n>end-start)throw std::runtime_error("Invalid WAV chunk length");
        if(!std::memcmp(b.data()+p,"fmt ",4)){
            if(fmt||n<16||u16(start)!=1)throw std::runtime_error("Expected PCM WAV");fmt=true;
            result.channels=u16(start+2);result.sampleRate=u32(start+4);result.bitsPerSample=u16(start+14);
            const auto block=result.channels*result.bitsPerSample/8;
            if((result.channels!=1&&result.channels!=2)||(result.bitsPerSample!=8&&result.bitsPerSample!=16)||!result.sampleRate||result.sampleRate>192000||u16(start+12)!=block||u32(start+8)!=block*result.sampleRate)throw std::runtime_error("Invalid PCM format");
        }else if(!std::memcmp(b.data()+p,"data",4)){
            if(data||!n||n>64*1024*1024)throw std::runtime_error("Invalid PCM payload");data=true;result.bytes.assign(b.begin()+start,b.begin()+start+n);
        }
        p=start+n+(n&1);if(p>end)throw std::runtime_error("Missing WAV pad byte");
    }
    if(!fmt||!data||result.bytes.size()%(result.channels*result.bitsPerSample/8))throw std::runtime_error("Incomplete PCM WAV");return result;
}
}
