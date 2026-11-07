#include "SessionTestSupport.hpp"
#include "Core/Session/Settlement.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include "Core/Simulation/PlayerParts.hpp"
#include "../Modules/SaveTransfer/Validate.hpp"
#include <iomanip>
int main(int argc,char** argv){assert(argc>=2);Resources r;r.root=argv[1];
 auto table=[&](const char* n){return DataTable::load(r.root/"Tables"/(std::string(n)+".tsv"));};
 auto builds=BuildCatalog::load(table("MachBuild_Parts"),table("MachBuild_Name"));Settlement policy(builds,table("MachRegen"));auto weapons=WeaponCatalog::load(table("WeaponInfo"),true);auto models=BattleModels::load(r);
 auto fixture=[](){auto s=SaveData::empty(29);s.createProfile("AIR",1);s.writeWord(SaveData::slotStart+0xc,5);s.setCredits(0,10000000);return s;};
 for(const auto& spec:alliedAircraftCatalog){
  auto s=fixture();auto first=policy.grantAircraftUnlocks(s,0,spec.mission-1);auto second=policy.grantAircraftUnlocks(s,0,spec.mission);assert(std::find(second.begin(),second.end(),spec.name)!=second.end());assert(policy.grantAircraftUnlocks(s,0,spec.mission).empty());
  s=fixture();policy.grantAircraftUnlocks(s,0,spec.mission-1);s.writeWord(SaveData::slotStart+0x34,spec.mission-1);DarwinRandom rng(7);auto reward=policy.clear(s,0,rng);assert(reward.build==spec.name&&s.clearCount(0,spec.mission-1)==1);
  Host h;h.saved=s.encode();GameSession game(r,h,true);tap(game,160,240);ticks(game);tap(game,160,140);ticks(game);tap(game,240,450);ticks(game);assert(game.validationWorld());assert(!has(game.frame(),"Common_Btn_OK.png")); // No new mission-entry acquisition dialog.
  // Drive the real session through clear -> original reward dialog, including its aircraft icon.
  {auto fresh=fixture();fresh.writeWord(SaveData::slotStart+0x34,spec.mission-1);fresh.writeWord(SaveData::slotStart+0x38,spec.mission-1);Host host;host.saved=fresh.encode();GameSession session(r,host,true);tap(session,160,240);ticks(session);tap(session,160,140);ticks(session);tap(session,240,450);ticks(session);auto* world=session.validationWorld();assert(world&&world->stage()==spec.mission-1);for(auto& a:world->actors())if(a->base&&!a->player){world->damage(a->id,a->hp);break;}session.tick();assert(world->state()==WorldState::cleared);auto f=session.frame();assert(has(f,"CommonUI_StageClear.png")&&has(f,"SummaryBtn_NextStage.png"));const char* body=spec.model=="AircraftLight"?"ALightBody.png":spec.model=="AircraftHeavy"?"AHeavyBody.png":spec.model=="AircraftDuct"?"ADuctBody.png":"ATransportBody.png";assert(has(f,body));checkImages(f,r);}
  int previousHP=0,previousRange=0;for(unsigned level=1;level<=10;++level){
   auto save=fixture();save.setBuilds(0,std::vector<OwnedBuild>{{std::string(spec.name),level-1,true}});Host host;World world(r,host,save,0);world.start(0);world.setValidationCells(100000);
   assert(world.enqueueBuild(spec.name));for(int tick=0;tick<1500&&world.statistics().produced==0;++tick)world.tick();assert(world.statistics().produced==1);
   WorldActor* air=nullptr;for(const auto& a:world.actors())if(a->name.starts_with(spec.model))air=a.get();assert(air&&air->flightTactics&&air->type==2&&air->hp>previousHP);previousHP=air->maxHP;assert(air->range>=previousRange);previousRange=int(air->range);
   air->position=air->aircraft.position={0,0};const auto id=world.validationSpawn("Tur_Gun_01",false,{0,60});auto* target=world.actor(id);target->weapons.clear();target->hp=target->maxHP=1000000;target->enemyAI.stopDelay=1e9;air->target=id;
   for(int tick=0;tick<300;++tick){const float old=air->angle;world.tick();assert(std::abs(std::remainder(air->angle-old,6.28318530718f))<=air->flightTactics->noseStep+.00001f);}
   assert(target->hp<1000000&&host.sounds>0&&host.writes==0);Frame f;world.prepareDraw();world.appendSprites(f);checkImages(f,r);
   auto decoded=transfer::decode(transfer::encode(save));transfer::validate(decoded,r);assert(decoded.encode()==save.encode());
   if(level<10){auto prior=save.credits(0);char key[48];std::snprintf(key,sizeof key,"%s_%02u",std::string(spec.name).c_str(),level);assert(policy.upgradeBuild(save,0,spec.name));assert(save.builds(0)[0].level==level&&save.credits(0)==prior-builds.find(key)->upgradeCost);}else assert(!policy.upgradeBuild(save,0,spec.name));
  }
 }
 // Largest effective range among every production assembly; total multiweapon attack estimate.
 for(unsigned level=1;level<=10;++level){int maxRange=0;std::array<int,4> attack{};char suffix[8];std::snprintf(suffix,sizeof suffix,"_%02u",level);
  auto visit=[&](auto&& self,const auto& nodes,int& atk,int& rng)->void{for(auto& p:nodes){if(auto w=weapons.find(p.name)){atk+=w->attackPoint();rng=std::max(rng,int(w->shotRange));}self(self,p.children,atk,rng);}};
  for(auto file:{"MachBuild_Parts","MachBuild_Name"}){auto t=table(file);for(auto& row:t.rows())if(row[0].ends_with(suffix)){auto d=builds.find(row[0]);if(alliedAircraftForModel(d->machine))continue;int a=0,b=0;if(d->setType==0){auto p=PlayerLoadout::fromBuild(*d,table("ItemInfo"),BattleGeometry::phone());b=weapons.find(p.weapon)->shotRange;}else visit(visit,*models.machine(d->machine),a,b);maxRange=std::max(maxRange,b);}}
  for(unsigned k=0;k<4;++k){auto& spec=alliedAircraftCatalog[k];int range=0;visit(visit,*models.machine(std::string(spec.model)+suffix),attack[k],range);if(k==1)assert(range==maxRange);if(k==0){auto d=builds.find(std::string("MachineGun")+suffix);auto p=PlayerLoadout::fromBuild(*d,table("ItemInfo"),BattleGeometry::phone());assert(range==weapons.find(p.weapon)->shotRange);}}
  assert(attack[1]>attack[0]&&attack[1]>attack[2]&&attack[1]>attack[3]);
 }
 // Existing advanced profile: all grants once, no level/money reset; recovery remains byte-identical.
 auto s=fixture();s.setBuilds(0,std::vector<OwnedBuild>{{"MachineGun",9,true},{"RifleTurret",7,false}});auto credits=s.credits(0);assert(policy.grantAircraftUnlocks(s,0,44).size()==4);auto bytes=s.encode();assert(policy.grantAircraftUnlocks(s,0,44).empty()&&s.encode()==bytes&&s.credits(0)==credits);transfer::validate(transfer::decode(transfer::encode(s)),r);
 if(argc==3){std::ifstream f(argv[2],std::ios::binary);Bytes b{std::istreambuf_iterator<char>(f),{}};auto local=SaveData::decode(b);for(size_t slot=0;slot<5;++slot)if(local.active(slot))policy.grantAircraftUnlocks(local,slot,local.stage(slot));transfer::validate(local,r);assert(transfer::decode(transfer::encode(local)).encode()==local.encode());std::cout<<"Local iPhone save copy + aircraft + recovery validated; original not modified\n";}
 std::cout<<"AircraftCampaign: 4 mission boundaries, original clear rewards, 40 production/fire/heading/upgrade/recovery cases, longest ranges and strongest gunship passed\n";
}
