#include "Assets.hpp"
#include <stdexcept>
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"
namespace md::android {
Bytes Assets::read(std::string_view name){
    if(name.empty()||name.front()=='/'||name.find("..")!=name.npos)throw std::runtime_error("Invalid resource path");
    auto* asset=AAssetManager_open(manager_,std::string(name).c_str(),AASSET_MODE_BUFFER);
    if(!asset)throw std::runtime_error("Missing resource: "+std::string(name));
    struct Close{AAsset* p;~Close(){AAsset_close(p);}} close{asset};
    auto length=AAsset_getLength64(asset);if(length<0||length>128*1024*1024)throw std::runtime_error("Invalid resource length");
    Bytes bytes(static_cast<std::size_t>(length));std::size_t pos=0;
    while(pos<bytes.size()){int n=AAsset_read(asset,bytes.data()+pos,bytes.size()-pos);if(n<=0)throw std::runtime_error("Truncated resource");pos+=n;}
    return bytes;
}
std::string Assets::imagePath(std::string_view name){return name.starts_with("Generated/")?std::string(name):"Images/"+std::string(name);}
Vec2 Assets::imageSize(std::string_view name){
    auto key=std::string(name);if(auto it=sizes_.find(key);it!=sizes_.end())return it->second;
    // The original dummy attachment has no image. Match Apple imageSize: zero size.
    auto path=imagePath(name);auto* probe=AAssetManager_open(manager_,path.c_str(),AASSET_MODE_UNKNOWN);
    if(!probe)return sizes_.emplace(key,Vec2{}).first->second;AAsset_close(probe);
    auto bytes=read(path);int w=0,h=0,c=0;
    if(!stbi_info_from_memory(bytes.data(),int(bytes.size()),&w,&h,&c))throw std::runtime_error("Cannot inspect image: "+key);
    return sizes_.emplace(key,Vec2{float(w),float(h)}).first->second;
}
std::vector<std::string> Assets::list(std::string_view folder){
    auto* dir=AAssetManager_openDir(manager_,std::string(folder).c_str());if(!dir)throw std::runtime_error("Missing resource directory");
    std::vector<std::string> out;while(auto* name=AAssetDir_getNextFileName(dir))out.emplace_back(name);AAssetDir_close(dir);return out;
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
