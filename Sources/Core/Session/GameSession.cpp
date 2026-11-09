#include "Core/Session/GameSession.hpp"
#include "Core/Presentation/Help.hpp"
#include "Core/Presentation/SupportDialog.hpp"
#include <limits>
#include "Core/Presentation/Menu.hpp"
#include "Core/Presentation/SummaryDialog.hpp"
#include "Core/Presentation/Transition.hpp"
#include "Core/Presentation/UpgradeDialog.hpp"
#include "Core/Simulation/World.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include "Core/Validation/ValidationTools.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <stdexcept>
namespace md {
namespace {
DataTable table(ResourceAccess& r,std::string_view n){auto b=r.read("Tables/"+std::string(n)+".tsv");return DataTable::parse({reinterpret_cast<const char*>(b.data()),b.size()});}
std::string fontMetrics(ResourceAccess& r){auto b=r.read("Generated/TrebuchetMS-Bold-48.tsv");return {b.begin(),b.end()};}
SaveData loadSave(HostServices& host){if(auto data=host.readSave())return SaveData::decode(*data);return SaveData::empty(std::uint32_t(std::time(nullptr)));}
std::string levelName(std::string_view name,unsigned level){char b[24];std::snprintf(b,sizeof b,"_%02u",level+1);return std::string(name)+b;}

class SessionHost final:public HostServices {
public:
    explicit SessionHost(HostServices& h):host(h){}
    HostServices& host;float music=.8f,effects=1;std::string song;
    std::optional<Bytes> readSave()override{return host.readSave();}
    bool writeSave(std::span<const std::uint8_t> b)override{return host.writeSave(b);}
    void playSound(std::string_view n,float gain=1,float pan=0)override{if(!n.empty()&&n!="-"&&n!="none")host.playSound(n,gain*effects,pan);}
    void playMusic(std::string_view n,float gain=1)override{song=n;host.playMusic(n,gain*music);}
    void stopMusic()override{song.clear();host.stopMusic();}
    void requestName(std::string n,std::size_t max,std::function<void(std::optional<std::string>)> cb)override{host.requestName(std::move(n),max,std::move(cb));}
    bool restoreSave(std::span<const std::uint8_t> bytes)override{return host.restoreSave(bytes);}
    bool supportsSaveTransfer()const override{return host.supportsSaveTransfer();}
    void openTransferField(std::string s,bool ro,Rect r)override{host.openTransferField(std::move(s),ro,r);}
    void layoutTransferField(Rect r)override{host.layoutTransferField(r);}
    std::string transferFieldText()const override{return host.transferFieldText();}
    void closeTransferField()override{host.closeTransferField();}
    void copyTransferCode(std::string s)override{host.copyTransferCode(std::move(s));}
    void refreshMusic(){if(!song.empty())host.playMusic(song,music);}
};
}
struct GameSession::State {
    ResourceAccess& resources;SessionHost host;SaveData save;
    DataTable items,machines,descriptions;SpriteFont messageFont;
    std::unordered_map<int,std::string> attackIcons;
    BuildCatalog builds;WeaponCatalog weapons;BaseUpgradeCatalog baseUpgrades;AttackCatalog attacks;Settlement settlement;BattleModels models;
    Menu menu;Transition transition;GameHUD hud;UpgradeDialog upgrades;SummaryDialog summary;
    EmergencySupport support;SupportDialog supportDialog;
    UpgradeDialogState upgradeState;SummaryState summaryState;
    std::unique_ptr<World> world;std::optional<Help> help;
    std::unique_ptr<ValidationTools> validationTools;std::unique_ptr<ValidationPanel> validationPanel;bool validationOpen=false;
    std::unordered_map<std::uint64_t,Vec2> activeTouches;
    std::optional<MissionRequest> mission;
    std::optional<std::string> placement;
    std::optional<std::size_t> selectedAttack;
    FrameLayout layout;Frame frame;unsigned screen=1,panel=1;std::size_t slot=0;
    bool active=true,paused=false,validation=false,pendingSave=false;
    float panelOffset=0,mapLoadingAlpha=0;unsigned mapLoadingStep=6;
    std::string message;Button messageOK;
    State(ResourceAccess& r,HostServices& h,bool v):resources(r),host(h),save(loadSave(h)),
      items(table(r,"ItemInfo")),machines(table(r,"MachList")),descriptions(table(r,"Description_us")),messageFont(fontMetrics(r)),
      builds(BuildCatalog::load(table(r,"MachBuild_Parts"),table(r,"MachBuild_Name"))),weapons(WeaponCatalog::load(table(r,"WeaponInfo"),true)),
      baseUpgrades(table(r,"BaseUpgrade")),attacks(table(r,"SpAttackList")),settlement(builds,table(r,"MachRegen")),
      models(BattleModels::load(r)),
      menu(r,host,save,true),transition(r,host),hud(r,[this](Frame& f,std::string_view n,Vec2 p,float s){drawUnit(f,n,p,s);}),
      upgrades(r,[this](Frame& f,const UpgradeUnit& u,bool next,Vec2 p,float s){drawUnit(f,levelName(u.name,u.level+(next?1:0)),p,s);},
                 [this](Frame& f,const auto&,Vec2 p,float s){auto parts=PlayerLoadout::forBase(save,slot,baseUpgrades,BattleGeometry::phone());drawParts(f,parts.assembly,p,s);}),
      summary(r,[this](Frame& f,const StageReward& reward,Vec2 p,float s){drawReward(f,reward,p,s);}),supportDialog(r),validation(v){
        // ItemInfo.Icon is the shared visual identity. Extended rows reuse the
        // first original item image with that identity, while ItemId/Param1
        // continue to select the new combat level.
        std::unordered_map<int,std::string> originalIcons;
        for(std::size_t row=0;row<items.rowCount();++row)if(items.value(row,"Type")=="SpAtk"){
            const auto code=int(*items.integer(row,"Code")),icon=int(*items.integer(row,"Icon"));
            char image[32];std::snprintf(image,sizeof image,"Icon%05d.png",code);
            auto [it,inserted]=originalIcons.try_emplace(icon,image);attackIcons.emplace(code,it->second);
        }
        menu.setProfilePreparation([this](std::size_t selected){settlement.grantCampaignUnlock(save,selected,save.stage(selected));settlement.grantAircraftUnlocks(save,selected,save.stage(selected));});
        messageOK.bounds={128,279,64,32};if(validation){validationTools=std::make_unique<ValidationTools>(r);validationPanel=std::make_unique<ValidationPanel>(r);}rebuild();
    }
    std::string attackIcon(int itemId)const{return attackIcons.at(itemId);}
    std::size_t offset()const{return SaveData::slotStart+slot*SaveData::slotStride;}
    std::string label(std::string_view name)const{auto row=descriptions.findRow("MN_"+std::string(name));return row?std::string(descriptions.value(*row,"Value")):std::string(name);}
    void drawParts(Frame& f,const std::vector<AssemblyPart>& parts,Vec2 p,float scale,bool unitPreview=false){
        auto poses=models.pose(parts,scale,0,0,0);
        // UiObjMachParts::attachTo:X:Y: (0x3baf8) bakes unit art facing up.
        // Rotate the complete preview, including fixed-angle child parts.
        if(unitPreview)for(auto& pose:poses){const auto c=pose.localCenter;pose.localCenter={-c.y,c.x};pose.angle+=1.5707963267948966f;}
        models.appendSprites(f,poses,p,[&](std::string_view n){return resources.imageSize(n);});
    }
    std::unordered_map<std::string,float> aircraftPreviewScales;
    void drawUnit(Frame& f,std::string_view n,Vec2 p,float scale){
        const auto* d=builds.find(n);if(!d){auto owned=save.builds(slot);auto i=std::find_if(owned.begin(),owned.end(),[&](const auto& b){return b.name==n;});if(i!=owned.end())d=settlement.definition(*i);}
        if(!d)return;
        if(d->setType==0){auto parts=PlayerLoadout::fromBuild(*d,items,BattleGeometry::phone());drawParts(f,parts.assembly,p,scale,true);}
        else if(const auto* parts=models.machine(d->machine)){
            if(const auto* aircraft=alliedAircraftForModel(d->machine)){
                auto [it,inserted]=aircraftPreviewScales.try_emplace(std::string(aircraft->model),1.f);
                if(inserted){
                    // Original preview assembly path, fitted once to the existing
                    // icon well. World scale and original unit previews are unchanged.
                    Frame preview;drawParts(preview,*parts,{},1,true);float halfX=0,halfY=0;
                    for(const auto& sprite:preview.sprites){auto r=sprite.destination;
                        const float a=sprite.rotationDegrees*3.14159265359f/180.f,c=std::cos(a),t=std::sin(a);
                        for(float x:{-.5f,.5f})for(float y:{-.5f,.5f}){
                            halfX=std::max(halfX,std::abs(r.x+r.width*.5f+x*r.width*c-y*r.height*t));
                            halfY=std::max(halfY,std::abs(r.y+r.height*.5f+x*r.width*t+y*r.height*c));
                        }
                    }
                    it->second=std::min({1.f,36.f/std::max(1.f,halfX),30.f/std::max(1.f,halfY)});
                }
                scale*=it->second;
            }
            drawParts(f,*parts,p,scale,true);
        }
    }
    void drawReward(Frame& f,const StageReward& reward,Vec2 p,float scale){
        if(!reward.build.empty()){drawUnit(f,reward.build,p,scale);return;}
        if(reward.attack.empty())return;
        const int item=attacks.definition({reward.attack,0,0,false}).itemId;
        // Resolve appended ItemIds through their reused original icon identity.
        const auto name=attackIcon(item);auto size=resources.imageSize(name);
        for(unsigned i=0;i<reward.attackCount;++i){Sprite s;s.image=name;const float x=p.x+(float(i)*80-float(reward.attackCount-1)*40)*scale;s.destination={x-size.x*scale*.5f,p.y-size.y*scale*.5f,size.x*scale,size.y*scale};f.sprites.push_back(s);}
    }
    bool persist(){pendingSave=!host.writeSave(save.encode());if(!pendingSave)return true;message="Could not save progress.";return false;}
    void updatePause(){if(world)world->setPaused(mapLoadingStep<5||paused||validationOpen||supportDialog.visible()||help.has_value()||upgradeState.kind!=UpgradeDialogState::Kind::hidden||!message.empty()||!active);}
    void requestScreen(unsigned target){if(transition.request(target))updatePause();}
    void openMission(MissionRequest request){mission=request;requestScreen(4);}
    void loadNextMission(unsigned next){
        // GobWorld::setNextStage (WeMade 1.0.3, 0x16b74): one sound before the illustration.
        host.playSound("Title_TitleIn.wav");
        mission=MissionRequest{slot,next};mapLoadingStep=0;mapLoadingAlpha=0;
        summaryState.visible=false;summary.setState(summaryState);updatePause();
    }
    void tickMapLoading(){
        // GuiMapLoading::tick, WeMade 1.0.3 0x2ae94: fade in, replace, fade out.
        switch(mapLoadingStep){
        case 0:mapLoadingAlpha+=(1-mapLoadingAlpha)/20;if(mapLoadingAlpha>.99f){mapLoadingAlpha=1;mapLoadingStep=1;}break;
        case 1:mapLoadingStep=2;break;
        case 2:mapLoadingStep=4;break;
        case 4:changeScreen(4);mapLoadingStep=5;updatePause();break;
        case 5:mapLoadingAlpha-=mapLoadingAlpha/20;if(mapLoadingAlpha<.01f){mapLoadingAlpha=0;mapLoadingStep=6;}break;
        default:break;
        }
    }
    void changeScreen(unsigned target){
        screen=target;supportDialog.close();support.reset(1);summaryState={};summary.setState(summaryState);help.reset();placement.reset();selectedAttack.reset();upgradeState={};upgrades.setState(upgradeState);panelOffset=0;paused=false;panel=1;
        if(target==4){
            if(!mission)throw std::logic_error("Missing mission selection");slot=mission->slot;
            host.music=std::bit_cast<float>(save.readWord(offset()+0x2058));host.effects=std::bit_cast<float>(save.readWord(offset()+0x205c));
            if(!world){world=std::make_unique<World>(resources,host,save,slot,BattleGeometry::phone(),save.readWord(0));world->setFinishHandler([this](WorldState state){finish(state);});}
            world->setViewport(layout.width,layout.height,layout.top);world->setUIMargin(62+layout.bottom);
            world->start(mission->mission);support.reset(mission->mission+1);host.playMusic("GamePlay.mp3");
            if(save.readWord(offset()+0xc)<5){help.emplace();save.writeWord(offset()+0xc,save.readWord(offset()+0xc)+5);}
            updatePause();
        }else{
            world.reset();menu.applyScreen(target==1?Menu::Screen::title:target==2?Menu::Screen::profiles:Menu::Screen::missions);
        }
    }
    std::int64_t creditRoom()const{return std::int64_t(std::numeric_limits<std::int32_t>::max())-save.credits(slot);}
    std::int64_t cellRoom()const{return world?std::int64_t(std::max(0.,std::floor(double(world->cellCapacity())-world->cells()))):0;}
    void refreshSupport(){supportDialog.update(support,creditRoom(),cellRoom());}
    void claimSupport(unsigned row){
        if(row==3){supportDialog.close();}
        else if(world&&world->state()==WorldState::running){
            const auto amount=support.claim(row,creditRoom(),cellRoom());
            if(!amount.empty()){
                save.setCredits(slot,std::int32_t(save.credits(slot)+amount.credits));
                world->receiveSupportCells(unsigned(amount.cells));supportDialog.received(row);
            }
        }
        host.playSound("UI_Click.wav");refreshSupport();updatePause();refreshHUD();
    }
    void refreshHUD(){
        if(!world)return;
        // GuiGame::tick 0x276c4 reads the accumulated slot score at +0x2054.
        GameHUDState s;s.score=save.readWord(offset()+0x2054);s.credits=unsigned(std::max(0,save.credits(slot)));s.missionNumber=world->stage()+1;
        s.radarCamera=world->cameraVisibleY();
        for(const auto& a:world->actors())if(!a->removed)
            s.radarMarks.push_back({a->position,unsigned(a->player?0:1)+unsigned(a->base?2:0)});
        s.minerals=unsigned(std::max(0.f,world->cells()));s.maximumMinerals=unsigned(world->cellCapacity());s.musicVolume=host.music;s.effectsVolume=host.effects;
        s.paused=paused;s.selectedPanel=panel;s.productionVisible=true;s.mainPanelOffsetY=panelOffset;s.selectedAttack=selectedAttack;
        s.upgradeMode=upgradeState.kind!=UpgradeDialogState::Kind::hidden;
        s.baseUpgradeOpen=upgradeState.kind==UpgradeDialogState::Kind::base;
        s.gameTime=world->time();
        for(unsigned i=0;i<3;++i)if(!support.available(i,creditRoom(),cellRoom()).empty())s.supportAvailable=true;
        if(placement){if(auto row=descriptions.findRow("TapToTurretPos"))s.instruction=descriptions.value(*row,"Value");}
        s.cellUpgradeAvailable=world->cellLevel()<5;s.cellUpgradeCost=world->cellUpgradeCost();s.cellUpgradeLevel=world->cellLevel();
        for(const auto& a:world->actors())if(a->base){const float hp=std::max(0,a->hp)/float(a->maxHP);if(a->player)s.playerBaseHealth=hp;else s.enemyBaseHealth=hp;}
        for(const auto& b:save.builds(slot))if(b.onSlot){
            const auto* d=settlement.definition(b);if(!d)throw std::runtime_error("Unknown saved build");
            unsigned maximum=0;while(builds.find(levelName(b.name,maximum+1)))++maximum;
            ProductionRow row;row.machine=d->name;row.englishName=label(b.name);row.mineralCost=d->cost;row.upgradeCost=d->upgradeCost;row.level=b.level;row.maximumLevel=maximum+1;
            row.capacity=d->maximumConcurrent;row.occupied=static_cast<unsigned>(world->productionOccupied(b.name));
            row.enabled=s.upgradeMode||(world->state()==WorldState::running&&world->cells()>=d->cost&&world->productionRows().size()<7&&row.occupied<row.capacity);
            s.production.push_back(std::move(row));
        }
        for(const auto& a:save.attacks(slot))if(a.onSlot){const auto& d=attacks.definition(a);
            s.attacks.push_back({a.name,attackIcon(d.itemId),a.level,attacks.maximumLevel(a.name),a.count,std::uint64_t(attacks.upgradeCost(a)),a.level+1<attacks.maximumLevel(a.name)});}
        for(const auto& q:world->productionRows())s.queue.push_back({q.name,q.progress});hud.setState(std::move(s));
    }
    void refreshUpgrades(){
        for(unsigned i=0;i<6;++i){auto& row=upgradeState.base[i];const auto type=BaseUpgrade(i);row.level=save.baseUpgrade(slot,i);row.maximumLevel=baseUpgrades.maximumLevel(type)+1;
            const auto& current=baseUpgrades.level(type,row.level);row.cost=current.cost;row.enabled=row.level+1<row.maximumLevel&&save.credits(slot)>=int(row.cost);
            row.stat.clear();
            if(i==0){char n[32];std::snprintf(n,sizeof n,"BaseWpn%02u",row.level+1);if(auto* w=weapons.find(n)){const int value=w->attackPoint();row.stat="ATK "+std::to_string(value);if(row.level+1<row.maximumLevel){std::snprintf(n,sizeof n,"BaseWpn%02u",row.level+2);if(auto* next=weapons.find(n))row.stat+="(+"+std::to_string(next->attackPoint()-value)+")";}}}
            else if((i==1||i==5)&&current.parameter){const int divisor=i==1?100:1;const int value=int(*current.parameter)/divisor;row.stat=std::string(i==1?"DEF ":"Size ")+std::to_string(value);if(row.level+1<row.maximumLevel){auto next=baseUpgrades.level(type,row.level+1).parameter;if(next)row.stat+="(+"+std::to_string(int(*next)/divisor-value)+")";}}
        }
        upgradeState.baseCenter={160,-36+panelOffset};
        upgradeState.units.clear();const auto owned=save.builds(slot);
        for(const auto& b:owned){const auto* d=settlement.definition(b);if(!d)throw std::runtime_error("Unknown saved unit");UpgradeUnit u;u.name=b.name;u.englishName=label(b.name);u.level=b.level;
            while(builds.find(levelName(b.name,u.maximumLevel)))++u.maximumLevel;
            u.cost=d->upgradeCost;u.installed=b.onSlot;u.hasNext=b.level+1<u.maximumLevel;u.canUpgrade=u.hasNext&&save.credits(slot)>=int(u.cost);u.canInstall=!b.onSlot;
            u.stats=unitStats(*d);if(u.hasNext)u.nextStats=unitStats(*builds.find(levelName(b.name,b.level+1)));upgradeState.units.push_back(u);
        }
        upgradeState.attacks.clear();const auto ownedAttacks=save.attacks(slot);const auto attackSlots=std::count_if(ownedAttacks.begin(),ownedAttacks.end(),[](const auto& a){return a.onSlot;});
        for(const auto& a:ownedAttacks){const auto& d=attacks.definition(a);UpgradeAttack row;
            row.name=a.name;row.icon=attackIcon(d.itemId);row.level=a.level;row.maximumLevel=attacks.maximumLevel(a.name);row.count=a.count;row.buyCost=d.cost;row.upgradeCost=attacks.upgradeCost(a);
            row.installed=a.onSlot;row.hasNext=a.level+1<row.maximumLevel;row.canBuy=save.credits(slot)>=int(row.buyCost);row.canUpgrade=row.hasNext&&std::int64_t(save.credits(slot))>=std::int64_t(row.upgradeCost);row.canInstall=!a.onSlot&&attackSlots<7;
            if(row.hasNext){auto next=a;++next.level;row.nextIcon=attackIcon(attacks.definition(next).itemId);}upgradeState.attacks.push_back(row);
        }
        upgrades.setState(upgradeState);
    }
    std::array<int,4> unitStats(const BuildDefinition& d){
        // Same assembled body, two arm weapons and foot data as uiMach.
        if(d.setType==0){const auto p=PlayerLoadout::fromBuild(d,items,BattleGeometry::phone());const auto* w=weapons.find(p.weapon);
            return {w?w->attackPoint()*2:0,w?w->shotRange:0,p.hp,originalStepSpeedPoint(p.stepSize*2)};}
        const auto row=machines.findRow(d.machine);if(!row)return {};
        const auto m=MachineDefinition::fromTable(machines,*row,BattleGeometry::phone());int attack=0,range=0;
        if(const auto* parts=models.machine(d.machine)){auto walk=[&](auto&& self,const auto& list)->void{for(const auto& part:list){if(const auto* w=weapons.find(part.name)){if(alliedAircraftForModel(d.machine)){attack+=w->attackPoint();range=std::max(range,int(w->shotRange));}else{attack=w->attackPoint();range=w->shotRange;}}self(self,part.children);}};walk(walk,*parts);}
        return {attack,range,m.hp,originalVehicleSpeedPoint(m.speed)};
    }
    void action(HUDAction a){
        // Original GuiGame/Dlg* handlers use UI_Click; actions below select
        // their own cues or deliberately remain silent.
        std::string_view actionSound="UI_Click.wav";
        switch(a.buttonId){
        case 0x415:paused=!paused;break;
        case 0x416:paused=false;break;
        case 0x417:if(persist())requestScreen(1);break;
        case 0x419:paused=false;help.emplace();break;
        case 0x41a:message="Leader Board is unavailable.";break;
        case 0x413:upgradeState.kind=upgradeState.kind==UpgradeDialogState::Kind::base?UpgradeDialogState::Kind::hidden:UpgradeDialogState::Kind::base;placement.reset();refreshUpgrades();break;
        case 0x409:actionSound="UIType_BuildMach.wav";panel=1;placement.reset();selectedAttack.reset();if(upgradeState.kind==UpgradeDialogState::Kind::attacks){upgradeState.kind=UpgradeDialogState::Kind::units;upgradeState.selectedUnit.reset();refreshUpgrades();}break;
        case 0x40b:actionSound="UIType_SpAttack.wav";panel=2;placement.reset();if(upgradeState.kind==UpgradeDialogState::Kind::units){upgradeState.kind=UpgradeDialogState::Kind::attacks;upgradeState.selectedAttack.reset();refreshUpgrades();}break;
        case 0x40a:actionSound="UIType_Formation.wav";panel=3;break;
        case 0x40c:placement.reset();selectedAttack.reset();supportDialog.open();refreshSupport();break;
        case 0x3fe:upgradeState.kind=upgradeState.kind==UpgradeDialogState::Kind::hidden?(panel==2?UpgradeDialogState::Kind::attacks:UpgradeDialogState::Kind::units):UpgradeDialogState::Kind::hidden;refreshUpgrades();break;
        case 0x3fd:actionSound={};world->upgradeCells();break;
        case 0x40d:{std::vector<OwnedBuild> visible;for(auto b:save.builds(slot))if(b.onSlot)visible.push_back(b);
            if(a.row<visible.size()){const auto& b=visible[a.row];const auto* d=settlement.definition(b);
                if(!a.machine.empty()&&(!d||d->name!=a.machine))break;
                if(upgradeState.kind==UpgradeDialogState::Kind::units){const auto list=save.builds(slot);for(std::size_t i=0;i<list.size();++i)if(list[i].name==b.name)upgradeState.selectedUnit=i;refreshUpgrades();}
                else if(d->placementType==1){placement=b.name;selectedAttack.reset();host.playSound("SFX_OnMessage.wav");}
                else {placement.reset();world->enqueueBuild(b.name);}}
            break;}
        case 0x41b:actionSound={};host.music=a.value;save.writeWord(offset()+0x2058,std::bit_cast<std::uint32_t>(a.value));host.refreshMusic();break;
        case 0x41c:actionSound={};host.effects=a.value;save.writeWord(offset()+0x205c,std::bit_cast<std::uint32_t>(a.value));break;
        case 0x407:break;
        case 0x408:if(a.row<6&&baseUpgrades.purchase(save,slot,BaseUpgrade(a.row))){world->refreshBaseUpgrades();host.playSound("UpgradeBase.wav");}refreshUpgrades();break;
        case 0x411:upgradeState.selectedUnit=a.row;refreshUpgrades();break;
        case 0x412:upgradeState.selectedAttack=a.row;refreshUpgrades();break;
        case 0x403:case 0x404:if(attacks.purchase(save,slot,a.row,a.buttonId==0x403))host.playSound(a.buttonId==0x403?"UpgradeSpAttack.wav":"BuySpAttack.wav");refreshUpgrades();break;
        case 0x405:case 0x406:{auto list=save.attacks(slot);if(a.row<list.size()){auto slots=std::count_if(list.begin(),list.end(),[](const auto& item){return item.onSlot;});if(a.buttonId==0x406||slots<7){list[a.row].onSlot=a.buttonId==0x405;save.setAttacks(slot,list);host.playSound(a.buttonId==0x405?"Install.wav":"Uninstall.wav");selectedAttack.reset();refreshUpgrades();}}break;}
        case 0x3fc:world->order(0);break;
        case 0x40e:if(panel==3)break;else if(upgradeState.kind==UpgradeDialogState::Kind::attacks){auto list=save.attacks(slot);std::size_t row=0;for(std::size_t i=0;i<list.size();++i)if(list[i].onSlot){if(row++==a.row){upgradeState.selectedAttack=i;break;}}refreshUpgrades();}else selectedAttack=a.row;break;
        case 0x400:if(a.row<upgradeState.units.size()){if(settlement.upgradeBuild(save,slot,upgradeState.units[a.row].name))host.playSound("UpgradeMach.wav");refreshUpgrades();}break;
        case 0x401:case 0x402:{auto b=save.builds(slot);if(a.row<b.size()){b[a.row].onSlot=a.buttonId==0x401;save.setBuilds(slot,b);host.playSound(a.buttonId==0x401?"Install.wav":"Uninstall.wav");refreshUpgrades();}break;}
        case 0x3ff:upgradeState.kind=UpgradeDialogState::Kind::hidden;upgrades.setState(upgradeState);break;
        case 0x3f9:actionSound={};if(pendingSave&&!persist())break;loadNextMission(save.readWord(offset()+0x34));break;
        case 0x3fa:actionSound={};if(pendingSave&&!persist())break;loadNextMission(summaryState.mission);break;
        case 0x3f8:actionSound={};if(pendingSave&&!persist())break;menu.showMissions(slot);break;
        default:actionSound={};break;
        }
        if(!actionSound.empty())host.playSound(actionSound);updatePause();refreshHUD();
    }
    void finish(WorldState state){
        if(summaryState.visible)return;
        summaryState.visible=true;summaryState.cleared=state==WorldState::cleared;summaryState.mission=world->stage();
        const auto stats=world->statistics();summaryState.kills=stats.enemyDestroyed;summaryState.deaths=stats.playerDestroyed;summaryState.built=stats.produced;summaryState.elapsedSeconds=world->time()-world->startTime();
        if(summaryState.cleared)summaryState.reward=settlement.clear(save,slot,world->randomSource());
        summary.setState(summaryState);
        persist();
    }
    Vec2 upgradeOffset()const{return upgradeState.kind==UpgradeDialogState::Kind::base?layout.bottomRight():layout.center();}
    void cancelGameTouches(){
        for(const auto& [id,position]:activeTouches){Touch t{id,TouchPhase::cancelled,position};menu.touch(t);hud.touch(t);const auto modal=translatedTouch(t,layout.center());upgrades.touch(translatedTouch(t,upgradeOffset()));summary.touch(modal);supportDialog.touch(modal);if(validationPanel)validationPanel->touch(modal);if(world)world->touch(t);}
    }
    bool validationAction(unsigned command){
        if(!validationTools)return false;
        if(world)validationTools->setSlot(slot);
        if(command==validationOpenCommand){cancelGameTouches();if(world)validationTools->setSlot(slot);else if(screen>1&&save.active(menu.selectedProfile()))validationTools->setSlot(menu.selectedProfile());validationOpen=true;}
        else if(command==validationCloseCommand)validationOpen=false;
        else{
            const auto result=validationTools->apply(command,save,world.get());
            if(result.start){validationOpen=false;openMission({validationTools->slot(),validationTools->mission()});}
            if(result.persist)persist();
            if(result.changed){placement.reset();selectedAttack.reset();if(world){refreshUpgrades();refreshHUD();if(world->state()!=WorldState::running)validationOpen=false;}else menu.applyScreen(menu.screen());}
        }
        updatePause();rebuild();return true;
    }
    void tick(){
        if(!active)return;
        if(auto target=transition.tick())changeScreen(*target);
        tickMapLoading();
        if(screen<4){menu.tick();if(auto req=menu.takeScreenRequest())requestScreen(unsigned(*req)+1);if(auto req=menu.takeMissionRequest())openMission(*req);}
        else if(world){
            world->tick();world->takeEvents();
            if(world->state()==WorldState::running){
                const float research=baseUpgrades.level(BaseUpgrade::cellResearch,save.baseUpgrade(slot,unsigned(BaseUpgrade::cellResearch))).parameter.value_or(1);
                support.advance(world->time()-world->startTime(),world->cellCapacity(),research);
            }
            if(supportDialog.visible()){refreshSupport();supportDialog.tick();}
            const float destination=upgradeState.kind==UpgradeDialogState::Kind::base?260:0;
            panelOffset=std::abs(destination-panelOffset)>.1f?panelOffset+(destination-panelOffset)/5:destination;
            world->setUIMargin(62+panelOffset+layout.bottom);
            if(upgradeState.kind!=UpgradeDialogState::Kind::hidden)refreshUpgrades();refreshHUD();
            if(auto req=menu.takeScreenRequest())requestScreen(unsigned(*req)+1);
        }
        if(world)world->prepareDraw();rebuild();
    }
    void rebuild(){
        if(screen<4)frame=menu.frame();else{
            frame.sprites.clear();frame.meshes.clear();frame.logicalWidth=layout.width;frame.logicalHeight=layout.height;
            if(world)world->appendSprites(frame);hud.append(frame);
            auto n=frame.sprites.size(),m=frame.meshes.size();upgrades.append(frame);
            translateFrame(frame,n,m,upgradeOffset());
            n=frame.sprites.size();m=frame.meshes.size();summary.append(frame);if(help)help->append(frame);
            translateFrame(frame,n,m,layout.center());
            if(help&&n<frame.sprites.size())for(auto i=n;i<frame.sprites.size();++i)if(frame.sprites[i].image.empty())frame.sprites[i].destination={0,0,layout.width,layout.height};
        }
        const Vec2 modalOffset=layout.center();
        const auto overlayStart=frame.sprites.size(),overlayMeshes=frame.meshes.size();
        supportDialog.append(frame);
        if(validationPanel){validationPanel->setState(validationTools->panel(save,world.get(),validationOpen));validationPanel->append(frame);}
        if(!message.empty()){
            Sprite shade;shade.destination={0,0,320,480};shade.color={0,0,0,.7f};frame.sprites.push_back(shade);
            Sprite box;box.image="MessageBox.png";box.destination={32,208,256,64};frame.sprites.push_back(box);
            messageFont.draw(frame,message,{160-messageFont.width(message,10)/2,235},10);
            Sprite ok;ok.image="Common_Btn_OK.png";auto size=resources.imageSize(ok.image);ok.sourcePx=Rect{0,messageOK.pressed()?size.y/4:0,size.x,size.y/4};ok.destination=messageOK.bounds;frame.sprites.push_back(ok);
        }
        translateFrame(frame,overlayStart,overlayMeshes,modalOffset);
        for(auto i=overlayStart;i<frame.sprites.size();++i)if(frame.sprites[i].image.empty())frame.sprites[i].destination={0,0,frame.logicalWidth,frame.logicalHeight};
        if(mapLoadingStep<6){
            Sprite illustration;illustration.image="StageClear01.png";
            const auto size=resources.imageSize(illustration.image);const float scale=std::max(frame.logicalWidth/size.x,frame.logicalHeight/size.y);
            illustration.destination={(frame.logicalWidth-size.x*scale)/2,(frame.logicalHeight-size.y*scale)/2,size.x*scale,size.y*scale};
            illustration.color.a=mapLoadingAlpha;frame.sprites.push_back(std::move(illustration));
        }
        if(transition.visible()){
            Frame doors;transition.append(doors);
            const float scale=std::max(frame.logicalWidth/320.f,frame.logicalHeight/480.f);
            for(auto& s:doors.sprites){auto& r=s.destination;r.x=(r.x-160)*scale+frame.logicalWidth/2;r.y=(r.y-240)*scale+frame.logicalHeight/2;r.width*=scale;r.height*=scale;frame.sprites.push_back(std::move(s));}
        }
    }
    void touch(const Touch& t){
        if(t.phase==TouchPhase::began||t.phase==TouchPhase::moved)activeTouches[t.id]=t.position;else activeTouches.erase(t.id);
        if(!active||transition.visible()||mapLoadingStep<6)return;
        const auto modal=translatedTouch(t,layout.center());
        if(!message.empty()){if(messageOK.touch(modal).action==ButtonAction::pop){message.clear();updatePause();}rebuild();return;}
        if(validationPanel&&validationPanel->touch(modal)){for(const auto& event:validationPanel->takeEvents())validationAction(event.commandId);rebuild();return;}
        if(screen<4){menu.touch(t);if(auto req=menu.takeScreenRequest())requestScreen(unsigned(*req)+1);if(auto req=menu.takeMissionRequest())openMission(*req);}
        else if(world){
            if(help){if(help->touch(modal)){help.reset();updatePause();}}
            else if(summaryState.visible){summary.touch(modal);for(auto a:summary.takeActions())action(a);}
            else if(supportDialog.visible()){supportDialog.touch(modal);for(auto row:supportDialog.takeActions())claimSupport(row);}
            else if(upgradeState.kind!=UpgradeDialogState::Kind::hidden){const bool consumed=upgrades.touch(translatedTouch(t,upgradeOffset()));for(auto a:upgrades.takeActions())action(a);if(!consumed){hud.touch(t);for(auto a:hud.takeActions())action(a);}}
            else if(hud.touch(t)){for(auto a:hud.takeActions())action(a);}
            else if(auto point=world->touch(t)){
                if(placement&&world->enqueueBuild(*placement,*point)){placement.reset();refreshHUD();}
                else if(selectedAttack){std::size_t row=0;for(const auto& a:save.attacks(slot))if(a.onSlot){if(row++==*selectedAttack){if(world->activateSpecial(a.name,*point))selectedAttack.reset();break;}}refreshHUD();}
            }
        }
        rebuild();
    }
};
GameSession::GameSession(ResourceAccess& r,HostServices& h,bool validation):state_(std::make_unique<State>(r,h,validation)){}
GameSession::~GameSession()=default;
const Frame& GameSession::frame()const{return state_->frame;}
void GameSession::tick(){state_->tick();}
void GameSession::touch(const Touch& t){state_->touch(t);}
bool GameSession::scrollProduction(Vec2 point,float delta){
    auto& s=*state_;
    if(!s.active||s.transition.visible()||s.mapLoadingStep<6||!s.world||s.validationOpen||s.help||s.summaryState.visible||s.supportDialog.visible()||!s.message.empty())return false;
    bool consumed=false;
    if(s.upgradeState.kind==UpgradeDialogState::Kind::units){const auto offset=s.upgradeOffset();consumed=s.upgrades.scrollUnits({point.x-offset.x,point.y-offset.y},delta);}
    else if(s.upgradeState.kind==UpgradeDialogState::Kind::hidden)consumed=s.hud.scrollProduction(point,delta);
    if(consumed)s.rebuild();return consumed;
}
void GameSession::setActive(bool active){if(!active){state_->cancelGameTouches();state_->activeTouches.clear();}state_->active=active;if(!active&&state_->world)state_->paused=true;state_->updatePause();state_->refreshHUD();state_->rebuild();}
void GameSession::resize(float w,float h,float top,float bottom){
    auto& s=*state_;auto next=s.layout;next.resize(w,h,top,bottom);
    if(next.width==s.layout.width&&next.height==s.layout.height&&next.top==s.layout.top&&next.bottom==s.layout.bottom)return;
    s.cancelGameTouches();s.activeTouches.clear();s.layout=next;s.hud.setLayout(s.layout);s.menu.setLayout(s.layout);s.upgrades.setLayout(s.layout);
    if(s.world){s.world->setUIMargin(62+s.panelOffset+s.layout.bottom);s.world->setViewport(s.layout.width,s.layout.height,s.layout.top);s.refreshHUD();}
    s.rebuild();
}
World* GameSession::validationWorld(){return state_->validation?state_->world.get():nullptr;}
bool GameSession::applyValidationCommand(unsigned command){return state_->validationAction(command);}
std::optional<SaveData> GameSession::validationSave()const{if(!state_->validation)return {};return state_->save;}
std::unique_ptr<Game> createGame(ResourceAccess& r,HostServices& h,bool validation){return std::make_unique<GameSession>(r,h,validation);}
}
