#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include "Core/Simulation/AircraftVisual.hpp"
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];
 for(const auto& spec:alliedAircraftCatalog){Host host;auto save=SaveData::empty(1);save.createProfile("HANDLING",1);World w(r,host,save,0);w.start(0);auto id=w.validationSpawn(std::string(spec.model)+"_01",true,{0,0});auto* a=w.actor(id);auto enemy=w.validationSpawn("Tur_Gun_01",false,{40,40});auto* e=w.actor(enemy);e->weapons.clear();e->hp=e->maxHP=10000000;a->target=enemy;
  for(int tick=0;tick<200;++tick)w.tick();
  const auto beforeAngle=a->angle;const auto second=w.validationSpawn("Tur_Gun_01",false,{-40,-40});auto* next=w.actor(second);next->weapons.clear();next->hp=next->maxHP=10000000;w.damage(enemy,e->hp);a->target=second;
  bool changed=false,banked=false,mesh=false;float previous=a->angle;
  for(int tick=0;tick<400;++tick){std::vector<float> guns;for(const auto& gun:a->weapons)guns.push_back(gun.controller.angle());w.tick();float step=std::abs(std::remainder(a->angle-previous,6.28318530718f));assert(step<=a->flightTactics->noseStep+.00001f);previous=a->angle;changed|=std::abs(std::remainder(a->angle-beforeAngle,6.28318530718f))>.3f;
   for(size_t k=0;k<guns.size();++k)assert(std::abs(std::remainder(a->weapons[k].controller.angle()-guns[k],6.28318530718f))<=a->flightTactics->weaponStep+.00001f);
   banked|=std::abs(a->flightTactics->bank)>.15f;Frame f;w.appendSprites(f);for(const auto& m:f.meshes)if(m.image.starts_with("A")){mesh=true;for(const auto& v:m.vertices)assert(std::isfinite(v.position.x)&&std::isfinite(v.position.y)&&v.uv.x>=0&&v.uv.x<=1&&v.uv.y>=0&&v.uv.y<=1);}
  }
  assert(changed&&banked&&mesh&&host.writes==0);
  w.damage(id,a->hp);w.tick();assert(a->hp==0&&a->flightHeight==0&&a->flightTactics->bank==0&&a->aircraft.speed==0);
  Frame dead;w.appendSprites(dead);bool shadow=false;for(const auto& s:dead.sprites)if(s.image.starts_with(spec.model=="AircraftLight"?"ALight":spec.model=="AircraftHeavy"?"AHeavy":spec.model=="AircraftDuct"?"ADuct":"ATransport")&&s.color.r==0){assert(s.color.a==.18f);shadow=true;}assert(shadow);
 }
 // Production table -> actual motion: every level retains Scout speed and halves Gunship speed.
 for(unsigned level=1;level<=10;++level){
  Host host;auto save=SaveData::empty(1);save.createProfile("SPEED",1);World w(r,host,save,0);w.start(0);
  char suffix[8];std::snprintf(suffix,sizeof suffix,"_%02u",level);
  auto scoutId=w.validationSpawn(std::string("AircraftLight")+suffix,true,{0,0});
  auto heavyId=w.validationSpawn(std::string("AircraftHeavy")+suffix,true,{0,0});
  auto scout=w.actor(scoutId)->aircraft,heavy=w.actor(heavyId)->aircraft;
  assert(std::abs(heavy.moveSpeed*2-scout.moveSpeed)<.00001f);
  assert(std::abs(w.actor(scoutId)->flightTactics->speed-(2.f+.03f*(level-1)))<.00001f);
  AircraftTactics lightTactics,heavyTactics;scout.position=heavy.position={0,0};
  scout.setDestination({100000,0});heavy.setDestination({100000,0});
  for(int tick=0;tick<120;++tick){lightTactics.advance(scout);heavyTactics.advance(heavy);}
  auto lightStart=scout.position.x,heavyStart=heavy.position.x;
  for(int tick=0;tick<600;++tick){lightTactics.advance(scout);heavyTactics.advance(heavy);}
  assert(std::abs((heavy.position.x-heavyStart)*2-(scout.position.x-lightStart))<.1f);
 }
 // Acceleration into each lateral direction produces the corresponding roll.
 for(float sign:{-1.f,1.f}){AircraftTactics t;t.engaged=true;AircraftMotion m;m.angle=1.57079632679f;m.moveSpeed=2;m.setDestination({sign*10000,0});for(int i=0;i<20;++i)t.advance(m);assert(t.bank*sign<0);}
 std::cout<<"AircraftHandling: 4 target switches, bounded nose and weapon turns, bidirectional banking, textured rotor projection, grounded wreck/contact shadows, no writes passed\n";
}
