#pragma once
#include "ContentSweep.hpp"
#include <array>
#include <set>

namespace md::validation {
namespace damage_audio_detail {
struct Host final:HostServices {
    HostServices& destination;unsigned writes=0;std::vector<std::string> sounds;
    explicit Host(HostServices& h):destination(h){}
    std::optional<Bytes> readSave()override{return {};}
    bool writeSave(std::span<const std::uint8_t>)override{++writes;return false;}
    void playSound(std::string_view n,float gain,float pan)override{sounds.emplace_back(n);destination.playSound(n,gain,pan);}
    void playMusic(std::string_view,float)override{}
    void stopMusic()override{}
    void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}
};
inline std::string hit(unsigned value){return "SFX_HitBody0"+std::to_string(value%3+1)+".wav";}
inline unsigned hitCount(const Host& host){unsigned n=0;for(const auto& s:host.sounds)if(s.starts_with("SFX_HitBody"))++n;return n;}
inline void isolate(World& world,ActorId keep){for(const auto& a:world.actors()){
    if(!a->base&&a->id!=keep)a->removed=true;
    a->weapons.clear();a->target=0;a->attackCheck=a->targetCheck=a->collisionCheck=100000;
    a->enemyAI.stopDelay=a->playerAI.stopDelay=100000;a->moveToTarget=false;
}}
}

// GobHvMach::onDamage:From: 0x13fd0 plays one of SFX8..10 after
// onCrash returns. No RNG is consumed by the audio host itself.
inline std::string runDamageAudioSweep(ResourceAccess& resources,HostServices& destination){
    using namespace damage_audio_detail;
    Host host(destination);std::set<std::string> variants;unsigned cases=0;
    for(auto geometry:{BattleGeometry::phone(),BattleGeometry::pad()}){
        auto save=SaveData::empty(1);save.createProfile("HITAUDIO",1);
        World world(resources,host,save,0,geometry,123);world.start(0);
        const auto victim=world.validationSpawn("E_Tank_01",false,{0,0});auto* a=world.actor(victim);
        for(unsigned seed=1;seed<=30;++seed){
            a->hp=a->maxHP=10000;world.randomSource()=DarwinRandom(seed);DarwinRandom expected(seed);
            const auto sound=hit(expected.next());host.sounds.clear();world.damage(victim,7);
            require(a->hp==9993,"Hit audio changed direct damage");
            require(host.sounds==std::vector<std::string>{sound},"Original nonlethal hit cue missing/order");
            require(world.randomSource().next()==expected.next(),"Nonlethal hit RNG must advance once");variants.insert(sound);++cases;
        }
        // Fatal enemy: explosion reset, drop roll, wreck lifetime, then hit cue.
        for(bool player:{false,true}){
            world.start(0);const auto id=world.validationSpawn("E_Tank_01",player,{0,0});auto* doomed=world.actor(id);
            doomed->coinRate=0;doomed->coin=0;
            constexpr unsigned seed=61;DarwinRandom expected(seed);world.randomSource()=DarwinRandom(seed);
            expected.next();if(!player)expected.next();
            const float life=float(expected.next())*0x1p-31f*2.f;
            const auto sound=hit(expected.next());host.sounds.clear();world.damage(id,doomed->hp);
            require(doomed->hp==0&&std::abs(doomed->crashTime-float(world.time()+3.0+double(life)))<.00001f,"Fatal hit RNG changed wreck lifetime");
            require(host.sounds==std::vector<std::string>{"SFX_ExplMach.wav",sound},"Fatal hit sound must follow crash");
            require(world.randomSource().next()==expected.next(),"Fatal hit RNG order");
            const auto rng=world.randomSource();host.sounds.clear();world.damage(id,7);world.damage(999999,7);
            auto copy=rng;require(host.sounds.empty()&&world.randomSource().next()==copy.next(),"Invalid/dead target consumed sound RNG");++cases;
        }
        // Successful drop values must be determined before the fatal hit draw.
        world.start(0);const auto enemy=world.validationSpawn("E_Tank_01",false,{0,0});a=world.actor(enemy);a->coin=600;a->coinRate=100;
        DarwinRandom expected(71);world.randomSource()=DarwinRandom(71);
        expected.next();expected.next();
        const int mineral=int(600.f+(float(expected.next())*0x1p-31f)*(600.f/3.f));
        for(unsigned i=0;i<3;++i)expected.next();
        int credit=0;if((expected.next()&1)==0){credit=int(600.f+(float(expected.next())*0x1p-31f)*(600.f/3.f));for(unsigned i=0;i<3;++i)expected.next();}
        const float life=float(expected.next())*0x1p-31f*2.f;const auto sound=hit(expected.next());
        host.sounds.clear();world.damage(enemy,a->hp);int actualMineral=0,actualCredit=0;
        for(const auto& l:world.loot())(l.definition.type==LootType::mineral?actualMineral:actualCredit)+=l.value;
        require(actualMineral==mineral&&actualCredit==credit,"Hit cue changed current enemy drops");
        require(a->crashTime==float(world.time()+3.0+double(life))&&host.sounds.back()==sound,"Hit cue precedes fatal RNG");
        require(world.randomSource().next()==expected.next(),"Fatal loot/hit RNG tail");++cases;
        // Railgun status invokes the same damage callback; EMP alone does not.
        for(bool plasma:{true,false}){
            world.start(0);const auto id=world.validationSpawn("E_Tank_01",false,{0,0});a=world.actor(id);a->hp=a->maxHP=100000;
            a->specialDamage.add({plasma?WeaponType::railgun:WeaponType::emp,7.f,.2f,float(world.time()),float(world.time()+2)});
            host.sounds.clear();int totalDamage=0;
            for(unsigned i=0;i<90;++i){isolate(world,id);const int hp=a->hp;const auto sounds=host.sounds.size();world.tick();const int loss=hp-a->hp;
                require(loss==0||(plasma&&loss==7),"Status damage amount changed");totalDamage+=loss;
                require(host.sounds.size()==sounds+(loss?2:0),"Status cue cadence");
                if(loss)require(host.sounds[sounds].starts_with("SFX_HitBody")&&host.sounds[sounds+1]=="Missile06.wav","Compatibility raw source 15 must follow onDamage audio");
            }
            require(hitCount(host)==unsigned(totalDamage/7),"Status damage hit audio count");
            require(plasma?totalDamage>0:totalDamage==0,"Status damage eligibility");
            const auto count=host.sounds.size();world.setPaused(true);for(unsigned i=0;i<60;++i)world.tick();
            require(host.sounds.size()==count,"Paused status played sound");++cases;
        }
        world.start(0);const auto doomed=world.validationSpawn("E_Tank_01",false,{0,0});a=world.actor(doomed);a->hp=a->maxHP=7;a->coinRate=0;
        a->specialDamage.add({WeaponType::railgun,7.f,0.f,float(world.time()),float(world.time()+1)});
        isolate(world,doomed);host.sounds.clear();world.tick();
        require(a->hp==0&&host.sounds.size()==3,"Fatal plasma cue count");
        require(host.sounds[0]=="SFX_ExplMach.wav"&&host.sounds[1].starts_with("SFX_HitBody")&&host.sounds[2]=="Missile06.wav","Fatal plasma crash/hit/raw15 order");++cases;
    }
    require(variants.size()==3&&cases==72&&host.writes==0,"Hit audio sweep coverage/save writes");
    return "Damage audio: 72 phone/pad cases, all three hit cues, fatal/drop RNG order, plasma raw-source cue order/cadence, silent EMP, no save writes";
}
}
