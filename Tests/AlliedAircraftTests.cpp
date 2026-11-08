#include "PNGTestSize.hpp"
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
    md::Vec2 imageSize(std::string_view name)override{auto it=sizes.find(std::string(name));if(it!=sizes.end())return it->second;if(!std::filesystem::exists(root/"Images"/name))return {};auto b=read("Images/"+std::string(name));if(b.size()>24&&b[0]==137){return sizes[std::string(name)]=mdtest::pngSize(b);}return {};}
};
struct Host:md::HostServices{int writes=0,sounds=0;std::optional<md::Bytes>readSave()override{return{};}bool writeSave(std::span<const std::uint8_t>)override{++writes;return true;}void playSound(std::string_view,float,float)override{++sounds;}void playMusic(std::string_view,float)override{}void stopMusic()override{}void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}};


#include "AircraftComparisonScene.hpp"
#include "Core/Validation/AlliedAircraft.hpp"
#include "Core/Validation/AircraftComparison.hpp"
#include <chrono>
#include <set>
int main(int argc,char** argv){assert(argc==2);Resources r;r.root=argv[1];
 auto table=[&](const char* n){auto b=r.read(std::string("Tables/")+n+".tsv");return md::DataTable::parse({reinterpret_cast<const char*>(b.data()),b.size()});};
 auto builds=md::BuildCatalog::load(table("MachBuild_Parts"),table("MachBuild_Name"));auto items=table("ItemInfo");
 std::cout<<"model,level,cells,hp,upgradeCredits,buildSeconds\n";
 for(unsigned k=0;k<4;++k){int hp=0,cells=0;for(unsigned level=1;level<=10;++level){auto e=md::validation::aircraftTrialEconomy(k,level,builds,items,md::BattleGeometry::pad());assert(e.hp>hp&&e.cells>cells);hp=e.hp;cells=e.cells;std::cout<<md::validation::alliedAircraft[k].model<<","<<level<<","<<e.cells<<","<<e.hp<<","<<e.upgradeCredits<<","<<e.duration<<"\n";}}
 // All clock directions remain in the map; the moving target updates the orbit center.
 md::AircraftTactics route;std::set<int> directions;
 for(int i=0;i<24;++i){auto p=route.destination(i*10,{0,0},{0,200},300,512,800,12);directions.insert(route.sector);assert(std::abs(p.x)<=244&&std::abs(p.y)<=788);}
 assert(directions.size()==12);route.destination(250,{0,0},{0,400},300,512,800,12);assert(route.anchor.y==400);

 // Right-to-left and left-to-right both decelerate to an exact stopped frame.
 for(float sign:{-1.f,1.f}){
  md::AircraftTactics t;md::AircraftMotion m;m.moveSpeed=2;m.rotationDivisor=12;m.setDestination({sign*10000,0});
  float last=0;for(int tick=0;tick<60;++tick){t.advance(m);assert(m.speed>=last-.00001f&&m.speed<=2.00001f);last=m.speed;}
  assert(std::abs(m.direction.x-sign*2)<.0001f);m.setDestination({-sign*10000,0});bool stopped=false,opposite=false;float previous=m.speed;
  for(int tick=0;tick<100;++tick){auto old=m.position;t.advance(m);if(!stopped){assert(m.speed<=previous+.00001f);assert(m.direction.x*sign>=0);if(m.speed==0){stopped=true;assert(m.position.x==old.x&&m.position.y==old.y);}}else{assert(m.direction.x*sign<=0);opposite|=m.direction.x*sign<-.1f;}previous=m.speed;}
  assert(stopped&&opposite);
 }
 for(unsigned mode=3;mode<6;++mode){
  md::validation::AircraftComparisonScene scene(r,mode);auto& w=*scene.world;assert(w.actors().size()==8);
  auto before=scene.save.encode();const float scale=mode==3?1.f:.5f;std::vector<double> ms;std::array<float,4> minX{},maxX{};
  for(unsigned k=0;k<4;++k){auto& a=*w.actors()[k*2+1];assert(a.player&&a.type==2&&a.flightTactics);assert(a.hp==md::validation::aircraftTrialEconomy(k,1,builds,items,md::BattleGeometry::pad()).hp);assert(a.weapons.size()==(k==0?2:k==1?5:k==2?2:4));minX[k]=maxX[k]=a.position.x;}
  bool shots=false;
  for(int tick=0;tick<3600;++tick){auto start=std::chrono::steady_clock::now();w.tick();auto f=scene.frame();ms.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());shots|=!w.projectiles().empty();
   for(unsigned k=0;k<4;++k){auto& a=*w.actors()[k*2+1];assert(a.hp>0&&std::isfinite(a.position.x)&&std::isfinite(a.position.y));assert(std::abs(a.position.x)<256*scale);minX[k]=std::min(minX[k],a.position.x);maxX[k]=std::max(maxX[k],a.position.x);assert(!a.collision);}
   for(const auto& s:f.sprites)assert(std::isfinite(s.destination.x)&&std::isfinite(s.destination.y));
  }
  for(unsigned k=0;k<4;++k){assert(maxX[k]-minX[k]>20*scale);assert(w.actors()[k*2+1]->target!=0);}
  assert(shots&&scene.host.sounds>0&&scene.host.writes==0);assert(scene.save.encode()==before);
  std::sort(ms.begin(),ms.end());std::cout<<"mode="<<mode<<" tick+Frame CPU p99_ms="<<ms[ms.size()*99/100]<<" max_ms="<<ms.back()<<"\n";
  auto time=w.time();auto position=w.actors()[1]->position;w.setPaused(true);for(int i=0;i<60;++i)w.tick();assert(w.time()==time&&w.actors()[1]->position.x==position.x);w.setPaused(false);
  // Same Crossfire qualification and original projectiles; every new ally participates.
  const std::vector<md::OwnedAttack> attacks{{"Crossfire-Missile",0,2,true}};scene.save.setAttacks(0,attacks);
  for(unsigned k=0;k<4;++k)w.actors()[k*2+1]->position={float(k*15),-150};
  assert(w.activateSpecial("Crossfire-Missile",{0,0}));std::set<md::ActorId> owners;
  for(const auto& p:w.projectiles())if(p.motion.lockOn)owners.insert(p.attacker);
  assert(owners.size()==4&&scene.save.attacks(0)[0].count==1);
  // Existing enemies acquire/fight an airborne player without an air-only damage path.
  auto enemy=w.validationSpawn("E_Tank_01",false,{0,0});auto* e=w.actor(enemy);auto* a=w.actors()[1].get();a->position={0,50*scale};a->flightTactics.reset();a->aircraft.state=0;a->weapons.clear();e->target=a->id;e->moveToTarget=false;e->enemyAI.stopDelay=1e9f;
  int hp=a->hp;for(int i=0;i<600&&a->hp==hp;++i)w.tick();assert(a->hp<hp);
  // Campaign reset removes the trial units and restores normal spawns.
  w.start(0);for(const auto& a:w.actors())assert(!a->flightTactics);assert(scene.host.writes==0);
 }
 std::cout<<"AlliedAircraft: 4 trial allies; 40 economy proposals; twelve clock routes; auto targeting/fire; Crossfire; enemy damage; pause; no file writes passed\n";
}
