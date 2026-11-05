#pragma once
#include "ContentSweep.hpp"
#include <array>
#include <sstream>

namespace md::validation {
namespace base_effect_detail {
// Protect the caller's save while retaining its audio backend for device tests.
struct Host final:HostServices {
    HostServices& destination; unsigned writes=0;
    explicit Host(HostServices& h):destination(h){}
    std::optional<Bytes> readSave()override{return {};}
    bool writeSave(std::span<const std::uint8_t>)override{++writes;return false;}
    void playSound(std::string_view n,float g,float p)override{destination.playSound(n,g,p);}
    void playMusic(std::string_view n,float g)override{destination.playMusic(n,g);}
    void stopMusic()override{destination.stopMusic();}
    void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}
};
inline DataTable table(ResourceAccess& r,std::string_view name){
    const auto b=r.read("Tables/"+std::string(name)+".tsv");
    return DataTable::parse({reinterpret_cast<const char*>(b.data()),b.size()});
}
inline SaveData profile(){auto s=SaveData::empty(1);s.createProfile("BASETEST",1);s.setCredits(0,100000000);return s;}
inline WorldActor& base(World& w,bool player=true){
    for(const auto& a:w.actors())if(a->base&&a->player==player)return *a;
    throw std::runtime_error("Base effect test missing base");
}
inline bool containsPart(const std::vector<AssemblyPart>& parts,std::string_view name){
    for(const auto& p:parts)if(p.name==name||containsPart(p.children,name))return true;
    return false;
}
inline void appearance(World& w,unsigned bodyLevel,unsigned radarLevel,const std::string& context){
    char body[32],radar[32];std::snprintf(body,sizeof body,"Base%02u",bodyLevel+1);
    std::snprintf(radar,sizeof radar,"Base%02u_Rader",radarLevel+1);
    require(containsPart(base(w).assembly,body),context+" body appearance");
    require(containsPart(base(w).assembly,radar),context+" radar appearance");
}
inline void freeze(World& w,ActorId armed=0,ActorId collector=0){
    for(const auto& p:w.actors()){
        auto& a=*p;
        if(!a.base&&a.id!=collector)a.removed=true;
        if(a.id!=armed)a.weapons.clear();
        a.attackCheck=a.targetCheck=a.collisionCheck=1000000;
        a.playerAI.stopDelay=a.enemyAI.stopDelay=1000000;
        a.moveToTarget=false;a.destination=a.position;
        a.bipod.state=a.vehicle.state=a.aircraft.state=0;
        if(a.id!=armed)a.target=0;
    }
}
inline void buy(World& w,SaveData& s,const BaseUpgradeCatalog& c,BaseUpgrade type,unsigned level){
    for(unsigned i=0;i<level;++i){
        const auto credits=s.credits(0);const auto purchase=c.purchase(s,0,type);
        require(purchase&&purchase->newLevel==i+1,"Base effect purchase level");
        require(s.credits(0)==credits-purchase->cost,"Base effect purchase debit");
        w.refreshBaseUpgrades();
    }
}
struct Drops {int mineral=0,coin=0;unsigned count=0;};
inline Drops drops(const World& w){Drops r;for(const auto& l:w.loot()){
    if(l.definition.type==LootType::mineral)r.mineral+=l.value;
    else if(l.definition.type==LootType::coin)r.coin+=l.value;
    ++r.count;
}return r;}
inline void killForLoot(World& w,unsigned seed,int amount=60){
    const auto id=w.validationSpawn("E_Tank_01",false,{0,0});auto* a=w.actor(id);
    require(a,"Base effect loot enemy");a->coin=amount;a->coinRate=100;
    // Match the crash RNG on both worlds, including explosion particles.
    w.randomSource()=DarwinRandom(seed);w.damage(id,a->hp);
}
inline void collect(World& w,SaveData& s,int expectedCredits,float naturalRate){
    const auto collector=w.validationSpawn("E_Tank_01",true,{0,0});
    auto* a=w.actor(collector);a->type=0;a->hp=a->maxHP=100000;
    const auto creditStart=s.credits(0);
    for(unsigned tick=0;tick<300&&!w.loot().empty();++tick){
        freeze(w,0,collector);
        a->position=a->destination=w.loot().front().position;
        a->bipod.position=a->bipod.destination=a->position;
        const auto remaining=drops(w);const float before=w.cells();w.tick();
        const int minerals=remaining.mineral-drops(w).mineral;
        const float noGeneration=std::min(w.cellCapacity(),before+float(minerals));
        const float withGeneration=std::min(w.cellCapacity(),before+(before<w.cellCapacity()?naturalRate:0)+float(minerals));
        require(std::abs(w.cells()-noGeneration)<.0005f||std::abs(w.cells()-withGeneration)<.0005f,"Base effect mineral pickup amount/natural generation");
    }
    require(w.loot().empty(),"Base effect pickup timeout");
    require(s.credits(0)==creditStart+expectedCredits,"Base effect actual credit pickup");
}
} // namespace base_effect_detail

// WeMade 1.0.3 references: updateBaseUpgrade 0x1a53c, onCrash 0xdf1c,
// pickupItem 0x1048c, processGameRule 0x160ec, BuildItem setup 0x304d8,
// setStage 0x1627c, and DlgUpgradeBase::setUpgrade 0x344cc.
// This measures purchased upgrade effects in controlled live World instances.
inline std::string runBaseUpgradeEffectSweep(ResourceAccess& resources,HostServices& destination){
    using namespace base_effect_detail;
    Host host(destination);
    const BaseUpgradeCatalog catalog(table(resources,"BaseUpgrade"));
    const auto builds=BuildCatalog::load(table(resources,"MachBuild_Parts"),table(resources,"MachBuild_Name"));
    const auto* production=builds.find("MachineGun_01");require(production,"Base effect production definition");
    constexpr std::array hp{10000,15000,20000,30000,50000,70000,100000};
    constexpr std::array damage{10,12,14,16,20,25,30};
    constexpr std::array range{300,340,380,420,460,500,540};
    constexpr std::array buildFactor{1.f,.96f,.92f,.88f,.84f,.8f,.76f,.72f,.68f,.64f};
    constexpr std::array creditFactor{1.f,1.2f,1.4f,1.6f,1.8f,2.f,2.2f,2.4f,2.7f,3.f};
    constexpr std::array cellRate{1.f,1.1f,1.2f,1.3f,1.4f,1.5f,1.6f,1.8f,2.f,2.2f};
    constexpr std::array capacity{500.f,700.f,1000.f,1400.f,1800.f,2300.f,2900.f,3400.f,4000.f,4700.f};
    unsigned checks=0,purchases=0,creditPickups=0,cellPickups=0,capacityClamps=0,appearanceChecks=0;
    for(auto geometry:{BattleGeometry::phone(),BattleGeometry::pad()}){
        for(unsigned category=0;category<6;++category){
            const auto type=BaseUpgrade(category);
            for(unsigned level=0;level<=catalog.maximumLevel(type);++level){
                auto save=profile();World world(resources,host,save,0,geometry,123);world.start(0);
                if(type==BaseUpgrade::defense)world.damage(base(world).id,123);
                buy(world,save,catalog,type,level);purchases+=level;
                const std::string context="Base upgrade "+std::to_string(category)+" level "+std::to_string(level);
                appearance(world,0,0,context+" before next stage");
                switch(type){
                case BaseUpgrade::cannon:{
                    auto& friendly=base(world);auto& hostile=base(world,false);
                    char name[32];std::snprintf(name,sizeof name,"BaseWpn%02u",level+1);
                    require(friendly.weapons.size()==2,context+" two guns");
                    for(const auto& gun:friendly.weapons)require(gun.name==name,context+" weapon binding");
                    require(friendly.range==int(float(range[level])*1.5f*geometry.deviceScale),context+" range");
                    hostile.position={friendly.position.x,friendly.position.y+60};hostile.hp=hostile.maxHP=1000000;
                    friendly.target=hostile.id;
                    unsigned shots=0;
                    for(unsigned i=0;i<400&&shots<4;++i){
                        freeze(world,friendly.id);const int before=hostile.hp;world.tick();const int loss=before-hostile.hp;
                        require(loss>=0&&loss%damage[level]==0,context+" per-shot damage");shots+=unsigned(loss/damage[level]);
                    }
                    require(shots>=4,context+" guns did not fire");
                    world.start(1);appearance(world,0,level,context+" next stage");++appearanceChecks;break;
                }
                case BaseUpgrade::defense:{
                    auto& a=base(world);require(a.maxHP==hp[level],context+" maximum HP");
                    require(a.hp==hp[level]-(level?0:123),context+" upgrade healing");
                    world.damage(a.id,321);require(a.hp==hp[level]-(level?0:123)-321,context+" damage after upgrade");
                    world.start(1);appearance(world,level,0,context+" next stage");++appearanceChecks;break;
                }
                case BaseUpgrade::buildTime:{
                    freeze(world);world.setValidationCells(500);
                    require(world.enqueueBuild("MachineGun"),context+" enqueue");const double queued=world.time();
                    const float duration=production->duration*buildFactor[level];
                    double previous=queued;unsigned ticks=0;
                    while(world.productionCount()&&ticks++<1000){
                        previous=world.time();freeze(world);world.tick();
                        if(world.productionCount())require(std::abs(world.productionRows().front().progress-float((world.time()-queued)/double(duration)))<.00001f,context+" production progress");
                    }
                    require(world.statistics().produced==1&&world.productionCount()==0,context+" production completion");
                    require(previous-queued<=duration+.00001&&world.time()-queued>duration,context+" production duration");break;
                }
                case BaseUpgrade::creditResearch:
                case BaseUpgrade::cellResearch:{
                    if(type==BaseUpgrade::cellResearch){
                        freeze(world);world.setValidationCells(0);unsigned increments=0;
                        for(unsigned i=0;i<150;++i){const float before=world.cells();world.tick();const float delta=world.cells()-before;
                            require(std::abs(delta)<.0001f||std::abs(delta-cellRate[level])<.0001f,context+" natural cell increment");if(delta>.001f)++increments;freeze(world);
                        }
                        require(increments>=25&&increments<=31,context+" natural cell timing");
                        const float before=world.cells();world.setPaused(true);for(unsigned i=0;i<20;++i)world.tick();
                        require(world.cells()==before,context+" paused cells");world.setPaused(false);
                        // Restart keeps purchased levels and resets the test scene.
                        world.start(0);
                    }
                    auto referenceSave=profile();World reference(resources,host,referenceSave,0,geometry,123);reference.start(0);
                    // Find a deterministic two-item drop; never replace the game's drop rule.
                    unsigned seed=1;Drops expected,actual;
                    for(;seed<=100;++seed){
                        world.start(0);reference.start(0);killForLoot(world,seed);killForLoot(reference,seed);
                        expected=drops(reference);actual=drops(world);if(expected.coin>0)break;
                    }
                    require(seed<=100&&actual.count==2&&expected.count==2,context+" credit drop fixture");
                    require(actual.mineral==expected.mineral&&actual.mineral>=60&&actual.mineral<81,context+" research changed dropped cells");
                    if(type==BaseUpgrade::creditResearch){
                        // The multiplier precedes integer truncation in the original.
                        const float lower=float(expected.coin)*creditFactor[level];
                        const float upper=float(expected.coin+1)*creditFactor[level];
                        require(actual.coin>=int(lower)&&actual.coin<int(std::ceil(upper)),context+" credit multiplier");
                    }else require(actual.coin==expected.coin,context+" cell research changed credit drop");
                    require(save.credits(0)==100000000-[&]{int sum=0;for(unsigned i=0;i<level;++i)sum+=catalog.level(type,i).cost;return sum;}(),context+" credits awarded before pickup");
                    world.setValidationCells(0);
                    collect(world,save,actual.coin,type==BaseUpgrade::cellResearch?cellRate[level]:1.f);++creditPickups;++cellPickups;
                    require(world.cells()>=actual.mineral&&world.cells()<world.cellCapacity(),context+" untruncated actual mineral pickup");break;
                }
                case BaseUpgrade::cellStorage:{
                    require(world.cellCapacity()==capacity[level],context+" live capacity");
                    world.start(0);require(world.cells()==capacity[level]*.4f,context+" initial cells");
                    for(unsigned temporary=0;temporary<5;++temporary){
                        require(world.cellUpgradeCost()==int(capacity[level]*float(temporary+1)*.6f),context+" cell upgrade cost");
                        const auto cost=world.cellUpgradeCost();world.setValidationCells(float(cost));
                        require(world.upgradeCells()&&world.cells()==0,context+" cell upgrade debit");
                        require(world.cellCapacity()==capacity[level]*float(temporary+2),context+" temporary storage growth");
                    }
                    require(!world.upgradeCells(),context+" temporary maximum");
                    world.start(0);killForLoot(world,1,10000);const auto items=drops(world);
                    world.setValidationCells(world.cellCapacity()-1);collect(world,save,items.coin,1.f);
                    require(world.cells()==capacity[level],context+" actual mineral capacity clamp");++capacityClamps;break;
                }
                }
                require(world.state()==WorldState::running,context+" unexpected mission outcome");
                require(host.writes==0,context+" unexpected save write");++checks;
            }
        }
        // Changing the guns must not silently apply a pending body upgrade.
        auto save=profile();World world(resources,host,save,0,geometry,123);world.start(0);
        buy(world,save,catalog,BaseUpgrade::defense,1);buy(world,save,catalog,BaseUpgrade::cannon,1);
        world.start(1);appearance(world,1,1,"Combined upgrades initial stage");
        for(unsigned level=2;level<=3;++level){
            require(catalog.purchase(save,0,BaseUpgrade::defense).has_value(),"Combined defense purchase");world.refreshBaseUpgrades();
            require(catalog.purchase(save,0,BaseUpgrade::cannon).has_value(),"Combined cannon purchase");world.refreshBaseUpgrades();
            appearance(world,1,1,"Combined upgrades pending stage");
        }
        world.start(2);appearance(world,3,3,"Combined upgrades next stage");++appearanceChecks;purchases+=6;
        require(host.writes==0,"Combined upgrades unexpected save write");
    }
    require(checks==108&&creditPickups==40&&cellPickups==40&&capacityClamps==20&&appearanceChecks==30,"Base upgrade effect coverage");
    std::ostringstream out;out<<"Base upgrade effects: "<<checks<<" phone/pad level cases, "<<purchases
        <<" purchases, "<<creditPickups<<" credit pickups, "<<cellPickups<<" mineral pickups, "<<capacityClamps
        <<" capacity clamps, "<<appearanceChecks<<" appearance transitions; no save writes";
    return out.str();
}
} // namespace md::validation
