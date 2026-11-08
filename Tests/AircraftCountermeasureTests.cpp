#include "SessionTestSupport.hpp"
#include "Core/Simulation/AircraftCountermeasures.hpp"
#include <chrono>
int main(int argc,char** argv){
 assert(argc==2);Resources r;r.root=argv[1];
 for(float scale:{.5f,1.f}){
  AircraftCountermeasures fx(scale);assert(fx.active()==0);
  assert(fx.emit(0,{0,0},1.5707963f,60*scale));assert(fx.active()==22);
  Frame f;for(int i=0;i<30;++i)fx.tick(float(i)/60);
  fx.append(f,.5f,[](Vec2 p){return p;},[&](std::string_view n){return r.imageSize(n);});
  checkImages(f,r);assert(!f.sprites.empty()&&!f.meshes.empty());
  assert(f.meshes.size()==14); // Seven visible flare trails on each side.
  unsigned left=0,right=0;
  for(const auto& m:f.meshes){float x=0;for(const auto& v:m.vertices)x+=v.position.x; if(x<0)++left;else ++right;}
  assert(left==7&&right==7);
  for(auto& m:f.meshes)for(auto& v:m.vertices)assert(std::isfinite(v.position.x)&&std::isfinite(v.position.y));
  for(int i=30;i<=120;++i)fx.tick(float(i)/60);assert(fx.active()==0);
  for(unsigned count:{40,100}){
   std::vector<double> times;unsigned accepted=0;
   for(int repeat=0;repeat<100;++repeat){fx.clear();float now=repeat*3.f;
    auto begin=std::chrono::steady_clock::now();
    for(unsigned i=0;i<count;++i)accepted+=fx.emit(now,{float(i),0},0,60*scale);
    assert(fx.active()==132); // 6 complete fans, never a partial seventh.
    for(unsigned i=0;i<120;++i)fx.tick(now+float(i)/60);
    assert(fx.active()==0);
    times.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count()/120);
   }
   std::sort(times.begin(),times.end());assert(accepted==600);
   std::cout<<"scale="<<scale<<" simultaneous="<<count<<" max_particles=132 amortized_cpu_p99_ms="<<times[98]<<"\n";
  }
 }
 Host host;auto save=SaveData::empty(1);save.createProfile("FLARE",1);
 World w(r,host,save,0);w.start(0);w.validationAircraftDodgeComparison();
 for(auto& a:w.actors())if(a->flightTactics)a->weapons.clear();
 for(int i=0;i<1800&&w.countermeasureBursts()==0;++i)w.tick();
 assert(w.countermeasureBursts()>0&&w.countermeasureParticles()>0);
 const auto bursts=w.countermeasureBursts(),particles=w.countermeasureParticles();const double time=w.time();
 w.setPaused(true);for(int i=0;i<120;++i)w.tick();
 assert(w.time()==time&&w.countermeasureBursts()==bursts&&w.countermeasureParticles()==particles);
 Frame f;w.appendSprites(f);checkImages(f,r);assert(host.writes==0);
 std::cout<<"Real dodge trigger, capped visuals, finite trails, expiry, pause, no save writes passed\n";
}
