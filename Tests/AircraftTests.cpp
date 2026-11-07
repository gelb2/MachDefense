#include "Core/Simulation/World.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
struct Resources:md::ResourceAccess{
    std::filesystem::path root;std::unordered_map<std::string,md::Vec2> sizes;
    md::Bytes read(std::string_view path)override{std::ifstream f(root/path,std::ios::binary);if(!f)throw std::runtime_error(std::string(path));return {std::istreambuf_iterator<char>(f),{}};}
    md::Vec2 imageSize(std::string_view name)override{auto it=sizes.find(std::string(name));if(it!=sizes.end())return it->second;if(!std::filesystem::exists(root/"Images"/name))return {};auto b=read("Images/"+std::string(name));if(b.size()>24&&b[0]==137){auto be=[&](int i){return unsigned(b[i])<<24|unsigned(b[i+1])<<16|unsigned(b[i+2])<<8|b[i+3];};return sizes[std::string(name)]={float(be(16)),float(be(20))};}return {};}
};
struct Host:md::HostServices{int writes=0,sounds=0;std::optional<md::Bytes>readSave()override{return{};}bool writeSave(std::span<const std::uint8_t>)override{++writes;return true;}void playSound(std::string_view,float,float)override{++sounds;}void playMusic(std::string_view,float)override{}void stopMusic()override{}void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}};

#include "AircraftComparisonScene.hpp"
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];
 for(unsigned mode=0;mode<3;++mode){md::validation::AircraftComparisonScene scene(r,mode);auto& w=*scene.world;assert(w.actors().size()==4);auto before=scene.frame();unsigned heliSprites=0,shadows=0;for(const auto& s:before.sprites)if(s.image.starts_with("ALight")||s.image.starts_with("AHeavy")){assert(s.nearest);++heliSprites;if(s.color.r==0){++shadows;assert(s.color.a==.55f);}}assert(heliSprites==10&&shadows==5);
 for(const char* name:{"ALightBody.png","ALightRotor.png","AHeavyBody.png","AHeavyRotor.png"}){
  const md::Sprite *shadow=nullptr,*body=nullptr;for(const auto& s:before.sprites)if(s.image==name){if(s.color.r==0)shadow=&s;else body=&s;}
  assert(shadow&&body);const float z=mode==0?1.f:(mode==1?.5f:1.375f);const float gap=(std::string_view(name).starts_with("ALight")?16.f:22.f)*z;
  assert(std::abs(shadow->destination.x-body->destination.x-gap)<.001f&&std::abs(shadow->destination.y-body->destination.y-gap)<.001f);assert(shadow->rotationDegrees==body->rotationDegrees);
 }
 auto* light=w.actor(w.actors()[1]->id);float angle=light->radarAngle;w.tick();assert(std::abs(light->radarAngle-angle-7.3f*6.28318530718f/60)<.00001f);
 w.setPaused(true);angle=light->radarAngle;auto time=w.time();for(int i=0;i<60;++i)w.tick();assert(light->radarAngle==angle&&w.time()==time);w.setPaused(false);
 auto pos=light->position;scene.move();for(int i=0;i<240;++i)w.tick();assert(light->position.x!=pos.x||light->position.y!=pos.y);assert(scene.host.writes==0);assert(w.actors().size()==4);
 auto frame=scene.frame();for(const auto& s:frame.sprites){assert(std::isfinite(s.destination.x)&&std::isfinite(s.destination.y));}
 // Starting a real stage restores its normal original spawn path.
 w.start(0);for(int i=0;i<900;++i)w.tick();assert(w.actors().size()>2);
 }
 std::cout<<"AircraftTests: 3 viewports; 4 real World actors; original part assembly, independent fast rotors, pause, movement, ground shadows, no save writes, campaign restart passed\n";
}
