#include "Core/Contracts/Game.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include "Core/Simulation/PlayerParts.hpp"
#include "Core/Simulation/Combat.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
struct Resources:md::ResourceAccess{
 std::filesystem::path root;
 md::Bytes read(std::string_view p)override{std::ifstream f(root/p,std::ios::binary);if(!f)throw std::runtime_error(std::string(p));return {std::istreambuf_iterator<char>(f),{}};}
 md::Vec2 imageSize(std::string_view)override{return {};}
};
int main(int argc,char**argv){if(argc!=2)return 2;Resources r;r.root=argv[1];auto t=[&](const char*n){return md::DataTable::load(r.root/"Tables"/(std::string(n)+".tsv"));};
 auto models=md::BattleModels::load(r);auto machine=t("MachList"),wt=t("WeaponInfo");auto weapons=md::WeaponCatalog::load(wt,false),phone=md::WeaponCatalog::load(wt,true);auto builds=md::BuildCatalog::load(t("MachBuild_Parts"),t("MachBuild_Name"));
 std::cout<<"unit\tlevel\tmission\thp\tcells\tupgradeCredits\tbuildSeconds\tqueueLimit\tspeedPerTick\tradius\theight\tnoseDegreesPerSecond\tweaponDegreesPerSecond\tweapon\tcount\ttype\tdamage\trange\tphoneRange\tsplashRadius\tburst\tshotDelay\treloadDelay\tbulletSpeed\taccuracyParameter\thudAttackPoint\n";
 for(const auto& a:md::alliedAircraftCatalog)for(unsigned level=1;level<=10;level++){
 char suffix[8];snprintf(suffix,sizeof suffix,"_%02u",level);auto d=builds.find(std::string(a.name)+suffix);auto m=md::MachineDefinition::fromTable(machine,*machine.findRow(d->machine),md::BattleGeometry::pad());std::map<std::string,int> counts;
 auto walk=[&](auto&& self,const auto& nodes)->void{for(const auto& n:nodes){if(weapons.find(n.name))++counts[n.name];self(self,n.children);}};walk(walk,*models.machine(d->machine));
 for(const auto& [name,count]:counts){auto w=weapons.find(name),p=phone.find(name);auto row=*wt.findRow(name);
 std::cout<<a.label<<'\t'<<level<<'\t'<<a.mission<<'\t'<<m.hp<<'\t'<<d->cost<<'\t'<<d->upgradeCost<<'\t'<<d->duration<<'\t'<<d->maximumConcurrent<<'\t'<<m.speed<<'\t'<<m.radius<<'\t'<<a.height<<'\t'<<a.noseDegreesPerSecond<<'\t'<<a.weaponDegreesPerSecond<<'\t'<<name<<'\t'<<count<<'\t'<<wt.value(row,"Type")<<'\t'<<w->damage<<'\t'<<w->shotRange<<'\t'<<p->shotRange<<'\t'<<w->damageRange<<'\t'<<int(w->shotCount)<<'\t'<<w->shotDelay<<'\t'<<w->reloadDelay<<'\t'<<w->speed<<'\t'<<w->aimRate<<'\t'<<w->attackPoint()<<'\n';
 }}
}
