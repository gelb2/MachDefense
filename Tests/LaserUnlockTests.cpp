#include "SessionTestSupport.hpp"
#include "Core/Session/Settlement.hpp"
#include "../Modules/SaveTransfer/Validate.hpp"

struct TransferHost:Host {
    bool fail=false;std::string field;
    bool supportsSaveTransfer()const override{return true;}
    void openTransferField(std::string text,bool,Rect)override{field=std::move(text);}
    std::string transferFieldText()const override{return field;}
    void closeTransferField()override{field.clear();}
    bool writeSave(std::span<const uint8_t> bytes)override{return !fail&&Host::writeSave(bytes);}
};
bool owns(const SaveData& save,std::size_t slot,std::string_view name){const auto b=save.builds(slot);return std::any_of(b.begin(),b.end(),[&](const auto& x){return x.name==name;});}
SaveData fixture(unsigned completed,bool vulcan=true){
    auto s=SaveData::empty(8);s.createProfile("UNLOCK",1);s.createProfile("OTHER",2);
    s.writeWord(SaveData::slotStart+0x38,completed);s.writeWord(SaveData::slotStart+0x34,completed);
    s.writeWord(SaveData::slotStart+0x2054,123456);s.setCredits(0,87654);
    auto b=s.builds(0);b[0].level=7;b[0].onSlot=false;if(vulcan)b.push_back({"Vulcan",4,true});s.setBuilds(0,b);return s;
}
int main(int argc,char**argv){assert(argc>=2);Resources r;r.root=argv[1];auto table=[&](const char* n){return DataTable::load(r.root/"Tables"/(std::string(n)+".tsv"));};
    auto catalog=BuildCatalog::load(table("MachBuild_Parts"),table("MachBuild_Name"));Settlement policy(catalog,table("MachRegen"));
    for(unsigned n:{0u,29u,30u,39u,44u,79u,80u})for(bool v:{false,true}){
        auto s=fixture(n,v),before=s;const bool expected=n>=30&&v;
        assert(policy.grantCampaignUnlock(s,0,s.stage(0))==expected);assert(owns(s,0,"LaserTurret")==expected);
        auto after=s.encode();assert(!policy.grantCampaignUnlock(s,0,s.stage(0))&&s.encode()==after);
        if(!expected)assert(s.encode()==before.encode());else{
            auto inventory=s.builds(0);std::erase_if(inventory,[](const auto& x){return x.name=="LaserTurret";});
            assert(inventory.size()==before.builds(0).size());
            for(size_t i=0;i<inventory.size();++i){auto old=before.builds(0)[i];assert(inventory[i].name==old.name&&inventory[i].level==old.level&&inventory[i].onSlot==old.onSlot);}
            assert(s.credits(0)==before.credits(0)&&s.stage(0)==before.stage(0));
            // All bytes outside this slot's build inventory remain identical.
            const auto a=s.plaintext(),b=before.plaintext();for(size_t i=0;i<a.size();++i)if(i<SaveData::slotStart+0xb44||i>=SaveData::slotStart+0xb48+128*0x28)assert(a[i]==b[i]);
            auto restored=transfer::decode(transfer::encode(s));transfer::validate(restored,r);assert(restored.encode()==s.encode());
        }
    }
    // Mission 30 uses the existing result dialog reward, once, independent of RNG.
    for(unsigned seed=1;seed<=64;++seed){auto s=fixture(29);DarwinRandom random(seed);auto reward=policy.clear(s,0,random);assert(reward.build=="LaserTurret"&&s.stage(0)==30&&s.clearCount(0,29)==1);assert(s.credits(0)==87654+500+29*25);policy.clear(s,0,random);auto b=s.builds(0);assert(std::count_if(b.begin(),b.end(),[](const auto& x){return x.name=="LaserTurret";})==1);}
    // A late Vulcan unlock enables the laser without resetting the campaign.
    {auto s=fixture(39,false);assert(!policy.grantCampaignUnlock(s,0,39));auto b=s.builds(0);b.push_back({"Vulcan",0,false});s.setBuilds(0,b);assert(policy.grantCampaignUnlock(s,0,39));}
    // An already upgraded, uninstalled laser remains exactly as it was.
    {auto s=fixture(44);auto b=s.builds(0);b.push_back({"LaserTurret",9,false});s.setBuilds(0,b);auto bytes=s.encode();assert(!policy.grantCampaignUnlock(s,0,44)&&s.encode()==bytes);}
    // Existing installed limit/order is not reset when inventory is extended.
    {auto s=fixture(44);auto b=s.builds(0);b[0].onSlot=true;b.push_back({"ShotGun",2,true});b.push_back({"HMG",3,true});s.setBuilds(0,b);assert(policy.grantCampaignUnlock(s,0,44));b=s.builds(0);auto l=std::find_if(b.begin(),b.end(),[](const auto& x){return x.name=="LaserTurret";});assert(l!=b.end()&&!l->onSlot&&l->level==0);}
    // Real menu selection: persisted once, failed write rolls back, retry succeeds.
    {TransferHost h;h.saved=fixture(44).encode();auto original=h.saved;GameSession g(r,h,true);tap(g,160,240);ticks(g);tap(g,160,140);h.fail=true;tap(g,160,140);assert(h.saved==original&&h.writes==0&&!owns(*g.validationSave(),0,"LaserTurret"));tap(g,160,295);h.fail=false;tap(g,160,140);ticks(g);assert(h.writes==1&&owns(SaveData::decode(h.saved),0,"LaserTurret"));ticks(g,300);assert(h.writes==1);GameSession restart(r,h,true);assert(owns(*restart.validationSave(),0,"LaserTurret"));}
    // Restored saves use exactly the same selection/migration path.
    {TransferHost h;auto empty=SaveData::empty(1);empty.createProfile("OLD",1);h.saved=empty.encode();GameSession g(r,h,true);tap(g,160,240);ticks(g);tap(g,235,72);h.field=transfer::encode(fixture(44));tap(g,85,71);tap(g,112,295);tap(g,160,295);tap(g,160,140);tap(g,160,140);ticks(g);assert(owns(SaveData::decode(h.saved),0,"LaserTurret"));}
    // All ten actual laser assemblies produce, fire, damage a target and request
    // the existing effect sound. Resource decoding/hardware playback is separate.
    for(unsigned level=0;level<10;++level){auto s=fixture(44);s.setBuilds(0,std::vector<OwnedBuild>{{"LaserTurret",level,true}});Host host;World w(r,host,s,0);w.start(0);w.setValidationCells(10000);assert(w.enqueueBuild("LaserTurret",Vec2{0,0}));for(unsigned i=0;i<800;++i)w.tick();assert(w.statistics().produced==1);for(const auto& a:w.actors())if(a->player&&!a->postBuild.empty()){a->position=a->aircraft.position=a->destination;a->aircraft.state=0;}w.tick();WorldActor* gun=nullptr;for(const auto& a:w.actors())if(a->player&&a->name.starts_with("Tur_Lsr_"))gun=a.get();assert(gun&&!gun->weapons.empty());gun->position={0,0};const auto id=w.validationSpawn("Tur_Gun_01",false,{0,60});auto* enemy=w.actor(id);assert(enemy);enemy->weapons.clear();enemy->hp=enemy->maxHP=1000000;const int before=enemy->hp;for(unsigned i=0;i<240;++i)w.tick();assert(enemy->hp<before);const auto sound=level<3?"Plasma02.wav":"Plasma05.wav";assert(host.soundCalls[sound]>0);assert(!r.read("Audio/"+std::string(sound)).empty());Frame f;w.prepareDraw();w.appendSprites(f);checkImages(f,r);assert(host.writes==0);}
    if(argc==3){std::ifstream f(argv[2],std::ios::binary);Bytes bytes{std::istreambuf_iterator<char>(f),{}};auto s=SaveData::decode(bytes);unsigned grants=0;for(size_t i=0;i<SaveData::slotCount;++i)if(s.active(i))grants+=policy.grantCampaignUnlock(s,i,s.stage(i));auto copy=transfer::decode(transfer::encode(s));transfer::validate(copy,r);assert(copy.encode()==s.encode());std::cout<<"Local device save copy: "<<grants<<" grants, recovery roundtrip passed; source untouched\n";}
    std::cout<<"Laser unlock: boundaries, prerequisite, 64 rewards, existing save/order, menu write failure, recovery, ten levels firing/assets passed\n";
}
