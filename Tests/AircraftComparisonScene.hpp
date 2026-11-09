#pragma once
#include "Core/Simulation/World.hpp"
#include <memory>
#include <sstream>
namespace md::validation {
struct AircraftHost:HostServices{
 std::function<void(std::string_view,float,float)> audio;
 unsigned writes=0,sounds=0;std::vector<std::string> soundNames;
 std::optional<Bytes> readSave()override{return {};}
 bool writeSave(std::span<const std::uint8_t>)override{++writes;return false;}
 void playSound(std::string_view name,float gain,float pan)override{++sounds;soundNames.emplace_back(name);if(audio)audio(name,gain,pan);}
 void playMusic(std::string_view,float)override{}
 void stopMusic()override{}
 void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}
};
struct AircraftComparisonScene {
 AircraftHost host;SaveData save=SaveData::empty(1);std::unique_ptr<World> world;float w=640,h=640;bool allied=false;
 AircraftComparisonScene(ResourceAccess& r,unsigned mode){save.createProfile("ART REVIEW",1);allied=mode>=3;mode%=3;auto g=mode==0?BattleGeometry::pad():BattleGeometry::phone();if(mode==1){w=320;h=480;}else if(mode==2){w=768;h=1024;}world=std::make_unique<World>(r,host,save,0,g,123);world->start(0);world->setViewport(w,h);if(allied)world->validationAlliedAircraftComparison();else world->validationAircraftComparison();}
 Frame frame(){world->prepareDraw();Frame f;f.logicalWidth=w;f.logicalHeight=h;world->appendSprites(f);return f;}
 void move(){for(const auto& a:world->actors())if(a->flightHeight>0)a->aircraft.setDestination({a->position.x+30,a->position.y+30});}
 void production(unsigned level,unsigned count,bool firing){
  for(auto& a:world->actors())a->removed=true;
  static const char* names[]{"AircraftLight","AircraftHeavy","AircraftDuct","AircraftTransport"};
  char suffix[8];std::snprintf(suffix,8,"_%02u",std::clamp(level,1u,10u));
  for(unsigned i=0;i<count;++i){
   float x=count==1?0.f:count==4?-165.f+110.f*i:-220.f+48.f*(i%10);float y=count==1?-90.f:count==4?-90.f:-220.f+45.f*(i/10);
   auto id=world->validationSpawn(std::string(names[i%4])+suffix,true,{x,y});auto* a=world->actor(id);
   if(!firing){a->weapons.clear();a->flightTactics->speed=0;a->aircraft.moveSpeed=0;}
   else{auto target=world->validationSpawn("E_Tank_32",false,{x,110});auto* t=world->actor(target);t->weapons.clear();t->enemyAI.stopDelay=t->attackCheck=1e9f;t->hp=t->maxHP=10000000;a->target=target;}
  }
 }
 void fight(){
  // Explicit targets isolate muzzle/weapon behavior from campaign spawning.
  for(unsigned i=0;i<world->actors().size();i+=2){auto* target=world->actor(world->actors()[i]->id);auto* heli=world->actor(world->actors()[i+1]->id);target->hp=target->maxHP=100000;heli->target=target->id;heli->moveToTarget=false;}
 }
 std::string state(){std::ostringstream o;o<<"{\"time\":"<<world->time()<<",\"writes\":"<<host.writes<<",\"sounds\":"<<host.sounds<<",\"projectiles\":"<<world->projectiles().size()<<",\"countermeasureBursts\":"<<world->countermeasureBursts()<<",\"countermeasureParticles\":"<<world->countermeasureParticles()<<",\"actors\":[";bool first=true;for(const auto& a:world->actors()){if(a->removed)continue;if(!first)o<<",";first=false;o<<"{\"name\":\""<<a->name<<"\",\"x\":"<<a->position.x<<",\"y\":"<<a->position.y<<",\"rotor\":"<<a->radarAngle<<",\"height\":"<<a->flightHeight<<",\"hp\":"<<a->hp<<",\"vx\":"<<a->aircraft.direction.x<<",\"vy\":"<<a->aircraft.direction.y<<",\"speed\":"<<a->aircraft.speed<<",\"braking\":"<<(a->flightTactics&&a->flightTactics->braking?"true":"false")<<",\"bank\":"<<(a->flightTactics?a->flightTactics->bank:0)<<",\"dodging\":"<<(a->flightTactics&&a->flightTactics->dodging?"true":"false")<<",\"dodges\":"<<(a->flightTactics?a->flightTactics->dodgeCount:0)<<",\"ready\":"<<(a->flightTactics?a->flightTactics->dodgeReady:0)<<",\"heading\":"<<a->angle<<",\"weapons\":"<<a->weapons.size()<<"}";}o<<"]}";return o.str();}
};
}
