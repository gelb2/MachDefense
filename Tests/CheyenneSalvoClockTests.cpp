#include "SessionTestSupport.hpp"
#include <set>
#include <limits>
int main(int argc,char**argv){
 assert(argc==2);Resources r;r.root=argv[1];
 // Reproduce the existing controller's float/double boundary without changing it.
 const double upward=std::nextafter(5.,0.);
 assert(double(float(upward))>upward);
 for(double now:{upward,5.,299./60.,5.1})for(bool normalized:{false,true}){
  unsigned shots=0;
  for(unsigned count:{4u,3u}){
   WeaponDefinition d;d.shotCount=count;d.shotDelay=0;d.reloadDelay=0;
   WeaponController controller;WeaponFrame f;f.time=normalized?double(float(now)):now;f.manualFire=true;
   DarwinRandom rng(1);controller.tick(d,f,rng,[&](const WeaponFire&){++shots;});
  }
  assert(shots==(!normalized&&now==upward?2u:7u));
 }
 unsigned cases=0,up=0,down=0;
 for(auto geometry:{BattleGeometry::phone(),BattleGeometry::pad()})for(unsigned level=1;level<=10;++level)
 for(unsigned wait:{0u,4u,18u,58u,59u,118u,179u,299u}){
  Host host;auto save=SaveData::empty(1);save.createProfile("CLOCK",1);
  World world(r,host,save,0,geometry);world.start(0);
  for(unsigned i=0;i<wait;++i)world.tick();
  for(auto& actor:world.actors())actor->removed=true;
  char model[32];std::snprintf(model,sizeof model,"AircraftDuct_%02u",level);
  // Keep the first-frame projectiles clear of immediate impacts even at Lv10 splash radius.
  auto aid=world.validationSpawn(model,true,{0,0});auto tid=world.validationSpawn("Tur_Gun_01",false,{140*geometry.deviceScale,0});
  auto* air=world.actor(aid);auto* target=world.actor(tid);target->weapons.clear();target->hp=target->maxHP=10000000;
  air->angle=air->aircraft.angle=0;air->target=tid;
  for(auto& gun:air->weapons){gun.controller.setAngle(0);gun.controller.setReload(-100);}
  world.tick();
  const double rawTime=world.time(),floatTime=double(float(rawTime));up+=floatTime>rawTime;down+=floatTime<rawTime;
  std::vector<const WorldProjectile*> shots;for(auto& p:world.projectiles())if(p.attacker==aid)shots.push_back(&p);
  if(shots.size()!=7)std::cerr<<"level="<<level<<" wait="<<wait<<" time="<<rawTime<<" scale="<<geometry.deviceScale<<" shots="<<shots.size()<<"\n";
  assert(shots.size()==7);unsigned center=0,spread=0;std::set<std::pair<float,float>> impacts;
  const auto catalog=WeaponCatalog::load(DataTable::load(r.root/"Tables/WeaponInfo.tsv"),geometry.deviceScale==.5f);
  for(const auto* shot:shots){
   const float distance=std::hypot(shot->motion.destination.x-target->position.x,shot->motion.destination.y-target->position.y);
   if(distance<.001f)++center;else{assert(std::abs(distance-catalog.find(shot->weapon)->damageRange*.5f)<.001f);++spread;}
   impacts.insert({shot->motion.destination.x,shot->motion.destination.y});
  }
  assert(center==1&&spread==6&&impacts.size()==7&&host.writes==0);++cases;
 }
 assert(up>0&&down>0);
 std::cout<<cases<<" live Cheyenne salvos: 7 same-tick missiles, center1/spread6; rounded-up="<<up<<" rounded-down="<<down<<"; original controller boundary reproduced unchanged\n";
}
