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
 // Seven original-style tiles: neutral uses the approved dimensions; rolls
 // select six distinct source rectangles without shearing the drawn body.
 for(const auto& body:std::array<std::pair<const char*,Vec2>,4>{{
  {"ALightBody",{62,39}},{"AHeavyBody",{76,112}},
  {"ADuctBody",{110,88}},{"ATransportBody",{140,131}}}}){
  const auto models=BattleModels::load(r);const auto* definition=models.part(body.first);
  assert(definition&&definition->tileCount==7&&definition->splitX==7);
  const auto size=r.imageSize(definition->image);assert(size.x==body.second.x*7&&size.y==body.second.y);
  Sprite sprite;sprite.image=definition->image;sprite.sourcePx=Rect{0,0,body.second.x,body.second.y};
  sprite.destination={10,20,body.second.x,body.second.y};sprite.nearest=true;
  for(unsigned tile=0;tile<7;++tile){
   const float bank=tile==0?0.f:(tile<=3?-float(tile):float(tile-3))*.13962634f;
   auto mesh=aircraftBankMesh(sprite,{100,100},0,bank,size);
   assert(mesh.nearest&&mesh.image==sprite.image&&mesh.vertices.size()==4);
   assert(std::abs(mesh.vertices[0].uv.x-float(tile)/7)<.000001f);
   assert(std::abs(mesh.vertices[1].uv.x-float(tile+1)/7)<.000001f);
   assert(mesh.vertices[0].position.x==10&&mesh.vertices[0].position.y==20);
   assert(mesh.vertices[3].position.x==10+body.second.x&&mesh.vertices[3].position.y==20+body.second.y);
  }
 }
 // A banked part must retain the neutral part's texture sampling.
 for(bool nearest:{false,true}){Sprite s;s.image="ALightBody";s.nearest=nearest;s.destination={0,0,20,60};
  auto m=aircraftBankMesh(s,{10,30},1.5707963f,.5f,{20,60});assert(m.nearest==nearest);
 }
 // Opposite rolls must be visually distinct; all part/muzzle coordinates use
 // this same projection, so bank changes cannot detach a mounted weapon.
 {auto a=aircraftBankPoint({10,5},0,.5f),b=aircraftBankPoint({10,5},0,-.5f);
  assert(std::abs(a.x-b.x)>1.f);assert(std::abs(a.y-b.y)<.0001f);
  auto n=aircraftBankPoint({10,5},0,0);assert(n.x==10&&n.y==5);
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
