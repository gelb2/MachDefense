#include "SessionTestSupport.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];auto catalog=WeaponCatalog::load(DataTable::load(r.root/"Tables/WeaponInfo.tsv"),true);unsigned cases=0;
 for(const auto& spec:alliedAircraftCatalog)for(unsigned level:{1,10}){
  char suffix[8];snprintf(suffix,8,"_%02u",level);
  for(unsigned selected=0;selected<(spec.model=="AircraftHeavy"?5:spec.model=="AircraftTransport"?4:2);++selected){
   Host host;auto save=SaveData::empty(1);save.createProfile("EFFECT",1);World w(r,host,save,0);w.start(0);w.validationAlliedAircraftComparison();for(auto& a:w.actors())a->removed=true;
   auto aid=w.validationSpawn(std::string(spec.model)+suffix,true,{0,-50});auto* a=w.actor(aid);auto chosen=a->weapons.at(selected);a->weapons={chosen};
   auto tid=w.validationSpawn("Tur_Gun_01",false,{0,45});auto* t=w.actor(tid);t->weapons.clear();t->hp=t->maxHP=10000000;a->target=tid;
   auto* def=catalog.find(chosen.name);assert(def);bool muzzle=false,shell=false,trail=false,impact=false,recoil=false;
   for(int tick=0;tick<600;++tick){w.tick();w.prepareDraw();Frame f;w.appendSprites(f);checkImages(f,r);
    recoil|=a->weapons[0].recoil<0;
    for(auto& s:f.sprites){muzzle|=s.image=="Efx_Fire.png";shell|=s.image=="BulletShell.png";impact|=s.image==def->explodeImage;}
    for(auto& m:f.meshes)trail|=m.image==def->effectImage;
   }
   assert(muzzle&&recoil&&impact&&host.soundCalls[def->fireSound]>0&&host.writes==0);
   if(def->type==WeaponType::gun||def->type==WeaponType::sniper)assert(shell);
   else assert(!shell); // Original missile/cannon definition does not eject machinegun cases.
   if(def->effectType>=5&&def->effectType<=7)assert(trail);
   ++cases;
  }
 }
 std::cout<<cases<<" isolated weapons: original muzzle, shell, recoil, impact, missile trail and audio passed\n";
}
