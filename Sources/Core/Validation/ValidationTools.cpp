#include "Core/Validation/ValidationTools.hpp"
#include "Core/Simulation/World.hpp"
#include <algorithm>
#include <cstdio>
namespace md {
namespace {
DataTable table(ResourceAccess& r,std::string_view n){auto b=r.read("Tables/"+std::string(n)+".tsv");return DataTable::parse({reinterpret_cast<const char*>(b.data()),b.size()});}
std::string levelName(std::string_view n,unsigned level){char suffix[24];std::snprintf(suffix,sizeof suffix,"_%02u",level+1);return std::string(n)+suffix;}
unsigned id(ValidationCommand c){return unsigned(c);}
void shift(std::size_t& i,std::size_t count,int delta){if(count)i=(i+count+delta)%count;}
template<class T> auto owned(std::vector<T>& list,const std::string& name){return std::find_if(list.begin(),list.end(),[&](const auto& a){return a.name==name;});}
std::vector<ActorId> live(const World* w){std::vector<ActorId> result;if(w)for(const auto& a:w->actors())if(!a->removed&&a->hp>0)result.push_back(a->id);return result;}
}
ValidationTools::ValidationTools(ResourceAccess& r):builds_(BuildCatalog::load(table(r,"MachBuild_Parts"),table(r,"MachBuild_Name"))),bases_(table(r,"BaseUpgrade")),attacks_(table(r,"SpAttackList")){
    for(auto file:{"MachBuild_Parts","MachBuild_Name"}){auto source=table(r,file);for(const auto& row:source.rows())if(row[0].ends_with("_01"))buildNames_.push_back(row[0].substr(0,row[0].size()-3));}
    auto attackTable=table(r,"SpAttackList");for(const auto& row:attackTable.rows())if(row[0].ends_with("_01"))attackNames_.push_back(row[0].substr(0,row[0].size()-3));
    auto models=BattleModels::load(r);
    auto machineTable=table(r,"MachList");for(const auto& row:machineTable.rows())if(models.machine(row[0]))machineNames_.push_back(row[0]);
}
ValidationPanelState ValidationTools::panel(const SaveData& s,const World* w,bool open)const{
    ValidationPanelState p;p.enabled=true;p.open=open;p.page=page;p.pageCount=6;p.title="TEST CONTROLS";p.entryOrigin=w?Vec2{2,170}:Vec2{2,2};
    const bool active=s.active(slot_);const bool running=w&&w->state()==WorldState::running;
    auto add=[&](std::string label,std::string value,ValidationCommand minus={},ValidationCommand plus={},ValidationCommand action={},bool enabled=true,std::string actionLabel="SET"){
        p.rows.push_back({std::move(label),std::move(value),id(minus),id(plus),id(action),enabled,std::move(actionLabel)});};
    if(page==0){
        add("PROFILE",std::to_string(slot_+1)+" "+(active?s.name(slot_):"EMPTY"),ValidationCommand::profilePrevious,ValidationCommand::profileNext,{},!w);
        bool freeSlot=false;for(std::size_t i=0;i<SaveData::slotCount;++i)freeSlot|=!s.active(i);
        add("NEW PROFILE",freeSlot?"":"SLOTS FULL",{},{},ValidationCommand::newProfile,!w&&freeSlot,"CREATE");
        add("MISSION",std::to_string(mission_+1)+" / 80",ValidationCommand::missionPrevious,ValidationCommand::missionNext,ValidationCommand::startMission,active,"START");
        add("CREDITS",active?std::to_string(s.credits(slot_)):"-",ValidationCommand::creditMinus,ValidationCommand::creditPlus,{},active);
        add("CREDIT STEP",std::to_string(creditStep_),{},{},ValidationCommand::creditStep,true,"CHANGE");
        add("CELLS",w?std::to_string(int(w->cells()))+" / "+std::to_string(int(w->cellCapacity())):"-",ValidationCommand::cellsMinus,ValidationCommand::cellsPlus,ValidationCommand::fillCells,running,"FILL");
        add("SAVE TEST SLOT","",{},{},ValidationCommand::save,active,"SAVE");
    }else if(page==1){
        const char* names[]={"BASE CANNON","BASE DEFENSE","BUILD TIME","CREDIT RESEARCH","CELL RESEARCH","CELL STORAGE"};
        for(unsigned i=0;i<6;++i)add(names[i],active?std::to_string(s.baseUpgrade(slot_,i)+1)+" / "+std::to_string(bases_.maximumLevel(BaseUpgrade(i))+1):"-",ValidationCommand(100+i),ValidationCommand(110+i),{},active);
    }else if(page==2){
        const auto& name=buildNames_.at(build_);auto list=s.builds(slot_);auto found=owned(list,name);const bool has=found!=list.end();
        add("EQUIPMENT",name,ValidationCommand::buildPrevious,ValidationCommand::buildNext);
        add("OWNED",has?"YES":"NO",{},{},ValidationCommand::buildGrant,active&&!has,"ADD");
        add("LEVEL",has?std::to_string(found->level+1):"-",ValidationCommand::buildLevelMinus,ValidationCommand::buildLevelPlus,{},active&&has);
        add("EQUIPPED",has&&found->onSlot?"YES":"NO",{},{},ValidationCommand::buildInstall,active&&has,"TOGGLE");
    }else if(page==3){
        const auto& name=attackNames_.at(attack_);auto list=s.attacks(slot_);auto found=owned(list,name);const bool has=found!=list.end();
        add("SPECIAL ATTACK",name,ValidationCommand::attackPrevious,ValidationCommand::attackNext);
        add("OWNED",has?"YES":"NO",{},{},ValidationCommand::attackGrant,active&&!has,"ADD");
        add("LEVEL",has?std::to_string(found->level+1):"-",ValidationCommand::attackLevelMinus,ValidationCommand::attackLevelPlus,{},active&&has);
        add("STOCK",has?std::to_string(found->count):"-",ValidationCommand::attackCountMinus,ValidationCommand::attackCountPlus,{},active&&has);
        add("EQUIPPED",has&&found->onSlot?"YES":"NO",{},{},ValidationCommand::attackInstall,active&&has,"TOGGLE");
    }else if(page==4){
        auto actors=live(w);const auto* a=actors.empty()?nullptr:w->actor(actors[actor_%actors.size()]);
        add("SPAWN UNIT",machineNames_.at(machine_),ValidationCommand::machinePrevious,ValidationCommand::machineNext,ValidationCommand::spawnMachine,running,"SPAWN");
        add("SPAWN SIDE",playerSpawn_?"PLAYER":"ENEMY",{},{},ValidationCommand::spawnSide,running,"CHANGE");
        add("ACTOR",a?std::to_string(a->id)+" "+a->name:"-",ValidationCommand::actorPrevious,ValidationCommand::actorNext,{},running&&a);
        add("HEALTH",a?std::to_string(a->hp)+" / "+std::to_string(a->maxHP):"-",ValidationCommand::healthMinus,ValidationCommand::healthPlus,ValidationCommand::healthFull,running&&a,"FULL");
        add("ENEMY BASE","",{},{},ValidationCommand::win,running,"DESTROY");
        add("PLAYER BASE","",{},{},ValidationCommand::lose,running,"DESTROY");
    }
    if(page==5){
        add("LIGHT HELI","LEVEL 1 / ALLIED",{},{},ValidationCommand::aircraftLight,running,"SPAWN");
        add("MEDIUM HELI","LEVEL 1 / ALLIED",{},{},ValidationCommand::aircraftHeavy,running,"SPAWN");
        add("DUCT VTOL","LEVEL 1 / ALLIED",{},{},ValidationCommand::aircraftDuct,running,"SPAWN");
        add("CARGO VTOL","LEVEL 1 / ALLIED",{},{},ValidationCommand::aircraftTransport,running,"SPAWN");
        add("TRIAL LEVEL 1","FREE TEST SPAWN");
    }
    return p;
}
ValidationResult ValidationTools::apply(unsigned raw,SaveData& s,World* w){
    ValidationResult result;const auto c=ValidationCommand(raw);const bool active=s.active(slot_);const bool running=w&&w->state()==WorldState::running;
    if(raw==validationPreviousPageCommand){page=(page+5)%6;return result;}
    if(raw==validationNextPageCommand){page=(page+1)%6;return result;}
    switch(c){
    case ValidationCommand::profilePrevious:if(!w)shift(slot_,5,-1);return result;
    case ValidationCommand::profileNext:if(!w)shift(slot_,5,1);return result;
    case ValidationCommand::newProfile:if(!w){if(auto i=s.createProfile("TEST",1)){slot_=*i;result.changed=true;}}return result;
    case ValidationCommand::missionPrevious:mission_=(mission_+79)%80;return result;
    case ValidationCommand::missionNext:mission_=(mission_+1)%80;return result;
    case ValidationCommand::startMission:result.start=active;return result;
    case ValidationCommand::creditStep:creditStep_=creditStep_==1?100:creditStep_==100?1000:creditStep_==1000?100000:1;return result;
    case ValidationCommand::buildPrevious:shift(build_,buildNames_.size(),-1);return result;
    case ValidationCommand::buildNext:shift(build_,buildNames_.size(),1);return result;
    case ValidationCommand::attackPrevious:shift(attack_,attackNames_.size(),-1);return result;
    case ValidationCommand::attackNext:shift(attack_,attackNames_.size(),1);return result;
    case ValidationCommand::machinePrevious:shift(machine_,machineNames_.size(),-1);return result;
    case ValidationCommand::machineNext:shift(machine_,machineNames_.size(),1);return result;
    case ValidationCommand::spawnSide:playerSpawn_=!playerSpawn_;return result;
    default:break;
    }
    if(!active)return result;
    if(c==ValidationCommand::save){result.persist=true;return result;}
    if(c==ValidationCommand::creditMinus||c==ValidationCommand::creditPlus){auto value=std::int64_t(s.credits(slot_))+(c==ValidationCommand::creditPlus?1:-1)*std::int64_t(creditStep_);s.setCredits(slot_,int(std::clamp<std::int64_t>(value,0,1000000000)));result.changed=true;}
    if((raw>=100&&raw<106)||(raw>=110&&raw<116)){const unsigned i=raw%10;auto value=s.baseUpgrade(slot_,i);value=raw>=110?std::min(value+1,bases_.maximumLevel(BaseUpgrade(i))):(value?value-1:0);s.setBaseUpgrade(slot_,i,value);if(w)w->refreshBaseUpgrades();result.changed=true;}
    if(raw>=id(ValidationCommand::buildLevelMinus)&&raw<=id(ValidationCommand::buildInstall)){
        auto list=s.builds(slot_);const auto& name=buildNames_.at(build_);auto i=owned(list,name);
        if(c==ValidationCommand::buildGrant&&i==list.end()){list.push_back({name,0,false});i=list.end()-1;}
        if(i!=list.end()){
            if(c==ValidationCommand::buildLevelMinus&&i->level)--i->level;
            if(c==ValidationCommand::buildLevelPlus&&builds_.find(levelName(name,i->level+1)))++i->level;
            if(c==ValidationCommand::buildInstall)i->onSlot=!i->onSlot;
            s.setBuilds(slot_,list);result.changed=true;
        }
    }
    if(raw>=id(ValidationCommand::attackLevelMinus)&&raw<=id(ValidationCommand::attackInstall)){
        auto list=s.attacks(slot_);const auto& name=attackNames_.at(attack_);auto i=owned(list,name);
        if(c==ValidationCommand::attackGrant&&i==list.end()){list.push_back({name,0,1,false});i=list.end()-1;}
        if(i!=list.end()){
            if(c==ValidationCommand::attackLevelMinus&&i->level)--i->level;
            if(c==ValidationCommand::attackLevelPlus&&i->level+1<attacks_.maximumLevel(name))++i->level;
            if(c==ValidationCommand::attackCountMinus&&i->count)--i->count;
            if(c==ValidationCommand::attackCountPlus&&i->count<100000)++i->count;
            if(c==ValidationCommand::attackInstall&&(i->onSlot||std::count_if(list.begin(),list.end(),[](const auto& a){return a.onSlot;})<7))i->onSlot=!i->onSlot;
            s.setAttacks(slot_,list);result.changed=true;
        }
    }
    if(!running)return result;
    if(c==ValidationCommand::cellsMinus||c==ValidationCommand::cellsPlus||c==ValidationCommand::fillCells){w->setValidationCells(c==ValidationCommand::fillCells?w->cellCapacity():std::max(0.f,w->cells()+(c==ValidationCommand::cellsPlus?100:-100)));result.changed=true;}
    if(raw>=600&&raw<604){w->validationAlliedAircraft(raw-600,w->screenToWorld({160,240}));result.changed=true;}
    if(c==ValidationCommand::spawnMachine){w->validationSpawn(machineNames_.at(machine_),playerSpawn_,w->screenToWorld({160,240}));result.changed=true;}
    auto actors=live(w);
    if(c==ValidationCommand::actorPrevious)shift(actor_,actors.size(),-1);
    if(c==ValidationCommand::actorNext)shift(actor_,actors.size(),1);
    if(!actors.empty()&&(c==ValidationCommand::healthMinus||c==ValidationCommand::healthPlus||c==ValidationCommand::healthFull)){auto* a=w->actor(actors[actor_%actors.size()]);const int step=std::max(1,a->maxHP/10);const int hp=c==ValidationCommand::healthFull?a->maxHP:std::clamp(a->hp+(c==ValidationCommand::healthPlus?step:-step),1,a->maxHP);w->setValidationHealth(a->id,hp);result.changed=true;}
    if(c==ValidationCommand::win||c==ValidationCommand::lose){for(const auto& a:w->actors())if(a->base&&a->player==(c==ValidationCommand::lose)){const auto target=a->id;w->damage(target,a->hp);result.changed=true;break;}}
    return result;
}
}
