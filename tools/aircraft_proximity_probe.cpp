#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include <iomanip>
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];
for(auto g:{BattleGeometry::phone(),BattleGeometry::pad()})for(const auto& spec:alliedAircraftCatalog){
 Host host;auto save=SaveData::empty(1);save.createProfile("PROXIMITY",1);World w(r,host,save,0,g);w.start(0);auto id=w.validationSpawn(std::string(spec.model)+"_01",true,{0,0});auto* a=w.actor(id);
 float worst=1e9f;unsigned seedWorst=0,tickWorst=0;float radius=0;
 for(unsigned seed=1;seed<=32;++seed){auto t=*a->flightTactics;auto m=a->aircraft;t.random=seed*2654435761u|1u;m.position={0,-a->range*.95f};m.direction={};m.moveSpeed=t.speed*g.deviceScale;
  for(unsigned tick=0;tick<1200;++tick){auto p=t.destination(tick*.0166,m.position,{0,0},a->range,512*g.deviceScale,g.halfLength(),a->radius);if(tick==0)radius=std::hypot(p.x,p.y);m.setDestination(p);t.advance(m);float d=std::hypot(m.position.x,m.position.y);if(d<worst){worst=d;seedWorst=seed;tickWorst=tick;}}
 }
 std::cout<<(g.deviceScale==.5f?"phone":"pad")<<" "<<spec.label<<" range="<<a->range<<" bodyRadius="<<a->radius<<" orbit="<<radius<<" closest="<<worst<<" seed="<<seedWorst<<" tick="<<tickWorst<<"\n";
}}
