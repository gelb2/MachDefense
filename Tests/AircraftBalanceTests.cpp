#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include "Core/Simulation/PlayerParts.hpp"
#include "Core/Session/Settlement.hpp"
#include <set>
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];auto table=[&](const char*n){return DataTable::load(r.root/"Tables"/(std::string(n)+".tsv"));};
 auto builds=BuildCatalog::load(table("MachBuild_Parts"),table("MachBuild_Name"));auto models=BattleModels::load(r);auto items=table("ItemInfo"),machines=table("MachList"),wt=table("WeaponInfo");unsigned cases=0;
 auto pad=WeaponCatalog::load(wt,false);
 constexpr int cheyenneHP[]{850,936,1062,1190,1360,1530,1700,1956,2210,2550};
 constexpr int belugaHP[]{1500,1750,2026,2300,2700,3300,3800,4450,5200,6100};
 constexpr int cheyenneCost[]{216,258,300,342,390,450,510,582,660,750};
 constexpr int belugaCost[]{378,444,510,576,645,720,795,876,960,1050};
 constexpr int cheyenneUpgrade[]{420,900,1800,3600,7200,14400,28800,57600,115200,0};
 constexpr int belugaUpgrade[]{660,1350,2700,5400,10800,21600,43200,86400,172800,0};
 for(bool phone:{false,true}){auto catalog=WeaponCatalog::load(wt,phone);auto g=phone?BattleGeometry::phone():BattleGeometry::pad();
  for(unsigned level=1;level<=10;++level){char suffix[8];snprintf(suffix,8,"_%02u",level);
   auto reference=[&](const char* name){auto d=builds.find(std::string(name)+suffix);assert(d);std::vector<AssemblyPart> parts;
    if(d->setType==0)parts=PlayerLoadout::fromBuild(*d,items,g).assembly;else parts=*models.machine(d->machine);
    std::string weapon;auto visit=[&](auto&& self,const auto& nodes)->void{for(auto& p:nodes){if(catalog.find(p.name))weapon=p.name;self(self,p.children);}};visit(visit,parts);assert(!weapon.empty());return weapon;
   };
   const auto mg=builds.find(std::string("MachineGun")+suffix),vu=builds.find(std::string("Vulcan")+suffix);
   const auto vhp=MachineDefinition::fromTable(machines,*machines.findRow(vu->machine),g).hp;
   for(const auto& spec:alliedAircraftCatalog){const auto d=builds.find(std::string(spec.name)+suffix);assert(d);const auto actual=MachineDefinition::fromTable(machines,*machines.findRow(d->machine),g);
    assert(level<10?d->upgradeCost>0:d->upgradeCost==0);assert(!builds.find(std::string(spec.name)+"_11"));
    assert(d->cost>=std::lround(mg->cost*1.2)&&d->cost<=std::lround(vu->cost*1.2));
    int expected=0;
    if(spec.model=="AircraftLight"){expected=catalog.find(reference("MachineGun"))->shotRange;assert(d->cost==std::lround(mg->cost*1.2));}
    else if(spec.model=="AircraftHeavy"){expected=catalog.find(reference("Vulcan"))->shotRange;assert(d->cost==std::lround(vu->cost*1.2));assert(actual.hp==std::lround(vhp*.9));}
    else if(spec.model=="AircraftTransport")expected=catalog.find(reference("PlasmaGun"))->shotRange;
    else{
     const double desired=pad.find(reference("ShotGun"))->shotRange*1.1;int best=1;double error=1e9;
     for(int raw=1;raw<1000;++raw){double diff=std::abs(int(raw*1.5)-desired);if(diff<error){best=int(raw*1.5);error=diff;}}
     expected=phone?int(best*.5):best;assert(error<=1.0);
    }
    if(spec.model=="AircraftDuct"||spec.model=="AircraftTransport"){
     const bool cheyenne=spec.model=="AircraftDuct";const auto i=level-1;
     assert(actual.hp==(cheyenne?cheyenneHP[i]:belugaHP[i]));
     assert(d->cost==(cheyenne?cheyenneCost[i]:belugaCost[i]));
     assert(d->upgradeCost==(cheyenne?cheyenneUpgrade[i]:belugaUpgrade[i]));
     const auto plasma=builds.find(std::string("PlasmaGun")+suffix);
     const int plasmaHP=PlayerLoadout::fromBuild(*plasma,items,g).hp;
     const auto tanker=builds.find(std::string("Tanker")+suffix);
     const int tankerHP=MachineDefinition::fromTable(machines,*machines.findRow(tanker->machine),g).hp;
     if(cheyenne)assert(std::abs(actual.hp*100-plasmaHP*85)<=100);
     else {assert(plasmaHP<actual.hp&&actual.hp<tankerHP);assert(std::abs(actual.hp*2-(plasmaHP+tankerHP))<=2);}
     ProductionQueue queue;float cells=float(d->cost-1);assert(!queue.enqueue(*d,0,1,cells));assert(cells==d->cost-1);
     cells=float(d->cost+7);assert(queue.enqueue(*d,0,1,cells));assert(cells==7);
     auto save=SaveData::empty(1);save.createProfile("BALANCE",1);save.setBuilds(0,std::array<OwnedBuild,1>{{{std::string(spec.name),i,true}}});
     Settlement settlement(builds,table("MachRegen"));
     if(level<10){save.setCredits(0,d->upgradeCost-1);assert(!settlement.upgradeBuild(save,0,spec.name));assert(save.builds(0)[0].level==i);
      save.setCredits(0,d->upgradeCost+7);assert(settlement.upgradeBuild(save,0,spec.name));assert(save.credits(0)==7&&save.builds(0)[0].level==level);}
     else {save.setCredits(0,1000000);assert(!settlement.upgradeBuild(save,0,spec.name));assert(save.credits(0)==1000000);}
    }
    unsigned count=0;std::set<std::string> names;auto visit=[&](auto&&self,const auto& nodes)->void{for(auto& p:nodes){if(auto w=catalog.find(p.name)){++count;names.insert(p.name);assert(w->shotRange==expected&&w->damage>0&&w->shotCount>0&&w->reloadDelay>0);assert(!w->fireSound.empty());}self(self,p.children);}};visit(visit,*models.machine(d->machine));
    assert(count==(spec.model=="AircraftHeavy"?5:spec.model=="AircraftTransport"?4:2));
    if(spec.model=="AircraftHeavy")assert(names.size()==5);
    ++cases;
   }
  }
 }
 std::cout<<cases<<" phone/pad level cases: range references, quantization, cost ratios, AH-10 HP, all mounts, positive weapon values, level-10 cap passed\n";
}
