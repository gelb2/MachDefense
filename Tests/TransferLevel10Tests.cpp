#include "SessionTestSupport.hpp"
#include "../Modules/SaveTransfer/Validate.hpp"
#include "../Modules/SaveTransfer/FileStore.hpp"
#include <set>
struct RecoveryHost:Host {
 std::string field;bool opened=false;
 bool supportsSaveTransfer()const override{return true;}
 void openTransferField(std::string text,bool,Rect)override{field=text;opened=true;}
 std::string transferFieldText()const override{return field;}
 void closeTransferField()override{opened=false;field.clear();}
};
int main(int argc,char**argv){
 assert(argc>=2);Resources r;r.root=argv[1];
 auto table=[&](const char*n){return DataTable::load(r.root/"Tables"/(std::string(n)+".tsv"));};
 const auto parts=table("MachBuild_Parts"),names=table("MachBuild_Name"),weapons=table("WeaponInfo"),machines=table("MachList");
 const auto catalog=BuildCatalog::load(parts,names);Settlement settlement(catalog,table("MachRegen"));BaseUpgradeCatalog bases(table("BaseUpgrade"));AttackCatalog attacks(table("SpAttackList"));
 std::vector<std::string> units;std::set<std::string> seen;
 for(auto* t:{&parts,&names})for(size_t i=0;i<t->rowCount();++i){auto n=std::string(t->value(i,"MachName"));n.resize(n.rfind('_'));if(seen.insert(n).second)units.push_back(n);}
 const std::array specials{"AirStrike-Bomb","AirStrike-Missile","AirStrike-Nuclear","AirStrike-EMP","Crossfire-Missile"};
 auto seed=[&](unsigned level){auto s=SaveData::empty(773);for(unsigned slot=0;slot<5;++slot){s.createProfile("RECOVERY"+std::to_string(slot),slot+1);s.setCredits(slot,10000000-slot*12345);const auto off=SaveData::slotStart+slot*SaveData::slotStride;s.writeWord(off+0xc,5);s.writeWord(off+0x38,slot*17);for(unsigned m=0;m<80;++m)s.setClearCount(slot,m,m<=slot*17?1+slot:0);std::vector<OwnedBuild> b;for(unsigned i=0;i<units.size();++i)b.push_back({units[(i+slot)%units.size()],level,(i+slot)%3!=0});s.setBuilds(slot,b);std::vector<OwnedAttack>a;for(unsigned i=0;i<5;++i)a.push_back({specials[(i+slot)%5],level,7+slot*100+i,(i+slot)%2==0});s.setAttacks(slot,a);for(unsigned i=0;i<6;++i)s.setBaseUpgrade(slot,i,level);}return s;};
 unsigned restored=0,worlds=0,rejected=0;
 for(unsigned level=0;level<10;++level){auto expected=seed(level);const auto code=transfer::encode(expected);auto decoded=transfer::decode(code);transfer::validate(decoded,r);assert(decoded.encode()==expected.encode());
  RecoveryHost h;auto old=SaveData::empty(1);old.createProfile("KEEP",1);h.saved=old.encode();GameSession game(r,h,false);tap(game,160,240);ticks(game);tap(game,235,72);assert(h.opened);h.field=code;tap(game,85,71);assert(!h.opened);tap(game,112,295);assert(h.saved==expected.encode()&&h.writes==1);tap(game,160,295);tap(game,85,72);assert(h.opened&&h.field==code);tap(game,235,71);
  for(unsigned slot=0;slot<5;++slot){assert(decoded.credits(slot)==expected.credits(slot));for(auto g:{BattleGeometry::phone(),BattleGeometry::pad()}){Host worldHost;World world(r,worldHost,decoded,slot,g,123);world.start(0);auto load=PlayerLoadout::forBase(decoded,slot,bases,g);bool baseSeen=false;for(auto& a:world.actors())if(a->player&&a->base){assert(a->hp==load.hp&&a->weapons.front().name==load.weapon);baseSeen=true;}assert(baseSeen);
    for(const auto& b:decoded.builds(slot)){auto d=settlement.definition(b);assert(d);if(d->setType==0){auto partsLoad=PlayerLoadout::fromBuild(*d,table("ItemInfo"),g);assert(partsLoad.hp>0&&weapons.findRow(partsLoad.weapon));assert(!BattleModels::load(r).pose(partsLoad.assembly,g.deviceScale,0,0,0).empty());continue;}const auto id=world.validationSpawn(d->machine,true,{0,0});auto a=world.actor(id);assert(a&&!a->weapons.empty());const auto w=WeaponDefinition::fromTable(weapons,*weapons.findRow(a->weapons.front().name),g.deviceScale==.5f);assert(a->range==w.shotRange);if(a->type==1){auto m=MachineDefinition::fromTable(machines,*machines.findRow(d->machine),g);assert(a->hp==m.hp);}assert(w.damage>0);}
    if(slot==0){
     world.setValidationCells(100000);assert(world.enqueueBuild("MachineGun"));WorldActor* mech=nullptr;
     for(unsigned tick=0;tick<1200&&!mech;++tick){world.tick();for(auto& a:world.actors())if(a->player&&!a->base&&a->type==0)mech=a.get();}
     assert(mech);mech->position={0,-110};
     for(const auto& attack:decoded.attacks(slot)){assert(world.activateSpecial(attack.name,{0,0}));auto current=decoded.attacks(slot);auto it=std::find_if(current.begin(),current.end(),[&](auto& a){return a.name==attack.name;});assert(it!=current.end()&&it->count==attack.count-1);}
     for(unsigned tick=0;tick<240;++tick)world.tick();
    }
    Frame f;world.appendSprites(f);checkImages(f,r);assert(worldHost.writes==0);++worlds;
   }++restored;}
  // Reject an unavailable level in any slot without replacing any existing slot.
  for(unsigned slot=0;slot<5;++slot){auto reject=[&](const SaveData& bad){const auto before=h.saved;tap(game,235,72);h.field=transfer::encode(bad);tap(game,85,71);assert(has(game.frame(),"Common_Btn_OK.png"));assert(h.saved==before&&h.writes==1);tap(game,160,295);++rejected;};
   for(unsigned type=0;type<6;++type){auto bad=expected;bad.setBaseUpgrade(slot,type,10);reject(bad);}
   for(unsigned i=0;i<units.size();++i){auto bad=expected;auto b=bad.builds(slot);b[i].level=10;bad.setBuilds(slot,b);reject(bad);}
   for(unsigned i=0;i<5;++i){auto bad=expected;auto a=bad.attacks(slot);a[i].level=10;bad.setAttacks(slot,a);reject(bad);}
  }
 }
 // Mixed extended levels, order, flags and amounts: fixture for actual browser UI.
 auto mixed=seed(9);for(unsigned slot=0;slot<5;++slot){auto b=mixed.builds(slot);for(unsigned i=0;i<b.size();++i)b[i].level=5+(i+slot)%5;mixed.setBuilds(slot,b);auto a=mixed.attacks(slot);for(unsigned i=0;i<a.size();++i)a[i].level=5+(i+slot)%5;mixed.setAttacks(slot,a);for(unsigned i=0;i<6;++i)mixed.setBaseUpgrade(slot,i,5+(i+slot)%5);}
 transfer::validate(mixed,r);assert(transfer::decode(transfer::encode(mixed)).encode()==mixed.encode());
 if(argc>=3){const std::filesystem::path out=argv[2];std::filesystem::create_directories(out/"iphone");const auto bytes=mixed.encode();std::ofstream f(out/"iphone/MachDefense.sav",std::ios::binary);f.write((const char*)bytes.data(),bytes.size());std::ofstream(out/"iphone/recovery.txt")<<transfer::encode(mixed);}
 if(argc>=4){Resources old;old.root=argv[3];bool denied=false;try{transfer::validate(mixed,old);}catch(const std::exception&){denied=true;}assert(denied);std::cout<<"Old resource catalog rejects extended levels without clamping\n";}
 std::cout<<"TransferLevel10: "<<restored<<" slots restored, "<<worlds<<" phone/pad worlds, "<<rejected<<" unavailable-level imports rejected; 5-slot mixed fixture roundtrip passed\n";
}
