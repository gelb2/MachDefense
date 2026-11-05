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
int main(int argc,char**argv){assert(argc==2);Resources resources;resources.root=argv[1];Host host;auto save=md::SaveData::empty(1);save.createProfile("PLAYER",1);
    for(auto geometry:{md::BattleGeometry::phone(),md::BattleGeometry::pad()}){
        md::World world(resources,host,save,0,geometry,123);
        for(unsigned stage=0;stage<80;++stage){world.start(stage);assert(world.state()==md::WorldState::running);assert(save.readWord(md::SaveData::slotStart+0x34)==stage);int bases=0;for(const auto& a:world.actors())if(a->base){++bases;if(a->player){assert(a->hp==10000);assert(a->position.y==84*geometry.deviceScale-geometry.halfLength());}else{assert(a->hp==40000+int(stage)*10000);assert(a->position.y==geometry.halfLength()-50*geometry.deviceScale);}}assert(bases==2);md::Frame frame;world.appendSprites(frame);assert(frame.sprites.size()>2);for(const auto& s:frame.sprites){assert(std::isfinite(s.destination.x));if(s.sourcePx){auto size=resources.imageSize(s.image);assert(s.sourcePx->x+s.sourcePx->width<=size.x);assert(s.sourcePx->y+s.sourcePx->height<=size.y);}}}
        world.start(0);assert(world.cells()==200);world.setPaused(true);double before=world.time();for(int i=0;i<60;++i)world.tick();assert(world.time()==before&&world.cells()==200);world.setPaused(false);world.tick();assert(world.time()>before);
        auto owned=save.builds(0);assert(!owned.empty());assert(world.enqueueBuild(owned[0].name));assert(world.cells()<201);const auto startCount=world.actors().size();for(int i=0;i<900;++i)world.tick();assert(world.productionCount()==0);assert(world.actors().size()>startCount);
        md::Vec2 p{12,-100};auto screen=world.worldToScreen(p);auto round=world.screenToWorld(screen);assert(std::abs(round.x-p.x)<.001f&&std::abs(round.y-p.y)<.001f);
        // Below the playable world is the original command HUD, not a camera tap.
        assert(!world.touch({1,md::TouchPhase::began,{160,470}}));assert(!world.touch({1,md::TouchPhase::ended,{160,470}})||geometry.deviceScale==1);
        world.touch({2,md::TouchPhase::began,{160,100}});const float cam=world.cameraTargetY();
        world.touch({2,md::TouchPhase::moved,{160,110}});world.touch({2,md::TouchPhase::moved,{160,120}});world.touch({2,md::TouchPhase::moved,{160,130}});
        assert(world.cameraTargetY()>cam);assert(!world.touch({2,md::TouchPhase::ended,{160,130}}));
        md::ActorId enemy=0,player=0;for(const auto&a:world.actors())if(a->base)(a->player?player:enemy)=a->id;
        // Bring the original enemy base into range to exercise the real weapon
        // controller/direct-damage/audio path without waiting for a full stage.
        auto* friendly=world.actor(player);auto* hostile=world.actor(enemy);hostile->position={friendly->position.x,friendly->position.y+60};
        const int initialHP=hostile->hp;for(int i=0;i<600;++i)world.tick();assert(hostile->hp<initialHP);assert(host.sounds>0);
        world.damage(enemy,100000000);assert(world.state()==md::WorldState::cleared);bool clear=false;for(const auto&e:world.takeEvents())clear|=e.kind==md::WorldEvent::Kind::stageCleared;assert(clear);
        world.start(1);for(const auto&a:world.actors())if(a->base&&a->player)player=a->id;world.damage(player,100000000);assert(world.state()==md::WorldState::failed);
    }
    // Enemy collision override must separate earlier same-side bodies. Freeze
    // their AI/motion to measure only the collision, away from map obstacles.
    {md::World world(resources,host,save,0,md::BattleGeometry::phone(),123);world.start(0);
        auto first=world.validationSpawn("E_Tank_01",false,{0,0});
        auto second=world.validationSpawn("E_Tank_01",false,{10,0});
        auto* a=world.actor(first);auto* b=world.actor(second);
        a->radius=b->radius=10;a->enemyAI.stopDelay=b->enemyAI.stopDelay=10000;
        a->vehicle.state=b->vehicle.state=0;a->attackCheck=b->attackCheck=10000;
        world.tick();world.tick();
        assert(std::abs(a->position.x+1.5f)<.0001f&&std::abs(b->position.x-11.5f)<.0001f);
    }
    assert(host.writes==0);std::cout<<"WorldTests: 80 phone + 80 pad maps, bases, production, pause, coordinates, clear/fail, no implicit save passed\n";
}
