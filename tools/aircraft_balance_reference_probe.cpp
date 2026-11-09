#include "Core/Contracts/Game.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include "Core/Simulation/PlayerParts.hpp"
#include "Core/Simulation/Combat.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <iomanip>
#include "Core/Simulation/WeaponController.hpp"
struct Resources:md::ResourceAccess{
 std::filesystem::path root;
 md::Bytes read(std::string_view p)override{std::ifstream f(root/p,std::ios::binary);if(!f)throw std::runtime_error(std::string(p));return {std::istreambuf_iterator<char>(f),{}};}
 md::Vec2 imageSize(std::string_view)override{return {};}
};
double shotsPerSecond(const md::WeaponDefinition& w){
 md::WeaponController c; md::DarwinRandom random; md::WeaponFrame f;
 f.manualFire=true; f.deviceScale=1; unsigned count=0;
 // Warm up 60 s, then count emissions for 600 s at the actual .0166f step.
 constexpr double dt=double(.0166f); const int warm=int(60/dt),steps=int(600/dt);
 for(int i=1;i<=warm+steps;++i){f.time=i*dt;c.tick(w,f,random,[&](const md::WeaponFire&){if(i>warm)++count;});}
 return count/(steps*dt);
}
int main(int argc,char**argv){if(argc!=3)return 2;Resources r;r.root=argv[1];auto t=[&](const char*n){return md::DataTable::load(r.root/"Tables"/(std::string(n)+".tsv"));};
 auto models=md::BattleModels::load(r);auto machine=t("MachList"),wt=t("WeaponInfo");auto weapons=md::WeaponCatalog::load(wt,false),phone=md::WeaponCatalog::load(wt,true);auto builds=md::BuildCatalog::load(t("MachBuild_Parts"),t("MachBuild_Name"));

 std::ofstream rates(argv[2]);rates<<std::setprecision(12)<<"unit\tlevel\tweapon\tcount\tdamage\tshotsPerSecond\tidealDps\trawRange\n";
 auto emitRates=[&](const std::string& unit,unsigned level,const std::vector<md::AssemblyPart>& assembly){
  std::map<std::string,int> counts;auto walk=[&](auto&& self,const auto& ns)->void{for(const auto& n:ns){if(weapons.find(n.name))++counts[n.name];self(self,n.children);}};walk(walk,assembly);
  for(auto [name,count]:counts){const auto& w=*weapons.find(name);auto rate=shotsPerSecond(w);auto row=*wt.findRow(name);rates<<unit<<'\t'<<level<<'\t'<<name<<'\t'<<count<<'\t'<<w.damage<<'\t'<<rate<<'\t'<<rate*w.damage*count<<'\t'<<wt.value(row,"ShtRng")<<'\n';}
 };
 std::cout<<"reference\tlevel\thp\tcells\tupgradeCredits\tbuildSeconds\trange\tphoneRange\thudAttack\n";
 for(const char* name:{"MachineGun","Vulcan"})for(unsigned level=1;level<=10;++level){
 char suffix[8];snprintf(suffix,sizeof suffix,"_%02u",level);auto d=builds.find(std::string(name)+suffix);int hp=0,attack=0,range=0,prange=0;std::vector<md::AssemblyPart> assembly;
 if(d->setType==0){auto p=md::PlayerLoadout::fromBuild(*d,t("ItemInfo"),md::BattleGeometry::pad());hp=p.hp;assembly=p.assembly;}
 else{auto m=md::MachineDefinition::fromTable(machine,*machine.findRow(d->machine),md::BattleGeometry::pad());hp=m.hp;assembly=*models.machine(d->machine);}
 auto walk=[&](auto&& self,const auto& ns)->void{for(const auto& n:ns){if(auto w=weapons.find(n.name)){attack+=w->attackPoint();range=std::max(range,int(w->shotRange));prange=std::max(prange,int(phone.find(n.name)->shotRange));}self(self,n.children);}};walk(walk,assembly);
 emitRates(name,level,assembly);
 std::cout<<name<<'\t'<<level<<'\t'<<hp<<'\t'<<d->cost<<'\t'<<d->upgradeCost<<'\t'<<d->duration<<'\t'<<range<<'\t'<<prange<<'\t'<<attack<<'\n';
 }
 for(const auto& a:md::alliedAircraftCatalog)for(unsigned level=1;level<=10;level++){
 char suffix[8];snprintf(suffix,sizeof suffix,"_%02u",level);auto d=builds.find(std::string(a.name)+suffix);emitRates(std::string(a.label),level,*models.machine(d->machine));
 }
}
