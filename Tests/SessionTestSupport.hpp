#pragma once
#include "GameSession.hpp"
#include "World.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace md;
struct Resources:ResourceAccess {
    std::filesystem::path root;std::map<std::string,Vec2> sizes;
    Bytes read(std::string_view p)override{std::ifstream f(root/p,std::ios::binary);if(!f)throw std::runtime_error(std::string(p));return {std::istreambuf_iterator<char>(f),{}};}
    Vec2 imageSize(std::string_view n)override{
        if(auto i=sizes.find(std::string(n));i!=sizes.end())return i->second;
        auto path=n.starts_with("Generated/")?std::string(n):"Images/"+std::string(n);
        if(!std::filesystem::exists(root/path))return {};
        auto b=read(path);auto u16=[&](std::size_t i){return unsigned(b.at(i))*256+b.at(i+1);};
        if(b.size()>24&&b[0]==137){auto u32=[&](int i){return unsigned(b[i])<<24|unsigned(b[i+1])<<16|unsigned(b[i+2])<<8|b[i+3];};return sizes[std::string(n)]={float(u32(16)),float(u32(20))};}
        for(std::size_t i=2;i+4<b.size();){if(b[i++]!=255)continue;unsigned marker=b[i++];if(marker==0xd9||marker==0xda)break;auto len=u16(i);if(marker==0xc0||marker==0xc1||marker==0xc2)return sizes[std::string(n)]={float(u16(i+5)),float(u16(i+3))};if(len<2)break;i+=len;}
        throw std::runtime_error("Image size: "+std::string(n));
    }
};
struct Host:HostServices {
    Bytes saved;int writes=0,sounds=0;std::string song;
    std::optional<Bytes> readSave()override{if(saved.empty())return {};return saved;}
    bool writeSave(std::span<const std::uint8_t> b)override{saved.assign(b.begin(),b.end());++writes;return true;}
    void playSound(std::string_view,float,float)override{++sounds;}
    void playMusic(std::string_view n,float)override{song=n;}
    void stopMusic()override{song.clear();}
    void requestName(std::string,std::size_t max,std::function<void(std::optional<std::string>)> cb)override{assert(max==10);cb("PLAYER");}
};
void ticks(GameSession& s,int n=160){for(int i=0;i<n;++i)s.tick();}
void tap(GameSession& s,float x,float y){s.touch({1,TouchPhase::began,{x,y}});s.touch({1,TouchPhase::ended,{x,y}});}
bool has(const Frame& f,std::string_view name){for(const auto& s:f.sprites)if(s.image==name)return true;return false;}
void checkImages(const Frame& f,Resources& r){for(const auto& s:f.sprites){assert(std::isfinite(s.destination.x)&&std::isfinite(s.destination.y));if(s.image.empty())continue;auto size=r.imageSize(s.image);if(!(size.x>0&&size.y>0))throw std::runtime_error("Rendered missing image: "+s.image);if(s.sourcePx){assert(s.sourcePx->x>=0&&s.sourcePx->y>=0);assert(s.sourcePx->x+s.sourcePx->width<=size.x+.01f&&s.sourcePx->y+s.sourcePx->height<=size.y+.01f);}}}
