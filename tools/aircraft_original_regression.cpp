// Snapshot existing actors and draw commands without spawning any new aircraft.
#include "../Tests/SessionTestSupport.hpp"
#include <bit>
#include <iomanip>
struct Digest {
 std::uint64_t value=1469598103934665603ull;
 void byte(unsigned char c){value=(value^c)*1099511628211ull;}
 void text(std::string_view s){for(auto c:s)byte(c);byte(0);}
 template<class T> void number(T x){auto b=std::bit_cast<std::array<unsigned char,sizeof(T)>>(x);for(auto c:b)byte(c);}
 void vec(Vec2 p){number(p.x);number(p.y);}
 void rect(Rect r){number(r.x);number(r.y);number(r.width);number(r.height);}
 void color(Color c){number(c.r);number(c.g);number(c.b);number(c.a);}
 void frame(const Frame& f){number(f.logicalWidth);number(f.logicalHeight);color(f.background);number(f.sprites.size());
  for(const auto& s:f.sprites){text(s.image);number(bool(s.sourcePx));if(s.sourcePx)rect(*s.sourcePx);rect(s.destination);number(s.rotationDegrees);vec(s.pivot);color(s.color);number(s.blend);number(s.nearest);number(s.flipX);number(s.flipY);number(bool(s.clip));if(s.clip)rect(*s.clip);}
  number(f.meshes.size());for(const auto& m:f.meshes){text(m.image);number(m.blend);number(m.nearest);number(m.beforeSpriteIndex);number(m.vertices.size());for(const auto& v:m.vertices){vec(v.position);vec(v.uv);color(v.color);}}
 }
};
int main(int argc,char** argv){assert(argc==2);Resources r;r.root=argv[1];
 for(auto g:{BattleGeometry::phone(),BattleGeometry::pad()})for(unsigned stage=0;stage<80;++stage){
  Host host;auto save=SaveData::empty(47);save.createProfile("ORIGINAL",1);World w(r,host,save,0,g,71);w.start(stage);
  for(auto name:{"E_Tank_01","E_Plane_03","Tur_Gun_01"})w.validationSpawn(name,true,{0,-50*g.deviceScale});
  Digest d;for(unsigned tick=0;tick<360;++tick){w.tick();w.prepareDraw();Frame f;w.appendSprites(f);d.frame(f);d.number(w.time());d.number(w.cells());d.number(w.state());
   for(const auto& a:w.actors()){assert(!a->flightTactics);d.number(a->id);d.text(a->name);d.vec(a->position);d.vec(a->destination);d.number(a->hp);d.number(a->angle);d.number(a->radarAngle);d.number(a->removed);d.number(a->target);}
  }
  d.number(host.writes);for(const auto& [sound,n]:host.soundCalls){d.text(sound);d.number(n);}for(auto b:save.encode())d.byte(b);
  std::cout<<(g.deviceScale==.5f?"phone":"pad")<<' '<<stage<<' '<<std::hex<<d.value<<std::dec<<'\n';
 }
}
