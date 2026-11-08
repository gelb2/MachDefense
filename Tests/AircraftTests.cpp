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
#include "Core/Validation/AircraftComparison.hpp"
#include <algorithm>
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];
 auto data=r.read("Tables/WeaponInfo.tsv");auto table=md::DataTable::parse({reinterpret_cast<const char*>(data.data()),data.size()});
 for(bool phone:{false,true}){
  auto catalog=md::WeaponCatalog::load(table,phone);const auto count=catalog.size();md::validation::appendAircraftWeapons(catalog);assert(catalog.size()==count+std::size(md::validation::aircraftWeapons));
  for(const auto& binding:md::validation::aircraftWeapons){auto a=*catalog.find(binding.part),b=*catalog.find(binding.original);
   assert(a.type==b.type&&a.damage==b.damage&&a.damageRange==b.damageRange&&a.shotRange==b.shotRange&&a.shotCount==b.shotCount&&a.shotDelay==b.shotDelay&&a.reloadDelay==b.reloadDelay&&a.speed==b.speed&&a.aimRate==b.aimRate&&a.special1==b.special1&&a.special2==b.special2&&a.shell==b.shell&&a.uiTile==b.uiTile&&a.bulletImage==b.bulletImage&&a.bulletTile==b.bulletTile&&a.effectImage==b.effectImage&&a.effectType==b.effectType&&a.effectColumns==b.effectColumns&&a.effectRows==b.effectRows&&a.explodeImage==b.explodeImage&&a.explodeScale==b.explodeScale&&a.fireSound==b.fireSound);
  }
  bool rejected=false;try{catalog.addAlias("E_Tank31_Wpn","E_Turret_Wpn_02");}catch(const std::runtime_error&){rejected=true;}assert(rejected&&catalog.find("E_Tank31_Wpn")->damage==120);
  // First medium missile burst is exactly two, with the original one-second spacing.
  auto missile=*catalog.find("AHeavyMissile");md::WeaponController controller;md::DarwinRandom random(12);md::WeaponFrame frame;frame.manualFire=true;frame.muzzlePosition={17,23};unsigned shots=0;std::vector<double> times;
  for(int i=0;i<240;++i){frame.time=i/60.;controller.tick(missile,frame,random,[&](const md::WeaponFire& f){assert(f.origin.x==17&&f.origin.y==23);++shots;times.push_back(frame.time);});}
  assert(shots==2&&times[1]-times[0]>=1.&&times[1]-times[0]<1.02);
 }
 auto models=md::BattleModels::load(r);md::validation::appendAircraftModels(models);
 // Every campaign level and the copied live assembly use the size-only change.
 // Check texture dimensions and weapon tips together, including phone scaling.
 for(const auto& [name,factor]:std::vector<std::pair<std::string,float>>{{"AircraftLight",118.f/114.f},{"AircraftHeavy",1.32f*(231.f/296.f)},{"AircraftDuct",.765f*(118.f/134.f)},{"AircraftTransport",.765f*(164.f/196.f)}})
 for(unsigned level=1;level<=10;++level)for(float viewport:{.5f,1.f,1.375f}){
  const auto variant=name+"_"+(level<10?"0":"")+std::to_string(level);
  auto poses=models.pose(*models.machine(variant),viewport,.7f,-.4f,1.1f);
  md::Frame frame;models.appendSprites(frame,poses,{},[&](std::string_view n){return r.imageSize(n);});
  for(const auto& pose:poses){
   assert(std::abs(pose.deviceScale-viewport*factor)<.00001f);
   if(pose.definition->type==4){auto muzzle=pose.muzzle();assert(muzzle);auto size=r.imageSize(pose.definition->image);
    float dx=muzzle->x-pose.localCenter.x,dy=muzzle->y-pose.localCenter.y;
    assert(std::abs(dx*std::cos(pose.angle)+dy*std::sin(pose.angle)-size.x*.5f*viewport*factor)<=.51f*viewport*factor);
   }
  }
  for(const auto& sprite:frame.sprites){auto size=r.imageSize(sprite.image);
   assert(std::abs(sprite.destination.width-size.x*viewport*factor)<.0001f);
   assert(std::abs(sprite.destination.height-size.y*viewport*factor)<.0001f);
  }
 }
 for(const char* name:{"AircraftLight","AircraftHeavy","AircraftDuct","AircraftTransport"})for(float angle:{0.f,1.57079632679f,3.14159265359f,4.71238898038f}){
  auto poses=models.pose(name,1,angle,angle,angle);
  for(const auto& pose:poses)if(pose.definition->type==4){auto m=pose.muzzle();assert(m);auto size=r.imageSize(pose.definition->image);float dx=m->x-pose.localCenter.x,dy=m->y-pose.localCenter.y;float forward=dx*std::cos(angle)+dy*std::sin(angle);assert(std::abs(forward-size.x*.5f*pose.deviceScale)<=.51f*pose.deviceScale);assert(pose.definition->sockets.front().x==0&&pose.definition->sockets.front().y==0);}
 }

 for(unsigned mode=0;mode<3;++mode){md::validation::AircraftComparisonScene scene(r,mode);auto& w=*scene.world;assert(w.actors().size()==4);auto before=scene.frame();unsigned heliSprites=0,shadows=0;for(const auto& s:before.sprites)if(s.image.starts_with("ALight")||s.image.starts_with("AHeavy")){assert(s.nearest);++heliSprites;if(s.color.r==0){++shadows;assert(s.color.a==.55f);}}assert(heliSprites==20&&shadows==10);
 for(const char* name:{"ALightBody.png","ALightRotor.png","AHeavyBody.png","AHeavyRotor.png","ALightGun.png","ALightMissile.png","AHeavyGun.png","AHeavyMissile.png","AHeavyCannon.png"}){
  const md::Sprite *shadow=nullptr,*body=nullptr;for(const auto& s:before.sprites)if(s.image==name){if(s.color.r==0)shadow=&s;else body=&s;}
  assert(shadow&&body);const float z=mode==0?1.f:(mode==1?.5f:1.375f);const float gap=(std::string_view(name).starts_with("ALight")?16.f:22.f)*z;
  assert(std::abs(shadow->destination.x-body->destination.x-gap)<.001f&&std::abs(shadow->destination.y-body->destination.y-gap)<.001f);assert(shadow->rotationDegrees==body->rotationDegrees);
 }
 auto* light=w.actor(w.actors()[1]->id);float angle=light->radarAngle;w.tick();assert(std::abs(light->radarAngle-angle-7.3f*6.28318530718f/60)<.00001f);
 w.setPaused(true);angle=light->radarAngle;auto time=w.time();for(int i=0;i<60;++i)w.tick();assert(light->radarAngle==angle&&w.time()==time);w.setPaused(false);
 auto pos=light->position;scene.move();for(int i=0;i<240;++i)w.tick();assert(light->position.x!=pos.x||light->position.y!=pos.y);assert(scene.host.writes==0);assert(w.actors().size()==4);
 auto frame=scene.frame();for(const auto& s:frame.sprites){assert(std::isfinite(s.destination.x)&&std::isfinite(s.destination.y));}
 scene.fight();
 const int initialHP=w.actors()[0]->hp,heavyHP=w.actors()[2]->hp;
 bool projectiles=false;for(int i=0;i<900;++i){w.tick();projectiles|=!w.projectiles().empty();}
 assert(projectiles&&scene.host.sounds>0&&scene.host.writes==0);
 assert(w.actors()[0]->hp<initialHP&&w.actors()[2]->hp<heavyHP);
 // Starting a real stage restores its normal original spawn path.
 w.start(0);for(int i=0;i<900;++i)w.tick();assert(w.actors().size()>2);
 }
 // Every attached weapon fires independently through the original World path.
 for(const auto& binding:md::validation::aircraftWeapons){
  md::validation::AircraftComparisonScene scene(r,3);scene.fight();auto& w=*scene.world;
  md::WorldActor* shooter=nullptr;
  for(const auto& a:w.actors())if(a->flightHeight>0){
   a->flightTactics.reset();auto& guns=a->weapons;std::erase_if(guns,[&](const auto& g){return g.name!=binding.part;});
   if(!guns.empty())shooter=a.get();
  }
  assert(shooter);auto* target=w.actor(shooter->target);int hp=target->hp;
  // Put the target well inside all original ranges to test each binding.
  target->position={shooter->position.x-100,shooter->position.y};target->vehicle.position=target->position;
  for(int i=0;i<1200;++i)w.tick();
  assert(target->hp<hp&&scene.host.sounds>0&&scene.host.writes==0);
  std::cout<<binding.part<<" -> "<<binding.original<<": damage="<<(hp-target->hp)<<" sounds="<<scene.host.sounds<<"\n";
 }
 std::cout<<"AircraftTests: 3 viewports; 4 real World actors; original part assembly, independent fast rotors, pause, movement, ground shadows, no save writes, campaign restart passed\n";
}
