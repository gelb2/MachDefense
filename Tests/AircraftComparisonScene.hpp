#pragma once
#include "Core/Simulation/World.hpp"
#include <memory>
#include <sstream>
namespace md::validation {
struct AircraftHost:HostServices{
 unsigned writes=0;
 std::optional<Bytes> readSave()override{return {};}
 bool writeSave(std::span<const std::uint8_t>)override{++writes;return false;}
 void playSound(std::string_view,float,float)override{}
 void playMusic(std::string_view,float)override{}
 void stopMusic()override{}
 void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}
};
struct AircraftComparisonScene {
 AircraftHost host;SaveData save=SaveData::empty(1);std::unique_ptr<World> world;float w=640,h=640;
 AircraftComparisonScene(ResourceAccess& r,unsigned mode){save.createProfile("ART REVIEW",1);auto g=mode==0?BattleGeometry::pad():BattleGeometry::phone();if(mode==1){w=320;h=480;}else if(mode==2){w=768;h=1024;}world=std::make_unique<World>(r,host,save,0,g,123);world->start(0);world->setViewport(w,h);world->validationAircraftComparison();}
 Frame frame(){Frame f;f.logicalWidth=w;f.logicalHeight=h;world->appendSprites(f);return f;}
 void move(){for(const auto& a:world->actors())if(a->flightHeight>0)a->aircraft.setDestination({a->position.x+30,a->position.y+30});}
 std::string state(){std::ostringstream o;o<<"{\"time\":"<<world->time()<<",\"writes\":"<<host.writes<<",\"actors\":[";bool first=true;for(const auto& a:world->actors()){if(!first)o<<",";first=false;o<<"{\"name\":\""<<a->name<<"\",\"x\":"<<a->position.x<<",\"y\":"<<a->position.y<<",\"rotor\":"<<a->radarAngle<<",\"height\":"<<a->flightHeight<<"}";}o<<"]}";return o.str();}
};
}
