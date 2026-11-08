#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include "Core/Simulation/Projectile.hpp"
#include <set>
#include <iomanip>

static void mission55(Resources& r,unsigned level){
 Host host;auto save=SaveData::empty(55);save.createProfile("EVASION",1);
 save.setBuilds(0,std::vector<OwnedBuild>{{"ScoutHeli",level-1,true}});
 auto weapons=WeaponCatalog::load(DataTable::load(r.root/"Tables/WeaponInfo.tsv"),true);
 World w(r,host,save,0,BattleGeometry::phone(),55);w.start(54);w.setValidationCells(100000);
 // The base survives the observation window; no aircraft/enemy health or weapons are changed.
 for(auto& a:w.actors())if(a->base&&a->player)a->hp=a->maxHP=100000000;
 for(int i=0;i<3;++i)assert(w.enqueueBuild("ScoutHeli"));
 for(auto& a:w.actors())if(!a->player){std::cout<<"placed enemy="<<a->name<<" x="<<a->position.x<<" y="<<a->position.y;for(auto& gun:a->weapons){auto* def=weapons.find(gun.name);std::cout<<" weapon="<<gun.name<<" type="<<int(def->type)<<" damage="<<def->damage<<" splash="<<def->damageRange<<" speedPerTick="<<def->speed/.0166f;}std::cout<<"\n";}
 std::set<ActorId> seen;unsigned damagedFrames=0;std::map<ActorId,int> previousHP;std::map<ActorId,double> died;
 for(int tick=0;tick<7200&&w.state()==WorldState::running;++tick){w.tick();for(auto& a:w.actors())if(a->flightTactics){
  if(seen.insert(a->id).second)std::cout<<"spawn level="<<level<<" id="<<a->id<<" x="<<a->position.x<<" y="<<a->position.y<<" hp="<<a->hp<<"\n";
  if(previousHP.contains(a->id)&&a->hp<previousHP[a->id])++damagedFrames;
  previousHP[a->id]=a->hp;if(a->hp==0&&!died.contains(a->id))died[a->id]=w.time();
 }}
 std::cout<<"mission55 level="<<level<<" produced="<<seen.size()<<" deaths="<<died.size()<<" damageFrames="<<damagedFrames<<" enemyDestroyed="<<w.statistics().enemyDestroyed<<" writes="<<host.writes<<"\n";
 for(auto [id,t]:died)std::cout<<"death id="<<id<<" second="<<t<<"\n";
 assert(host.writes==0);
}
int main(int argc,char**argv){assert(argc>=2);Resources r;r.root=argv[1];
 if(argc==3){mission55(r,1);mission55(r,10);return 0;}
 auto table=[&](const char* n){return DataTable::load(r.root/"Tables"/(std::string(n)+".tsv"));};
 auto builds=BuildCatalog::load(table("MachBuild_Parts"),table("MachBuild_Name"));
 for(auto g:{BattleGeometry::phone(),BattleGeometry::pad()}){
  ProductionQueue queue;float cells=100000;for(int i=0;i<3;++i)assert(queue.enqueue(*builds.find("ScoutHeli_01"),0,1,cells));
  DarwinRandom random(55),untouched(55);auto created=queue.tick(100,g,random);std::set<float> xs;
  for(auto& a:created){xs.insert(a.position.x);assert(std::abs(a.position.x)<=220*g.deviceScale);assert(a.position.y==-g.halfLength());}
  assert(created.size()==3&&xs.size()==3);assert(random.next()==untouched.next());
  // Approach from all twelve directions, then hold the assigned range.
  for(unsigned sector=0;sector<12;++sector){
   AircraftTactics flight;AircraftMotion motion;motion.moveSpeed=2*g.deviceScale;
   const float angle=sector*6.28318530718f/12;
   motion.position={std::sin(angle)*220*g.deviceScale,std::cos(angle)*220*g.deviceScale};
   for(int tick=0;tick<600;++tick){motion.setDestination(flight.destination(tick/60.,motion.position,{0,0},200*g.deviceScale,512*g.deviceScale,g.halfLength(),12*g.deviceScale));flight.advance(motion);
    assert(std::isfinite(motion.position.x)&&std::abs(motion.position.x)<=244*g.deviceScale+.01f);
   }
   const float distance=std::hypot(motion.position.x,motion.position.y);
   assert(motion.speed==0&&distance<200*g.deviceScale&&distance>100*g.deviceScale);
  }
 }
 // Actual unmodified original weapon rows: after firing, target movement cannot move the destination.
 auto weapons=WeaponCatalog::load(table("WeaponInfo"),true);unsigned checked=0;
 auto wt=table("WeaponInfo");for(auto& row:wt.rows()){
  const auto* weapon=weapons.find(row[0]);if(weapon->type==WeaponType::gun||weapon->type==WeaponType::sniper||weapon->speed<=0)continue;
  DarwinRandom rng(17);auto bullet=Projectile::fire(*weapon,{0,-100},{0,0},1.57079632679f,0,rng);assert(!bullet.lockOn);
  bool impact=false;for(int i=0;i<10000&&!impact;++i)impact=bullet.tick(*weapon);
  assert(impact&&bullet.destination.x==0&&bullet.destination.y==0);
  auto p=bullet.position;const float radius=12,range=std::max(float(weapon->damageRange),std::max(radius,weapon->special1));
  std::vector<ProjectileTarget> enemies{{{p.x+range+1,p.y},100000,radius}},players=enemies;
  assert(projectileCollide(*weapon,p,false,0,enemies,players).empty());
  assert(projectileCollide(*weapon,p,true,0,enemies,players).empty());
  enemies={{{p.x,p.y},100000,radius}};players=enemies;
  assert(!projectileCollide(*weapon,p,false,0,enemies,players).empty());
  if(weapon->damageRange>0){players={{{p.x+weapon->damageRange*.75f,p.y},100000,radius}};assert(!projectileCollide(*weapon,p,true,0,enemies,players).empty());}
  ++checked;
 }
 std::cout<<"AircraftEvasion: independent same-tick production/12 approach directions/range hold, "<<checked<<" original and additional projectile rows: outside radius misses, direct and splash hits passed\n";
}
