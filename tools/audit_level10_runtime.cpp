#include "Core/Simulation/WeaponController.hpp"
#include "Core/Simulation/Projectile.hpp"
#include <array>
#include <filesystem>
#include <iostream>
#include <stdexcept>

// Read-only audit. No player save, renderer, or platform service is opened.
int main(int argc,char** argv) {
    if(argc!=2)throw std::runtime_error("Pass Resources/Tables");
    auto table=md::DataTable::load(std::filesystem::path(argv[1])/"WeaponInfo.tsv");
    const std::array prefixes{"Turret_gun_","Turret_msl_","Turret_mg_","Turret_ion_",
        "Turret_lsr_","Turret_pla_","BaseWpn","AS_BOMB_","AS_MISSILE_",
        "AS_NUCLEAR_","AS_EMP_","CF_MISSILE_"};
    unsigned boundaries=0,collisions=0;
    std::cout<<"weapon,geometry,damage,ui_attack,shot_range,damage_radius,shots,shot_delay,reload,special1,special2\n";
    for(bool phone:{true,false})for(auto prefix:prefixes)for(unsigned level=1;level<=10;++level) {
        const auto name=std::string(prefix)+(level<10?"0":"")+std::to_string(level);
        auto row=table.findRow(name);if(!row)throw std::runtime_error(name);
        auto w=md::WeaponDefinition::fromTable(table,*row,phone);
        for(float offset:{-1.f,0.f,1.f}) {
            md::WeaponController controller;md::DarwinRandom random(1);md::WeaponFrame frame;
            frame.time=100;frame.target=md::WeaponTarget{{float(w.shotRange)+offset,0},1000000};
            const auto result=controller.tick(w,frame,random,[](const auto&){});
            if((result.shots>0)!=(offset<0))throw std::runtime_error(name+" firing boundary");
            ++boundaries;
        }
        std::array<md::ProjectileTarget,1> targets{{{{0,0},1000000,10}}};
        auto hits=md::projectileCollide(w,{0,0},false,100,targets,{});
        if(hits.size()!=1||hits.front().damage!=w.damage||targets.front().hp!=1000000-w.damage)
            throw std::runtime_error(name+" center damage");
        ++collisions;
        std::cout<<name<<','<<(phone?"phone":"pad")<<','<<w.damage<<','<<w.attackPoint()<<','
            <<w.shotRange<<','<<w.damageRange<<','<<unsigned(w.shotCount)<<','<<w.shotDelay<<','
            <<w.reloadDelay<<','<<w.special1<<','<<w.special2<<'\n';
    }
    std::cerr<<"120 weapon levels x 2 geometries: "<<boundaries<<" range boundaries, "
        <<collisions<<" center damage checks passed\n";
}
