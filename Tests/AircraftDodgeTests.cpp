#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include <chrono>
#include <set>
#include <mach/mach.h>
#include <thread>
static std::uint64_t memory(){mach_task_basic_info_data_t i{};mach_msg_type_number_t n=MACH_TASK_BASIC_INFO_COUNT;task_info(mach_task_self(),MACH_TASK_BASIC_INFO,reinterpret_cast<task_info_t>(&i),&n);return i.resident_size;}
static void stress(Resources& r){
 Host host;auto save=SaveData::empty(1);save.createProfile("MEMORY",1);World w(r,host,save,0);w.start(0);w.validationAlliedAircraftComparison();for(auto& a:w.actors()){a->weapons.clear();a->removed=true;}
 for(unsigned count:{10,50,100,250,500}){
  std::vector<ActorId> ids;for(unsigned i=0;i<count;++i){auto id=w.validationSpawn("AircraftLight_01",true,{float(int(i%20)*10-100),float(int(i/20)*10-100)});ids.push_back(id);w.actor(id)->weapons.clear();}
  std::vector<double> times;
  for(int tick=0;tick<180;++tick){auto start=std::chrono::steady_clock::now();w.tick();auto end=std::chrono::steady_clock::now();times.push_back(std::chrono::duration<double,std::milli>(end-start).count());}
  std::sort(times.begin(),times.end());std::cout<<"live="<<count<<" stored="<<w.actors().size()<<" rss="<<memory()<<" core_p99_ms="<<times[178]<<"\n";
  for(auto id:ids){auto* a=w.actor(id);a->flightTactics->dodging=false;w.damage(id,a->hp);}
  for(int t=0;t<330;++t)w.tick();
  assert(w.actors().size()==4);
  std::cout<<"after_destroy="<<count<<" stored="<<w.actors().size()<<" rss="<<memory()<<"\n";
 }
 std::cout<<"WorldActor_bytes="<<sizeof(WorldActor)<<" AircraftTactics_bytes="<<sizeof(AircraftTactics)<<" writes="<<host.writes<<"\n";
}
int main(int argc,char**argv){assert(argc>=2);Resources r;r.root=argv[1];if(argc>2){stress(r);return 0;}
 auto catalog=WeaponCatalog::load(DataTable::load(r.root/"Tables/WeaponInfo.tsv"),true);auto weapon=*catalog.find("E_Turret13_Wpn_01");DarwinRandom rng(1);
 auto bullet=Projectile::fire(weapon,{-45,0},{0,0},0,0,rng);bullet.toPlayer=true;
 AircraftThreatGrid grid;grid.clear(.5f);grid.add(bullet,weapon);assert(grid.size()==1);
 std::set<double> cooldowns;std::set<std::pair<int,int>> destinations;
 for(unsigned i=1;i<=128;++i){AircraftTactics t;t.random=i*2654435761u|1u;
  assert(t.updateDodge(0,{0,0},true,1,6,256,512,grid));assert(t.invulnerable(0)&&t.dodgeEnd==.45);
  destinations.insert({int(t.dodgeDestination.x),int(t.dodgeDestination.y)});
  t.updateDodge(.45,{0,0},true,1,6,256,512,grid);assert(!t.invulnerable(.45)&&!t.dodging);
  assert(t.dodgeReady>=.70&&t.dodgeReady<=.95);cooldowns.insert(t.dodgeReady);
  assert(!t.updateDodge(t.dodgeReady-.0001,{0,0},true,1,6,256,512,grid));
  assert(t.updateDodge(t.dodgeReady+.0001,{0,0},true,1,6,256,512,grid));
  auto arrival=t.dodgeDestination;t.updateDodge(t.dodgeReady+.01,arrival,true,1,6,256,512,grid);assert(!t.dodging);
 }
 assert(cooldowns.size()>60&&destinations.size()>=10);
 AircraftTactics friendly;assert(!friendly.updateDodge(0,{0,0},false,1,6,256,512,grid));
 auto retained=grid.retainedBytes();for(int j=0;j<10000;++j){grid.clear(.5f);grid.add(bullet,weapon);}assert(retained==grid.retainedBytes());
 Host host;auto save=SaveData::empty(1);save.createProfile("DODGE",1);World w(r,host,save,0);w.start(0);
 auto id=w.validationSpawn("AircraftLight_01",true,{0,0});auto* a=w.actor(id);a->flightTactics->dodging=true;a->flightTactics->dodgeEnd=w.time()+.45;
 int hp=a->hp,sounds=host.sounds;w.damage(id,50);assert(a->hp==hp&&host.sounds==sounds+1);
 a->flightTactics->finishDodge(w.time());w.damage(id,50);assert(a->hp==hp-50);
 auto normal=w.validationSpawn("E_Tank_01",false,{10,0});auto* n=w.actor(normal);hp=n->hp;w.damage(normal,10);assert(n->hp==hp-10);
 // No wall-clock timers: paused simulation retains remaining immunity/cooldown.
 a->flightTactics->dodging=true;a->flightTactics->dodgeEnd=w.time()+.45;auto now=w.time();w.setPaused(true);for(int i=0;i<120;++i)w.tick();assert(w.time()==now&&a->flightTactics->invulnerable(w.time()));
 assert(host.writes==0);
 std::cout<<"AircraftDodge passed: 128 independent cooldowns, 12 directions, arrival/timeout, faction, retained threat storage, immunity preserves hit sound, cooldown vulnerable, pause, original damage, no saves\n";
}
