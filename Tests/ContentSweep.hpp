#pragma once
#include "../Sources/Core/World.hpp"
#include "../Sources/Core/Economy.hpp"
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>
namespace md::validation {
inline bool has(const Frame& f,std::string_view name){for(const auto& s:f.sprites)if(s.image==name)return true;return false;}
inline void require(bool condition,const std::string& context){if(!condition)throw std::runtime_error(context);}
inline std::pair<std::string,unsigned> ownedName(std::string_view full){
    auto split=full.rfind('_');require(split!=std::string_view::npos,"Missing original level suffix");
    return {std::string(full.substr(0,split)),unsigned(std::stoul(std::string(full.substr(split+1))))-1};
}
// Controlled content test: bases are kept alive so all production timers and
// effects can finish. This is not a natural campaign or difficulty measurement.
inline void keepBasesAlive(World& world){for(const auto& a:world.actors())if(a->base&&!a->removed)world.setValidationHealth(a->id,100000000);}
inline void step(World& world){keepBasesAlive(world);world.tick();require(world.state()==WorldState::running,"Controlled world ended");}
inline const WorldActor* producedActor(World& world,const BuildDefinition& d){
    const std::string expected=d.setType==0?d.name:(d.placementType?d.bot:d.machine);
    for(const auto& a:world.actors())if(a->player&&!a->base&&a->name==expected)return a.get();
    return nullptr;
}

struct ContentCounts {unsigned builds,placements,attacks;};
template<class Host>
ContentCounts runContentSweep(ResourceAccess& resources,Host& host,const std::function<void(const Frame&)>& inspect){
    auto table=[&](std::string_view name){auto b=resources.read("Tables/"+std::string(name));return DataTable::parse(std::string_view(reinterpret_cast<const char*>(b.data()),b.size()));};
    const auto parts=table("MachBuild_Parts.tsv");
    const auto named=table("MachBuild_Name.tsv");
    const auto specials=table("SpAttackList.tsv");
    const auto items=table("ItemInfo.tsv");
    const auto buildCatalog=BuildCatalog::load(parts,named);const AttackCatalog attacks(specials);
    require(buildCatalog.size()==110&&specials.rowCount()==27,"Original active content count changed");
    unsigned buildChecks=0,placementChecks=0,attackChecks=0;
    for(const auto geometry:{BattleGeometry::phone(),BattleGeometry::pad()}){
        auto save=SaveData::empty(1);save.createProfile("CONTENT",1);World world(resources,host,save,0,geometry,123);
        for(const auto* table:{&parts,&named})for(std::size_t row=0;row<table->rowCount();++row){
            const std::string full(table->value(row,"MachName"));const auto [name,level]=ownedName(full);
            const auto* definition=buildCatalog.find(full);require(definition!=nullptr,full+" definition");
            const auto& d=*definition;save.setBuilds(0,std::vector<OwnedBuild>{{name,level,true}});world.start(0);
            const Vec2 destination{0,-geometry.halfLength()+32};
            world.setValidationCells(float(d.cost)-1);
            require(!world.enqueueBuild(name,destination),full+" insufficient cells accepted");
            require(world.productionCount()==0&&world.cells()==float(d.cost)-1,full+" failed enqueue mutated cost/queue");
            world.setValidationCells(float(d.cost)+10);
            if(d.placementType)require(!world.enqueueBuild(name),full+" placement accepted without position");
            world.setPaused(true);require(!world.enqueueBuild(name,destination),full+" paused enqueue");world.setPaused(false);
            require(world.enqueueBuild(name,d.placementType?std::optional<Vec2>(destination):std::nullopt),full+" enqueue");
            require(world.cells()==10&&world.productionCount()==1&&world.productionOccupied(name)==1,full+" exact cost/queue");
            require(world.productionRows().front().name==full,full+" queue level");
            const auto queued=world.time();unsigned elapsed=0;
            while(world.statistics().produced==0&&elapsed++<5000)step(world);
            require(world.statistics().produced==1&&world.productionCount()==0,full+" production timeout/count");
            require(world.time()>queued,full+" production skipped timer");
            const auto* unit=producedActor(world,d);require(unit&&unit->hp>0&&!unit->assembly.empty(),full+" missing assembled unit");
            require(unit->player&&!unit->base,full+" ownership");
            if(d.placementType){
                require(unit->postBuild==d.machine&&unit->destination.x==destination.x&&unit->destination.y==destination.y,full+" carrier destination/final model");
                const auto carrier=unit->id;bool transformed=false;
                for(unsigned i=0;i<1500&&!transformed;++i){
                    if(const auto* a=world.actor(carrier);a&&!a->removed)world.setValidationHealth(carrier,1000000);
                    step(world);for(const auto& a:world.actors())transformed|=a->player&&!a->base&&a->name==d.machine&&!a->removed;
                }
                require(transformed,full+" carrier did not deploy original turret");++placementChecks;
            }else require(!unit->weapons.empty(),full+" missing original weapons");
            Frame frame;world.prepareDraw();world.appendSprites(frame);inspect(frame);
            require(host.writes==0,full+" production wrote a save file");++buildChecks;
        }
        const auto weapons=WeaponCatalog::load(table("WeaponInfo.tsv"),geometry.deviceScale==.5f);
        for(std::size_t row=0;row<specials.rowCount();++row){
            const std::string full(specials.value(row,"AttackName"));const auto [name,level]=ownedName(full);
            save.setAttacks(0,std::vector<OwnedAttack>{{name,level,2,true}});save.setBuilds(0,std::vector<OwnedBuild>{{"MachineGun",0,true}});world.start(0);
            const auto& definition=attacks.definition(save.attacks(0).front());
            char code[16];std::snprintf(code,sizeof code,"%05d",definition.itemId);const auto item=items.findRow(code);require(item.has_value(),full+" item");
            const auto prefix=items.value(*item,"strParam");const auto weaponName=specialWeaponName(prefix,*items.number(*item,"Param1"));
            const auto* weapon=weapons.find(weaponName);require(weapon,full+" weapon");
            const bool crossfire=prefix.starts_with("CF_");const Vec2 target{0,-190};
            if(crossfire){
                require(!world.activateSpecial(name,target)&&save.attacks(0).front().count==2,full+" unqualified crossfire consumed inventory");
                world.setValidationCells(500);require(world.enqueueBuild("MachineGun"),full+" qualification unit enqueue");
                for(unsigned i=0;i<1000&&world.statistics().produced==0;++i)step(world);
                WorldActor* unit=nullptr;for(const auto& a:world.actors())if(a->player&&!a->base&&a->type==0)unit=a.get();
                require(unit,full+" qualification unit missing");unit->position={0,-300};
            }
            world.setPaused(true);require(!world.activateSpecial(name,target)&&save.attacks(0).front().count==2,full+" pause consumed inventory");world.setPaused(false);
            require(world.activateSpecial(name,target)&&save.attacks(0).front().count==1,full+" activation/count");
            Frame initial;world.appendSprites(initial);require(has(initial,"Target.png"),full+" target marker");
            if(!crossfire)require(has(initial,"Airplane.png"),full+" original aircraft");
            bool fired=false,impact=false,nuclear=false,emp=false;std::size_t crossfireShots=0;
            for(const auto& p:world.projectiles())if(p.weapon==weaponName){require(p.motion.lockOn,full+" crossfire lock");++crossfireShots;fired=true;}
            if(crossfire)require(crossfireShots==weapon->shotCount,full+" crossfire shot count");
            for(unsigned i=0;i<1000;++i){
                step(world);for(const auto& p:world.projectiles())if(p.weapon==weaponName){fired=true;require(crossfire?p.motion.lockOn:p.motion.drop,full+" projectile mode");}
                world.prepareDraw();Frame frame;world.appendSprites(frame);
                impact|=!weapon->explodeImage.empty()&&has(frame,weapon->explodeImage);
                nuclear|=has(frame,"Efx_Nuclear.png");emp|=has(frame,"Efx_ExplodeBoss.png");
                inspect(frame);
                if(fired&&impact&&(weapon->type!=WeaponType::nuclear||nuclear)&&(weapon->type!=WeaponType::emp||emp))break;
            }
            require(fired,full+" no projectile");require(weapon->explodeImage.empty()||impact,full+" impact image");
            if(weapon->type==WeaponType::nuclear)require(nuclear,full+" nuclear flash");
            // Efx_EMP.png is absent in the original WeMade package. The original
            // EMP's separately configured Efx_ExplodeBoss.png must still render.
            if(weapon->type==WeaponType::emp)require(emp,full+" EMP explosion");
            require(save.attacks(0).front().count==1,full+" repeated inventory debit");
            save.setAttacks(0,std::vector<OwnedAttack>{{name,level,0,true}});require(!world.activateSpecial(name,target),full+" zero inventory accepted");
            require(host.writes==0,full+" attack wrote save file");++attackChecks;
        }
    }
    require(buildChecks==220&&placementChecks==60&&attackChecks==54,"Coverage mismatch");
    return {buildChecks,placementChecks,attackChecks};
}
} // namespace md::validation
