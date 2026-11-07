#pragma once
#include "Core/Simulation/World.hpp"
#include <memory>
#include <sstream>
namespace md::validation {
struct AircraftHost:HostServices{
 unsigned writes=0,sounds=0;std::vector<std::string> soundNames;
 std::optional<Bytes> readSave()override{return {};}
 bool writeSave(std::span<const std::uint8_t>)override{++writes;return false;}
 void playSound(std::string_view name,float,float)override{++sounds;soundNames.emplace_back(name);}
 void playMusic(std::string_view,float)override{}
 void stopMusic()override{}
 void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}
};
struct AircraftComparisonScene {
 AircraftHost host;SaveData save=SaveData::empty(1);std::unique_ptr<World> world;float w=640,h=640;
 AircraftComparisonScene(ResourceAccess& r,unsigned mode){save.createProfile("ART REVIEW",1);auto g=mode==0?BattleGeometry::pad():BattleGeometry::phone();if(mode==1){w=320;h=480;}else if(mode==2){w=768;h=1024;}world=std::make_unique<World>(r,host,save,0,g,123);world->start(0);world->setViewport(w,h);world->validationAircraftComparison();}
 Frame frame(){Frame f;f.logicalWidth=w;f.logicalHeight=h;world->appendSprites(f);return f;}
 void move(){for(const auto& a:world->actors())if(a->flightHeight>0)a->aircraft.setDestination({a->position.x+30,a->position.y+30});}
 void fight(){
  // Explicit targets isolate muzzle/weapon behavior from campaign spawning.
  for(unsigned i=0;i<4;i+=2){auto* target=world->actor(world->actors()[i]->id);auto* heli=world->actor(world->actors()[i+1]->id);target->hp=target->maxHP=100000;heli->target=target->id;heli->moveToTarget=false;}
 }
 std::string state(){std::ostringstream o;o<<"{\"time\":"<<world->time()<<",\"writes\":"<<host.writes<<",\"sounds\":"<<host.sounds<<",\"projectiles\":"<<world->projectiles().size()<<",\"actors\":[";bool first=true;for(const auto& a:world->actors()){if(!first)o<<",";first=false;o<<"{\"name\":\""<<a->name<<"\",\"x\":"<<a->position.x<<",\"y\":"<<a->position.y<<",\"rotor\":"<<a->radarAngle<<",\"height\":"<<a->flightHeight<<",\"hp\":"<<a->hp<<",\"weapons\":"<<a->weapons.size()<<"}";}o<<"]}";return o.str();}
};
}
