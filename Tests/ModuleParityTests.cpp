// Deterministic before/after transcript. Run both builds with identical compiler,
// assets and seeds; compare stdout byte for byte. No object padding is hashed.
#include "SessionTestSupport.hpp"
#include <bit>
#include <iomanip>
#include <type_traits>
struct Digest {
    std::uint64_t value=14695981039346656037ull;
    void byte(std::uint8_t v){value^=v;value*=1099511628211ull;}
    template<class T> requires std::is_integral_v<T> void add(T v){
        std::uint64_t n=std::uint64_t(v);for(unsigned i=0;i<8;++i)byte(std::uint8_t(n>>(i*8)));
    }
    void add(float v){add(std::bit_cast<std::uint32_t>(v));}
    void add(double v){add(std::bit_cast<std::uint64_t>(v));}
    void add(std::string_view s){add(s.size());for(unsigned char c:s)byte(c);}
    void add(Vec2 p){add(p.x);add(p.y);}
    void add(Rect r){add(r.x);add(r.y);add(r.width);add(r.height);}
    void add(Color c){add(c.r);add(c.g);add(c.b);add(c.a);}
    void frame(const Frame& f){
        add(f.logicalWidth);add(f.logicalHeight);add(f.background);add(f.sprites.size());
        for(const auto& s:f.sprites){add(s.image);add(s.sourcePx.has_value());if(s.sourcePx)add(*s.sourcePx);
            add(s.destination);add(s.rotationDegrees);add(s.pivot);add(s.color);add(int(s.blend));
            add(s.nearest);add(s.flipX);add(s.flipY);add(s.clip.has_value());if(s.clip)add(*s.clip);}
        add(f.meshes.size());for(const auto& m:f.meshes){add(m.image);add(int(m.blend));add(m.beforeSpriteIndex);add(m.vertices.size());
            for(const auto& v:m.vertices){add(v.position);add(v.uv);add(v.color);}}
    }
};
struct TraceHost:Host {
    Digest audio;
    void playSound(std::string_view n,float g,float p)override {Host::playSound(n,g,p);audio.add(n);audio.add(g);audio.add(p);}
};
int main(int argc,char** argv){
    assert(argc==2);Resources resources;resources.root=argv[1];
    for(bool pad:{false,true})for(unsigned mission=0;mission<80;++mission){
        auto save=SaveData::empty(1);save.createProfile("PARITY",1);save.setCredits(0,1000000);
        TraceHost host;World world(resources,host,save,0,pad?BattleGeometry::pad():BattleGeometry::phone(),123+mission);
        world.start(mission);world.setViewport(pad?820:393,pad?1180:852,pad?0:32);
        Digest digest;
        for(unsigned tick=0;tick<900;++tick){
            if(tick==10){world.setValidationCells(10000);for(const auto& unit:save.builds(0))world.enqueueBuild(unit.name);}
            if(tick==100)world.order(1);
            if(tick==200)world.order(0);
            if(tick==300){world.setPaused(true);world.tick();world.setPaused(false);}
            world.tick();world.prepareDraw();
            digest.add(world.time());digest.add(int(world.state()));digest.add(world.cells());digest.add(world.cameraY());
            auto random=world.randomSource();digest.add(random.next());
            digest.add(world.actors().size());
            for(const auto& a:world.actors()){
                digest.add(a->id);digest.add(a->name);digest.add(a->hp);digest.add(a->maxHP);digest.add(a->player);digest.add(a->base);
                digest.add(a->removed);digest.add(a->position);digest.add(a->destination);digest.add(a->target);digest.add(a->angle);digest.add(a->radarAngle);
                for(const auto& w:a->weapons){digest.add(w.name);digest.add(w.muzzle);digest.add(w.recoil);}
            }
            digest.add(world.projectiles().size());for(const auto& p:world.projectiles()){
                digest.add(p.weapon);digest.add(p.attacker);digest.add(p.motion.position);digest.add(p.motion.angle);digest.add(p.renderOrder);}
            Frame frame;world.appendSprites(frame);digest.frame(frame);
        }
        for(auto b:save.plaintext())digest.byte(b);
        assert(host.writes==0);assert(SaveData::decode(save.encode()).encode()==save.encode());
        std::cout<<(pad?"pad":"phone")<<','<<mission<<','<<std::hex<<digest.value<<','<<host.audio.value<<std::dec<<','<<host.sounds<<'\n';
    }
}
