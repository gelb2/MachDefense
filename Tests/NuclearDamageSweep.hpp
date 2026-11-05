#pragma once
#include "ContentSweep.hpp"
#include <array>
#include <map>
#include <sstream>

namespace md::validation {
// Expected values come from WeMade 1.0.3 WeaponInfo and GobBullet::setCollide
// (0xa568): D * min(1, 1.5 - distance/range), truncated, strictly inside range.
inline std::string runNuclearDamageSweep(ResourceAccess& resources,HostServices& host) {
    const auto bytes=resources.read("Tables/WeaponInfo.tsv");
    const auto table=DataTable::parse({reinterpret_cast<const char*>(bytes.data()),bytes.size()});
    constexpr std::array damage{1500,2000,2700,3700,5000};
    constexpr std::array radius{300,320,360,400,450};
    std::ostringstream report;
    for(const auto geometry:{BattleGeometry::phone(),BattleGeometry::pad()}) {
        auto weapons=WeaponCatalog::load(table,geometry.deviceScale==.5f);
        for(unsigned level=0;level<5;++level) {
            const auto name=specialWeaponName("AS_NUCLEAR",float(level+1));
            const auto* weapon=weapons.find(name);require(weapon,name+" missing");
            const float range=radius[level]*geometry.deviceScale;
            require(weapon->damage==damage[level]&&weapon->damageRange==range,name+" original table/scaling");
            const std::array<float,7> distances{0,.25f,.5f,.75f,.99f,1,1.01f};
            const std::array<int,7> expected{damage[level],damage[level],damage[level],int(damage[level]*.75f),int(damage[level]*.51f),0,0};
            for(unsigned i=0;i<distances.size();++i){
                std::vector<ProjectileTarget> enemy{{{distances[i]*range,0},100000,1}};
                std::vector<ProjectileTarget> friendlies{{{0,0},100000,1}};
                projectileCollide(*weapon,{0,0},false,0,enemy,friendlies);
                require(100000-enemy[0].hp==expected[i],name+" radial boundary "+std::to_string(i));
                require(friendlies[0].hp==100000,name+" friendly fire");
                enemy[0].hp=1200;
                projectileCollide(*weapon,{0,0},false,0,enemy,friendlies);
                require(enemy[0].hp==std::max(0,1200-expected[i]),name+" lethal radius "+std::to_string(i));
            }
            // Use the production air-strike/drop/impact path; only other weapons
            // are disabled in this isolated fixture to measure nuclear damage.
            auto save=SaveData::empty(1);save.createProfile("NUCLEAR",1);
            save.setAttacks(0,std::vector<OwnedAttack>{{"AirStrike-Nuclear",level,2,true}});
            World world(resources,host,save,0,geometry,123);world.start(0);
            ActorId base=0;for(const auto& a:world.actors())if(a->base&&!a->player)base=a->id;
            require(base!=0,name+" enemy base");const Vec2 target=world.actor(base)->position;
            const auto ground=world.validationSpawn("E_Tank_01",false,{target.x+20*geometry.deviceScale,target.y-20*geometry.deviceScale});
            const auto friendly=world.validationSpawn("E_Tank_01",true,{target.x-20*geometry.deviceScale,target.y-20*geometry.deviceScale});
            const auto air=world.validationSpawn("E_Plane_01",false,{target.x,target.y-30*geometry.deviceScale});
            for(const auto& a:world.actors())world.setValidationHealth(a->id,100000);
            const auto weak=world.validationSpawn("E_Tank_01",false,{target.x+10*geometry.deviceScale,target.y-30*geometry.deviceScale});
            const auto strong=world.validationSpawn("E_Plane_03",false,{target.x+10*geometry.deviceScale,target.y-40*geometry.deviceScale});
            // GlobalInfo::openMachInfo (0x1c304) doubles MachList HP.
            require(world.actor(weak)->hp==1200&&world.actor(strong)->hp==7600,name+" original enemy HP");
            require(world.activateSpecial("AirStrike-Nuclear",target),name+" activation");
            require(save.attacks(0)[0].count==1,name+" inventory");
            bool impacted=false;int baseDamage=0,groundDamage=0,airDamage=0;Vec2 impact{};
            for(unsigned tick=0;tick<600&&!impacted;++tick){
                std::map<ActorId,int> hp;
                for(const auto& a:world.actors()){a->weapons.clear();hp[a->id]=a->hp;}
                bool pending=false;
                for(const auto& p:world.projectiles())if(p.weapon==name){auto next=p.motion;if(next.tick(*weapon)){pending=true;impact=next.position;}}
                world.tick();
                if(!pending)continue;
                impacted=true;
                for(auto id:{base,ground,air,friendly,weak,strong}){
                    const auto* a=world.actor(id);require(a!=nullptr,name+" actor disappeared");
                    const float distance=std::hypot(a->position.x-impact.x,a->position.y-impact.y);
                    const int expectedHP=std::max(0,hp[id]-(a->player||distance>=range?0:int(float(damage[level])*std::min(1.f,1.5f-distance/range))));
                    require(a->hp==expectedHP,name+" world HP "+std::to_string(id));
                    if(id==weak||id==strong)report<<name<<" target="<<a->name<<" before="<<hp[id]<<" distance="<<distance<<" radius="<<range<<" after="<<a->hp<<"\n";
                }
                baseDamage=hp[base]-world.actor(base)->hp;groundDamage=hp[ground]-world.actor(ground)->hp;airDamage=hp[air]-world.actor(air)->hp;
                require(baseDamage>0&&groundDamage>0,name+" no effective damage");
                require(world.actor(weak)->hp==0,name+" 1200 HP enemy survived center");
                require(world.actor(strong)->hp>0,name+" 7600 HP enemy killed by <=5000 damage");
            }
            require(impacted,name+" impact timeout");
            report<<(geometry.deviceScale==.5f?"phone":"pad")<<" level="<<level+1<<" damage="<<damage[level]<<" range="<<range<<" base="<<baseDamage<<" ground="<<groundDamage<<" air="<<airDamage<<" friendly=0 weakHP="<<world.actor(weak)->hp<<" strongHP="<<world.actor(strong)->hp<<" killed="<<world.statistics().enemyDestroyed<<"\n";
        }
    }
    return report.str();
}
}
