#include "Tests/SessionTestSupport.hpp"
#include <chrono>
#include <algorithm>
#include <iomanip>
struct Overlay:Resources {
 std::filesystem::path baseline;
 Bytes read(std::string_view p)override {
  if(!baseline.empty()&&std::filesystem::exists(baseline/p)){
   std::ifstream f(baseline/p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};
  }
  return Resources::read(p);
 }
};
int main(int argc,char**argv){
 assert(argc==3);std::cout<<"version,round,units,bank,samples,mean_us,p50_us,p95_us,sprites,meshes,writes,sound_calls\n";
 for(unsigned round=0;round<3;++round)for(unsigned order=0;order<2;++order){bool baseline=(order+round)%2==0;
  for(unsigned count:{1,4,40,100})for(bool bank:{false,true}){
   Overlay resources;resources.root=argv[1];if(baseline)resources.baseline=argv[2];
   Host host;auto save=SaveData::empty(1);save.createProfile("PERF",1);World world(resources,host,save,0,BattleGeometry::pad(),123);world.start(0);world.setViewport(640,800);
   for(auto&a:world.actors())a->removed=true;
   for(unsigned i=0;i<count;++i){auto id=world.validationSpawn("AircraftTransport_10",true,{-180.f+40.f*(i%10),-180.f+40.f*(i/10)});auto*a=world.actor(id);a->weapons.clear();a->angle=1.57079632679f;a->radarAngle=0;a->flightTactics->bank=bank?.27925268f:0;a->flightTactics->speed=0;}
   std::vector<double> times;size_t sprites=0,meshes=0;
   for(unsigned n=0;n<220;++n){auto t=std::chrono::steady_clock::now();world.prepareDraw();Frame f;f.logicalWidth=640;f.logicalHeight=800;world.appendSprites(f);auto end=std::chrono::steady_clock::now();if(n>=20)times.push_back(std::chrono::duration<double,std::micro>(end-t).count());sprites=f.sprites.size();meshes=f.meshes.size();}
   double sum=0;for(auto t:times)sum+=t;std::sort(times.begin(),times.end());
   std::cout<<(baseline?"baseline":"split")<<","<<round<<","<<count<<","<<(bank?16:0)<<","<<times.size()<<","<<std::fixed<<std::setprecision(3)<<sum/times.size()<<","<<times[times.size()/2]<<","<<times[times.size()*95/100]<<","<<sprites<<","<<meshes<<","<<host.writes<<","<<host.sounds<<"\n";
  }
 }
}
