#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include <set>
int main(int argc,char** argv){assert(argc==2);Resources r;r.root=argv[1];
 auto catalog=WeaponCatalog::load(DataTable::load(r.root/"Tables/WeaponInfo.tsv"),true);
 for(const auto& spec:alliedAircraftCatalog)for(unsigned lv=1;lv<=10;++lv){
  Host host;auto save=SaveData::empty(1);save.createProfile("ARMS",1);World world(r,host,save,0);world.start(0);world.validationAlliedAircraftComparison();
  for(auto& a:world.actors())a->removed=true;
  char suffix[8];std::snprintf(suffix,8,"_%02u",lv);
  auto aid=world.validationSpawn(std::string(spec.model)+suffix,true,{0,-60});auto* air=world.actor(aid);
  auto tid=world.validationSpawn("Tur_Gun_01",false,{0,30});auto* target=world.actor(tid);target->weapons.clear();target->hp=target->maxHP=10000000;
  air->target=tid;air->angle=1.5707963f;
  const bool cheyenne=spec.model=="AircraftDuct";
  assert(air->weapons.size()==(spec.model=="AircraftHeavy"?5:spec.model=="AircraftTransport"?4:2));
  std::set<std::string> sounds;for(auto& gun:air->weapons){auto def=catalog.find(gun.name);assert(def&&def->shotRange==air->range);sounds.insert(def->fireSound);}
  bool checkedSalvo=false;for(unsigned tick=0;tick<600;++tick){world.tick();
   if(cheyenne&&!checkedSalvo&&!world.projectiles().empty()){
    std::vector<const WorldProjectile*> shots;for(auto& p:world.projectiles())if(p.attacker==aid)shots.push_back(&p);
    assert(shots.size()==7);unsigned center=0,spread=0;std::set<std::pair<float,float>> destinations;
    for(auto* p:shots){const auto delta=Vec2{p->motion.destination.x-target->position.x,p->motion.destination.y-target->position.y};float distance=std::hypot(delta.x,delta.y);
     if(distance<.001f)++center;else{assert(std::abs(distance-catalog.find(p->weapon)->damageRange*.5f)<.001f);++spread;}
     destinations.insert({p->motion.destination.x,p->motion.destination.y});
    }assert(center==1&&spread==6&&destinations.size()==7);checkedSalvo=true;
   }
  }
  assert(target->hp<10000000);for(auto& s:sounds)assert(host.soundCalls[s]>0);
  if(cheyenne)assert(checkedSalvo);
  if(spec.model=="AircraftHeavy"){auto cannon=catalog.find("AHeavyCannon"+std::string(suffix));assert(cannon->type==WeaponType::bullet&&cannon->speed>catalog.find("E_Tank31_Wpn")->speed*2.9f);}
  assert(host.writes==0);Frame f;world.prepareDraw();world.appendSprites(f);checkImages(f,r);
 }
 std::cout<<"40 loadouts: requested weapon counts, live firing, sounds, damage, same aircraft range, Cheyenne 1+6 impact points, high-speed cannon, no saves passed\n";
}
