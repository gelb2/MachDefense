#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include <iomanip>
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];
 std::cout<<"device,model,level,velocity,tick,time,dodging,x,y,vx,vy,speed,destinationX,destinationY,immunityEnd\n"<<std::setprecision(12);
 for(bool phone:{false,true})for(const auto& spec:alliedAircraftCatalog)for(unsigned level:{1u,10u})for(unsigned velocity=0;velocity<4;++velocity){
  const auto g=phone?BattleGeometry::phone():BattleGeometry::pad();Host host;auto save=SaveData::empty(1);save.createProfile("PROFILE",1);World world(r,host,save,0,g);world.start(0);
  char suffix[8];std::snprintf(suffix,sizeof suffix,"_%02u",level);auto id=world.validationSpawn(std::string(spec.model)+suffix,true,{0,0});auto* actor=world.actor(id);
  auto tactic=*actor->flightTactics;auto motion=actor->aircraft;motion.moveSpeed=tactic.speed*g.deviceScale;
  motion.direction=velocity==0?Vec2{}:velocity==1?Vec2{motion.moveSpeed,0}:velocity==2?Vec2{-motion.moveSpeed,0}:Vec2{0,motion.moveSpeed};motion.speed=std::hypot(motion.direction.x,motion.direction.y);motion.state=0;tactic.advance(motion);
  auto weapons=WeaponCatalog::load(DataTable::load(r.root/"Tables/WeaponInfo.tsv"),phone);auto weapon=*weapons.find("E_Turret13_Wpn_01");DarwinRandom random(1);auto bullet=Projectile::fire(weapon,{-45*g.deviceScale,0},{0,0},0,0,random);bullet.toPlayer=true;AircraftThreatGrid threats;threats.clear(g.deviceScale);threats.add(bullet,weapon);
  for(unsigned tick=0;tick<40;++tick){const double now=tick*double(.0166f);bool dodge=tactic.updateDodge(now,motion.position,true,motion.moveSpeed,actor->radius,512*g.deviceScale,g.halfLength(),threats);
   if(dodge){motion.setDestination(tactic.dodgeDestination);tactic.advance(motion);}
   std::cout<<(phone?"phone":"pad")<<','<<spec.model<<','<<level<<','<<velocity<<','<<tick<<','<<now<<','<<dodge<<','<<motion.position.x<<','<<motion.position.y<<','<<motion.direction.x<<','<<motion.direction.y<<','<<motion.speed<<','<<tactic.dodgeDestination.x<<','<<tactic.dodgeDestination.y<<','<<tactic.dodgeEnd<<'\n';
   if(!dodge)break;
  }
 }
}
