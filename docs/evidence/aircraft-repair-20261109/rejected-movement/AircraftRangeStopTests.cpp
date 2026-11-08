#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include <cmath>
int main(int argc,char**argv){
 assert(argc==2);Resources r;r.root=argv[1];unsigned cases=0;
 for(auto geometry:{BattleGeometry::phone(),BattleGeometry::pad()})
 for(const auto& spec:alliedAircraftCatalog)for(unsigned level:{1u,10u}){
  Host host;auto save=SaveData::empty(1);save.createProfile("RANGE",1);
  World world(r,host,save,0,geometry,123);world.start(0);world.validationAircraftComparison();
  for(const auto& actor:world.actors())actor->removed=true;
  char suffix[8];std::snprintf(suffix,sizeof suffix,"_%02u",level);
  const auto id=world.validationSpawn(std::string(spec.model)+suffix,true,{0,-150*geometry.deviceScale});
  auto* aircraft=world.actor(id);const float range=aircraft->range;
  const auto targetId=world.validationSpawn("E_Tank_32",false,{0,aircraft->position.y+range*1.4f});
  auto* target=world.actor(targetId);target->weapons.clear();target->attackCheck=target->enemyAI.stopDelay=1e9f;
  target->hp=target->maxHP=10000000;aircraft->target=targetId;
  float minimum=1e9f;
  for(unsigned tick=0;tick<600;++tick){world.tick();minimum=std::min(minimum,std::hypot(aircraft->position.x-target->position.x,aircraft->position.y-target->position.y));}
  const auto stopped=aircraft->position;
  assert(aircraft->aircraft.speed==0);
  assert(minimum>range*.45f&&minimum<range);
  assert(target->hp<10000000&&host.sounds>0);
  for(unsigned tick=0;tick<120;++tick)world.tick();
  assert(aircraft->position.x==stopped.x&&aircraft->position.y==stopped.y);
  // Follow only after the assigned target exits the unchanged unit range.
  target->position.y=stopped.y+range*1.2f;target->vehicle.position=target->position;
  for(unsigned tick=0;tick<120;++tick)world.tick();
  assert(aircraft->position.y>stopped.y+geometry.deviceScale);
  for(unsigned tick=0;tick<480;++tick)world.tick();
  assert(aircraft->aircraft.speed==0&&aircraft->range==range);
  // A new target already inside range must not start an orbit.
  world.damage(targetId,target->hp);
  auto replacementId=world.validationSpawn("E_Tank_32",false,{aircraft->position.x+range*.6f,aircraft->position.y});
  auto* replacement=world.actor(replacementId);replacement->weapons.clear();replacement->attackCheck=replacement->enemyAI.stopDelay=1e9f;replacement->hp=replacement->maxHP=10000000;
  const auto before=aircraft->position;
  for(unsigned tick=0;tick<180;++tick)world.tick();
  assert(aircraft->target==replacementId&&aircraft->aircraft.speed==0);
  assert(aircraft->position.x==before.x&&aircraft->position.y==before.y);
  assert(host.writes==0);++cases;
 }
 std::cout<<"AircraftRangeStop: "<<cases<<" phone/pad Lv1/10 approach, stationary firing, target exit, reacquisition, unchanged ranges, zero writes passed\n";
}
