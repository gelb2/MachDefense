#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include <sstream>
struct Before {unsigned endTick=0;Vec2 destination{};};
int main(int argc,char**argv){
 assert(argc==3);Resources resources;resources.root=argv[1];
 std::ifstream input(argv[2]);assert(input);std::string line;std::getline(input,line);
 std::map<std::string,Before> before;
 while(std::getline(input,line)){
  std::istringstream row(line);std::vector<std::string> fields;std::string field;
  while(std::getline(row,field,','))fields.push_back(field);assert(fields.size()==15);
  const auto key=fields[0]+","+fields[1]+","+fields[2]+","+fields[3];
  before[key]={unsigned(std::stoul(fields[4])),{std::stof(fields[12]),std::stof(fields[13])}};
 }
 unsigned cases=0;
 for(bool phone:{false,true})for(const auto& spec:alliedAircraftCatalog)for(unsigned level:{1u,10u})for(unsigned velocity=0;velocity<4;++velocity){
  const auto geometry=phone?BattleGeometry::phone():BattleGeometry::pad();Host host;auto save=SaveData::empty(1);save.createProfile("CURVE",1);World world(resources,host,save,0,geometry);world.start(0);
  char suffix[8];std::snprintf(suffix,sizeof suffix,"_%02u",level);auto id=world.validationSpawn(std::string(spec.model)+suffix,true,{0,0});auto* actor=world.actor(id);
  auto tactic=*actor->flightTactics;auto motion=actor->aircraft;motion.moveSpeed=tactic.speed*geometry.deviceScale;
  motion.direction=velocity==0?Vec2{}:velocity==1?Vec2{motion.moveSpeed,0}:velocity==2?Vec2{-motion.moveSpeed,0}:Vec2{0,motion.moveSpeed};
  motion.speed=std::hypot(motion.direction.x,motion.direction.y);motion.state=0;tactic.advance(motion);
  auto weapons=WeaponCatalog::load(DataTable::load(resources.root/"Tables/WeaponInfo.tsv"),phone);auto weapon=*weapons.find("E_Turret13_Wpn_01");DarwinRandom random(1);
  auto bullet=Projectile::fire(weapon,{-45*geometry.deviceScale,0},{0,0},0,0,random);bullet.toPlayer=true;AircraftThreatGrid threats;threats.clear(geometry.deviceScale);threats.add(bullet,weapon);
  const std::string key=std::string(phone?"phone":"pad")+","+std::string(spec.model)+","+std::to_string(level)+","+std::to_string(velocity);
  const auto baseline=before.at(key);
  unsigned endTick=0;float previousDistance=-1,previousSpeed=-1,peak=-1,peakDistance=0,largestStep=0;bool accelerating=false,decelerating=false;
  for(unsigned tick=0;tick<40;++tick){
   const double now=tick*double(.0166f);
   if(!tactic.updateDodge(now,motion.position,true,motion.moveSpeed,actor->radius,512*geometry.deviceScale,geometry.halfLength(),threats)){endTick=tick;break;}
   assert(tactic.invulnerable(now)&&tactic.dodgeEnd==.45);
   motion.setDestination(tactic.dodgeDestination);tactic.advance(motion);
   const float distance=std::hypot(motion.position.x,motion.position.y),half=tactic.dodgeLength*.5f;
   assert(std::isfinite(distance)&&distance<=tactic.dodgeLength+.0001f&&distance>=previousDistance);
   if(previousDistance>=0){
    largestStep=std::max(largestStep,distance-previousDistance);
    if(distance<half){assert(motion.speed>previousSpeed);accelerating=true;}
    if(previousDistance>half){assert(motion.speed<previousSpeed);decelerating=true;}
   }
   if(motion.speed>peak){peak=motion.speed;peakDistance=distance;}
   previousDistance=distance;previousSpeed=motion.speed;
  }
  assert(endTick==baseline.endTick&&endTick>0);
  assert(std::hypot(tactic.dodgeDestination.x-baseline.destination.x,tactic.dodgeDestination.y-baseline.destination.y)<.0001f);
  assert(motion.position.x==tactic.dodgeDestination.x&&motion.position.y==tactic.dodgeDestination.y&&motion.speed==0);
  assert(accelerating&&decelerating&&std::abs(peakDistance-tactic.dodgeLength*.5f)<=largestStep+.0001f);
  assert(host.writes==0);++cases;
 }
 assert(cases==before.size()&&cases==64);
 // An immobile validation unit cannot divide by a zero-duration dodge.
 AircraftTactics stopped;AircraftThreatGrid empty;empty.clear(1);
 assert(!stopped.updateDodge(0,{0,0},true,0,12,512,512,empty));
 std::cout<<"AircraftDodgeCurve: 64 original arrival ticks/destinations retained, midpoint acceleration/deceleration, exact endpoint and zero terminal speed, zero writes passed\n";
}
