#include "Core/Simulation/World.hpp"
#include "Core/Session/Settlement.hpp"
#include "../Modules/SaveTransfer/Validate.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
struct Resources:md::ResourceAccess {
 std::filesystem::path root;
 md::Bytes read(std::string_view path)override {std::ifstream f(root/path,std::ios::binary);if(!f)throw std::runtime_error(std::string(path));return {std::istreambuf_iterator<char>(f),{}};}
 md::Vec2 imageSize(std::string_view n)override {if(!std::filesystem::exists(root/"Images"/n))return {};auto b=read("Images/"+std::string(n));auto be=[&](int i){return unsigned(b.at(i))<<24|unsigned(b.at(i+1))<<16|unsigned(b.at(i+2))<<8|b.at(i+3);};return {float(be(16)),float(be(20))};}
 md::DataTable table(const char* n){return md::DataTable::load(root/"Tables"/(std::string(n)+".tsv"));}
};
struct Host:md::HostServices {unsigned writes=0;std::optional<md::Bytes>readSave()override{return{};}bool writeSave(std::span<const std::uint8_t>)override{++writes;return true;}void playSound(std::string_view,float,float)override{}void playMusic(std::string_view,float)override{}void stopMusic()override{}void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}};
std::string key(std::string_view n,unsigned l){return std::string(n)+(l<10?"_0":"_")+std::to_string(l);}
void samePose(const std::vector<md::PartPose>& a,const std::vector<md::PartPose>& b,Resources&r){
 assert(a.size()==b.size());
 for(size_t i=0;i<a.size();++i){auto& x=a[i];auto& y=b[i];assert(x.definition->image==y.definition->image);if(x.definition->name!="DummyParts" && x.definition->name!="DummyWeapon" && x.definition->tileCount){if(r.imageSize(x.definition->image).x<=0)std::cerr<<"missing image "<<x.definition->image<<" type "<<unsigned(x.definition->type)<<"\n";assert(r.imageSize(x.definition->image).x>0);}assert(x.definition->type==y.definition->type);assert(x.definition->splitX==y.definition->splitX&&x.definition->splitY==y.definition->splitY);assert(x.localCenter.x==y.localCenter.x&&x.localCenter.y==y.localCenter.y&&x.angle==y.angle&&x.visualScale==y.visualScale&&x.reverse==y.reverse);assert(x.definition->sockets.size()==y.definition->sockets.size());for(unsigned j=1;j<x.definition->sockets.size();++j){auto u=x.muzzle(j),v=y.muzzle(j);assert(u&&v&&u->x==v->x&&u->y==v->y);}}
}
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];Host host;
 auto builds=md::BuildCatalog::load(r.table("MachBuild_Parts"),r.table("MachBuild_Name"));md::Settlement settlement(builds,r.table("MachRegen"));md::BaseUpgradeCatalog bases(r.table("BaseUpgrade"));md::AttackCatalog attacks(r.table("SpAttackList"));auto models=md::BattleModels::load(r);auto machineTable=r.table("MachList"),weaponTable=r.table("WeaponInfo"),items=r.table("ItemInfo");
 auto original=md::BattleModels::parse(md::Chunk::parse(r.read("RawData/PartsInfo.mcb")),md::Chunk::parse(r.read("RawData/MachInfo.mcb")));original.configureTiles(r.table("PartsList"),r.table("TileList"));
 bool duplicate=false;try{models.append(md::Chunk::parse(r.read("RawData/Level10Parts.mcb")),md::Chunk::parse(r.read("RawData/Level10Machines.mcb")));}catch(const std::runtime_error&){duplicate=true;}assert(duplicate);
 const std::array names{"RifleTurret","MissileTurret","Vulcan","IonCannon","LaserTurret","PlasmaCannon"};
 const std::array special{"AirStrike-Bomb","AirStrike-Missile","AirStrike-Nuclear","AirStrike-EMP","Crossfire-Missile"};
 // Extra-level damage effects: hit the annulus newly reached by each level,
 // check exact exclusion at the boundary, and run the real status controller.
 for(bool phone:{true,false})for(unsigned level=6;level<=10;++level){
  auto load=[&](const char* prefix,unsigned l){auto n=std::string(prefix)+(l<10?"0":"")+std::to_string(l);return md::WeaponDefinition::fromTable(weaponTable,*weaponTable.findRow(n),phone);};
  auto missile=load("Turret_msl_",level),oldMissile=load("Turret_msl_",level-1);
  const int raw=74+4*int(level-5);assert(missile.damageRange==(phone?raw/2:raw));
  std::vector<md::ProjectileTarget> empty;
  const float middle=(missile.damageRange+oldMissile.damageRange)*.5f;
  std::vector<md::ProjectileTarget> targets={{{middle,0},100000,1},{{float(missile.damageRange),0},100000,1}};
  assert(md::projectileCollide(oldMissile,{0,0},false,0,targets,empty).empty());
  auto hits=md::projectileCollide(missile,{0,0},false,0,targets,empty);
  assert(hits.size()==1&&hits[0].damage>0&&targets[1].hp==100000);
  auto ion=load("Turret_ion_",level),oldIon=load("Turret_ion_",level-1);
  assert(ion.special1==280+20*int(level-5)&&ion.special2==3);
  const float radius=(oldIon.special1+ion.special1)*.5f;
  targets={{{0,0},100000,1},{{radius,0},100000,1},{{ion.special1,0},100000,1}};
  assert(md::projectileCollide(oldIon,{0,0},false,0,targets,empty).size()==1);
  hits=md::projectileCollide(ion,{0,0},false,0,targets,empty);
  assert(hits.size()==2&&hits[1].secondary&&hits[1].damage==ion.damage/2&&targets[2].hp==100000);
  auto plasma=load("Turret_pla_",level),oldPlasma=load("Turret_pla_",level-1);
  assert(plasma.special1==70+10*int(level-5)&&plasma.special2==120+20*int(level-5));
  auto effect=md::projectileSpecialDamage(plasma,0);assert(effect&&effect->endTime==plasma.special1&&effect->damage==plasma.special2&&effect->delay==1);
  auto total=[&](const md::WeaponDefinition& w){md::SpecialDamageController state;state.add(*md::projectileSpecialDamage(w,0));md::DarwinRandom random(1);int damage=0;bool later=false;for(unsigned tick=1;tick<=unsigned((w.special1+1)*60);++tick){const double time=double(tick)/60;state.tick(time,random,[](){return false;},[&](const auto& event){if(event.dealsDamage){assert(event.damage==int(w.special2));assert(time<=w.special1);damage+=event.damage;if(time>oldPlasma.special1)later=true;}});}assert(state.orderedEntries().empty());return std::pair{damage,later};};
  const auto now=total(plasma),before=total(oldPlasma);assert(now.first>before.first&&now.second);
 }
 // Extended ranges must continue the original final step. Exercise the real
 // firing boundary between consecutive levels, not only the table lookup.
 const std::array prefixes{"Turret_gun_","Turret_msl_","Turret_mg_","Turret_ion_","Turret_lsr_","Turret_pla_","BaseWpn"};
 const std::array caps{5u,5u,5u,5u,5u,5u,7u};
 const std::array finalRanges{240,270,380,390,240,240,540};
 const std::array increments{5,10,20,20,5,5,40};
 for(bool phone:{true,false})for(unsigned family=0;family<prefixes.size();++family){
  auto name=[&](unsigned l){return std::string(prefixes[family])+(l<10?"0":"")+std::to_string(l);};
  auto previous=md::WeaponDefinition::fromTable(weaponTable,*weaponTable.findRow(name(caps[family])),phone);
  for(unsigned l=caps[family]+1;l<=10;++l){
   const auto row=*weaponTable.findRow(name(l));
   assert(weaponTable.integer(row,"ShtRng")==finalRanges[family]+increments[family]*int(l-caps[family]));
   const auto current=md::WeaponDefinition::fromTable(weaponTable,row,phone);
   assert(current.shotRange>previous.shotRange&&current.damage>previous.damage&&current.attackPoint()>previous.attackPoint());
   md::WeaponFrame f;f.time=100;f.target=md::WeaponTarget{{(previous.shotRange+current.shotRange)*.5f,0},1000000};
   md::DarwinRandom random(1);md::WeaponController oldGun,newGun;
   assert(oldGun.tick(previous,f,random,[](const auto&){}).shots==0);
   assert(newGun.tick(current,f,random,[](const auto&){}).shots>0);
   f.target->position.x=current.shotRange;md::WeaponController boundary;
   assert(boundary.tick(current,f,random,[](const auto&){}).shots==0);
   previous=current;
  }
 }
 auto save=md::SaveData::empty(1);for(unsigned slot=0;slot<5;++slot)save.createProfile("LEVEL TEST",1+slot);
 for(unsigned slot=0;slot<5;++slot){
  save.setCredits(slot,2000000000);std::vector<md::OwnedBuild> owned;for(auto name:names)owned.push_back({name,0,true});save.setBuilds(slot,owned);
  std::vector<md::OwnedAttack> ownedA;for(auto name:special)ownedA.push_back({name,0,3});save.setAttacks(slot,ownedA);
  for(unsigned level=0;level<10;++level){
   for(auto name:names){auto list=save.builds(slot);auto found=std::find_if(list.begin(),list.end(),[&](auto& b){return b.name==name;});assert(found!=list.end()&&found->level==level);auto d=settlement.definition(*found);assert(d);if(level<9){auto old=save.credits(slot);assert(settlement.upgradeBuild(save,slot,name));assert(save.credits(slot)==old-d->upgradeCost);}else assert(!settlement.upgradeBuild(save,slot,name));}
   for(unsigned type=0;type<2;++type){assert(save.baseUpgrade(slot,type)==level);if(level<9){int price=bases.level(md::BaseUpgrade(type),level).cost;auto old=save.credits(slot);assert(bases.purchase(save,slot,md::BaseUpgrade(type)));assert(save.credits(slot)==old-price);}else assert(!bases.purchase(save,slot,md::BaseUpgrade(type)));}
   for(size_t i=0;i<special.size();++i){assert(attacks.maximumLevel(special[i])==10);auto owned=save.attacks(slot);assert(owned[i].level==level);auto price=attacks.upgradeCost(owned[i]);auto old=save.credits(slot);if(level<9){assert(attacks.purchase(save,slot,i,true));assert(save.credits(slot)==old-price);}else assert(!attacks.purchase(save,slot,i,true));}
  }
 }
 auto encoded=md::transfer::encode(save);auto decoded=md::transfer::decode(encoded);md::transfer::validate(decoded,r);assert(std::equal(decoded.plaintext().begin(),decoded.plaintext().end(),save.plaintext().begin(),save.plaintext().end()));
 auto extreme=save;auto owned=extreme.attacks(0);owned[0].level=4;owned[0].count=2147483647u;extreme.setAttacks(0,owned);extreme.setCredits(0,std::numeric_limits<int>::max());auto before=extreme.encode();assert(attacks.upgradeCost(owned[0])==std::int64_t(8000)+std::int64_t(2147483647)*100);assert(!attacks.purchase(extreme,0,0,true)&&before==extreme.encode());
 unsigned turretCases=0,baseCases=0,specialCases=0;
 for(auto g:{md::BattleGeometry::phone(),md::BattleGeometry::pad()}){
  md::World world(r,host,save,0,g,123);world.start(0);
  for(auto name:names)for(unsigned l=1;l<=10;++l){auto d=builds.find(key(name,l));assert(d&&d->cost>0);auto m=md::MachineDefinition::fromTable(machineTable,*machineTable.findRow(d->machine),g);assert(m.hp>0);auto id=world.validationSpawn(d->machine,true,{0,0});auto a=world.actor(id);assert(a&&a->hp==m.hp&&!a->weapons.empty());auto weapon=md::WeaponDefinition::fromTable(weaponTable,*weaponTable.findRow(a->weapons[0].name),g.deviceScale==.5f);assert(a->range==weapon.shotRange);if(l>5){auto prevBuild=builds.find(key(name,l-1));auto prevMachine=md::MachineDefinition::fromTable(machineTable,*machineTable.findRow(prevBuild->machine),g);assert(m.hp>prevMachine.hp);}auto poses=models.pose(d->machine,g.deviceScale,.3f,.7f,1.2f);auto old=builds.find(key(name,std::min(l,5u)));samePose(poses,original.pose(old->machine,g.deviceScale,.3f,.7f,1.2f),r);if(l>5)assert(a->weapons[0].name!=world.actor(world.validationSpawn(old->machine,true,{0,0}))->weapons[0].name);md::ProductionQueue queue;float cells=10000;assert(queue.enqueue(*d,0,1,cells,md::Vec2{1,2}));assert(cells==10000-d->cost);md::DarwinRandom rng(1);auto units=queue.tick(d->duration+1,g,rng);assert(units.size()==1&&units[0].definition.machine==d->machine);++turretCases;}
  for(unsigned c=0;c<10;++c)for(unsigned d=0;d<10;++d){save.setBaseUpgrade(0,0,c);save.setBaseUpgrade(0,1,d);auto loadout=md::PlayerLoadout::forBase(save,0,bases,g);auto oldSave=save;oldSave.setBaseUpgrade(0,0,std::min(c,6u));oldSave.setBaseUpgrade(0,1,std::min(d,6u));auto old=md::PlayerLoadout::forBase(oldSave,0,bases,g);samePose(models.pose(loadout.assembly,g.deviceScale,.3f,.7f,1.2f),original.pose(old.assembly,g.deviceScale,.3f,.7f,1.2f),r);assert(weaponTable.findRow(loadout.weapon));world.refreshBaseUpgrades();++baseCases;}
  for(size_t n=0;n<special.size();++n)for(unsigned l=0;l<10;++l){auto a=save.attacks(0);a[n].level=l;a[n].count=2;save.setAttacks(0,a);world.start(0);if(n==4){auto b=save.builds(0);b.push_back({"MachineGun",0,true});save.setBuilds(0,b);world.setValidationCells(1000);assert(world.enqueueBuild("MachineGun"));for(unsigned tick=0;tick<220;++tick)world.tick();bool unit=false;for(const auto& actor:world.actors())if(actor->player&&!actor->base&&actor->type==0){actor->position={0,-110};unit=true;}assert(unit);b.pop_back();save.setBuilds(0,b);}assert(world.activateSpecial(special[n],{0,0}));assert(save.attacks(0)[n].count==1);for(int tick=0;tick<240;++tick)world.tick();md::Frame f;world.appendSprites(f);for(auto& s:f.sprites)assert(r.imageSize(s.image).x>0);++specialCases;}
 }
 assert(host.writes==0);std::cout<<"Level10Data: 5-slot all-level purchases/MD1, overflow, "<<turretCases<<" turret production/asset cases, "<<baseCases<<" base assembly combinations, "<<specialCases<<" actual world special attacks passed\n";
}
