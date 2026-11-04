#include "GameHUD.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

namespace md {
namespace {
std::string metrics(ResourceAccess& r){auto b=r.read("Generated/TrebuchetMS-Bold-48.tsv");return {b.begin(),b.end()};}
Rect bounds(Vec2 yUp,float w,float h){return {yUp.x-w/2,480-yUp.y-h/2,w,h};}
float ratio(float x){return std::isfinite(x)?std::clamp(x,0.f,1.f):0.f;}
}
GameHUD::GameHUD(ResourceAccess& r,DrawMachine draw):resources_(r),font_(metrics(r)),machine_(std::move(draw)){configure();}
void GameHUD::setState(GameHUDState s){
    const bool changed=s.paused!=state_.paused||s.baseUpgradeOpen!=state_.baseUpgradeOpen||s.productionVisible!=state_.productionVisible||s.upgradeMode!=state_.upgradeMode||s.production.size()!=state_.production.size()||s.attacks.size()!=state_.attacks.size()||s.selectedPanel!=state_.selectedPanel||s.mainPanelOffsetY!=state_.mainPanelOffsetY;
    state_=std::move(s);
    if(changed){slider_.reset();configure();}
    else for(auto& c:controls_)if(c.id==0x40d)c.button.enabled=c.row<state_.production.size()&&state_.production[c.row].enabled&&!state_.baseUpgradeOpen;
}
void GameHUD::configure(){
    controls_.clear();
    auto add=[&](unsigned id,Vec2 p,float w,float h,std::size_t row=0,bool enabled=true){Control c{id,row,{}};c.button.bounds=bounds(p,w,h);c.button.enabled=enabled;controls_.push_back(c);};
    if(state_.paused){
        const unsigned ids[]={0x416,0x417,0x419,0x41a};
        for(unsigned i=0;i<4;++i)add(ids[i],{130,384-24.f*i},240,24);
        const auto close=resources_.imageSize("Common_Btn_CLOSE.png");add(0x416,{198,200},close.x*.5f,close.y*.25f,1);
        return;
    }
    const float shift=state_.mainPanelOffsetY;
    add(0x415,{284,34+shift},120,50,0,!state_.upgradeMode);add(0x413,{284,11+shift},80,22,0,!state_.upgradeMode||state_.baseUpgradeOpen);
    add(0x409,{115.5f,37+shift},32,32,0,!state_.baseUpgradeOpen);add(0x40b,{161,37+shift},32,32,0,!state_.baseUpgradeOpen);add(0x40c,{208.5f,37+shift},32,32,0,!state_.baseUpgradeOpen);
    add(0x3fd,{287,465},69,32,0,!state_.baseUpgradeOpen);
    if(state_.selectedPanel!=3){const auto size=resources_.imageSize("Btn_UpgradeEquip.png");add(0x3fe,{293,74},size.x*.5f,size.y*.125f,0,!state_.baseUpgradeOpen);}
    if(state_.productionVisible&&state_.selectedPanel==1)for(std::size_t i=0;i<std::min<std::size_t>(7,state_.production.size());++i)add(0x40d,{292,428-51.f*i},90,48,i,state_.production[i].enabled&&!state_.baseUpgradeOpen);
    if(state_.productionVisible&&state_.selectedPanel==2)for(std::size_t i=0;i<state_.attacks.size();++i){auto size=resources_.imageSize(state_.attacks[i].icon);add(0x40e,{292,428-51.f*i},size.x*.5f,size.y*.5f,i,!state_.baseUpgradeOpen);}
    if(state_.productionVisible&&state_.selectedPanel==3)for(std::size_t i=0;i<5;++i){auto size=resources_.imageSize("FormationIcon_0"+std::to_string(i+1)+".png");add(0x40e,{292,428-51.f*i},size.x*.5f,size.y*.5f,i,!state_.baseUpgradeOpen);}
}
bool GameHUD::pressed(unsigned id,std::size_t row)const{for(const auto& c:controls_)if(c.id==id&&c.row==row&&c.button.pressed())return true;return false;}
void GameHUD::image(Frame& frame,std::string_view name,Vec2 p,unsigned columns,unsigned rows,unsigned tile,Vec2 scale)const{
    const auto size=resources_.imageSize(name);const float w=size.x/columns,h=size.y/rows;
    Sprite s;s.image=name;s.sourcePx=Rect{float(tile%columns)*w,float(tile/columns)*h,w,h};
    s.destination=bounds(p,w*.5f*scale.x,h*.5f*scale.y);frame.sprites.push_back(std::move(s));
}
void GameHUD::number(Frame& frame,std::string_view name,std::string_view digits,Vec2 p,float pitch,float align)const{
    if(digits.empty())return;
    float x=p.x-float(digits.size()-1)*pitch*align;
    for(char c:digits){unsigned n=0;if(c>='0'&&c<='9')n=unsigned(c-'0');else if(c=='.')n=10;else if(c=='/')n=11;else if(c==':')n=12;else if(c=='-')n=13;else if(c=='%')n=14;else n=15;
        image(frame,name,{x,p.y},16,1,n);x+=pitch;}
}
void GameHUD::text(Frame& f,std::string_view t,Vec2 p,float size,Color color)const{font_.draw(f,t,{p.x-font_.width(t,size)/2,480-p.y-size/2},size,color);}
void GameHUD::append(Frame& f)const{
    // GuiGame::init 0x25748, phone branch. Each root-child coordinate is
    // translated by (160,240); forRetina art is presented at half size.
    // GuiGame::init 0x25748 gives baseBuildSlot layer 5. The gameUI
    // background and its QuickBtn children inherit GuiGame layer 9 through
    // QobBase::addChild 0x3f274. Draw the slot background before that HUD.
    if(state_.productionVisible&&state_.selectedPanel>=1&&state_.selectedPanel<=3)
        image(f,"GameUI_MachSlot.png",{256,252});
    const auto panelStart=f.sprites.size();
    image(f,"iPon_GameUI_BG@2x.png",{160,62});
    image(f,"GameUI_BtmL.png",{32,-130});image(f,"GameUI_BtmR.png",{224,-130});
    for(auto i=panelStart;i<f.sprites.size();++i)f.sprites[i].destination.y-=state_.mainPanelOffsetY;
    image(f,"RaderBG.png",{18,398.5f});
    image(f,"GameUI_HpGauge.png",{58,471},2,1,0);image(f,"GameUI_HpGauge.png",{192,471},2,1,1);
    const float my=ratio(state_.playerBaseHealth),enemy=ratio(state_.enemyBaseHealth);
    image(f,"EnemyGauge.png",{31.5f+40.5f*my,473.5f},1,2,0,{my,1});
    image(f,"EnemyGauge.png",{217-40.5f*enemy,473.5f},1,2,1,{enemy,1});
    const auto buttonStart=f.sprites.size();
    number(f,"NumSet_Cr.png",std::to_string(state_.score),{6,57},4.5f,0);
    number(f,"NumSet_Cr.png",std::to_string(state_.credits),{72,13},4.5f,1);
    number(f,"NumSet_Stage.png",std::to_string(state_.missionNumber),{64,34},6,.5f);
    image(f,"MachButton.png",{115.5f,37},2,4,pressed(0x409)||state_.selectedPanel==1?1:0);
    image(f,"MachButton.png",{161,37},2,4,pressed(0x40b)||state_.selectedPanel==2?3:2);
    image(f,"MachButton.png",{208.5f,37},2,4,pressed(0x40c)||state_.selectedPanel==3?7:6);
    image(f,"iPon_QuickBtn@2x.png",{284,34},2,2,pressed(0x415)?1:0);
    image(f,"iPon_QuickBtn@2x.png",{284,11},2,2,pressed(0x413)?3:2);
    for(auto i=buttonStart;i<f.sprites.size();++i)f.sprites[i].destination.y-=state_.mainPanelOffsetY;
    if(state_.productionVisible&&state_.selectedPanel==1){
        for(std::size_t i=0;i<std::min<std::size_t>(7,state_.production.size());++i){
            const auto& row=state_.production[i];const float y=428-51.f*i;
            image(f,"Btn_BuildMachBG.png",{292,y});image(f,"Btn_BuildMach.png",{282.5f,y-8.5f});
            if(machine_)machine_(f,row.machine,{292,480-y},.5f);
            // ImageAttacher::attachText uses 14px in a half-scale composite.
            if(!state_.upgradeMode)text(f,std::to_string(row.mineralCost),{262,y+15.5f},7);
            else{
                image(f,"Btn_BuildMachUpgrade.png",{255.5f,y-9});
                text(f,std::to_string(row.level+1)+"/"+std::to_string(row.maximumLevel),{262,y+15.5f},7);
                // DlgBuildMach::refreshButtonsWithType: 0x2e090 uses Align=2
                // at x=158 in its half-scale composite: right edge x=271.
                const auto cost=std::to_string(row.upgradeCost)+"cr";
                font_.draw(f,cost,{271-font_.width(cost,6),480-(y-10.5f)-3},6);
            }
            text(f,row.englishName,{292,y-21.5f},5.5f,{240/255.f,1,1,1});
            for(unsigned n=0;n<row.capacity;++n)image(f,"Btn_BuildMachInd.png",{269.5f,y-1.5f-6.5f*n},2,1,n<row.occupied?0:1);
            if(row.upgrading)image(f,"Btn_BuildMachUpgradeOn.png",{256,y-1.5f});
            if(pressed(0x40d,i)){image(f,"SelMach.png",{292,y});f.sprites.back().blend=Blend::additive;}
        }
    }
    // DlgSpecialAttack::refreshButtonsWithType 0x30de0 and
    // DlgFormation::refreshButtons 0x3dd94: local(-35,-176) at(327,604).
    if(state_.productionVisible&&state_.selectedPanel==2){
        for(std::size_t i=0;i<state_.attacks.size();++i){const auto& a=state_.attacks[i];const float y=428-51.f*i;
            image(f,a.icon,{292,y});image(f,"Btn_BuildMach.png",{282.5f,y-8.5f});
            text(f,std::to_string(a.level+1)+"/"+std::to_string(a.maximumLevel),{262,y+14},7,{240/255.f,1,1,1});
            text(f,std::to_string(a.count),{292,y-21},6.5f,{240/255.f,1,1,1});
            if(state_.upgradeMode){image(f,"Btn_BuildMachUpgrade.png",{255.5f,y-9});
                const auto cost=std::to_string(a.upgradeCost)+"cr";
                font_.draw(f,cost,{271.5f-font_.width(cost,6),480-(y-12)-4},6,{240/255.f,1,1,1});
                if(a.upgradeAvailable)image(f,"Btn_BuildMachUpgradeOn.png",{256,y-1.5f});
            }
            if(state_.selectedAttack==i||pressed(0x40e,i)){image(f,"SelMach.png",{292,y});f.sprites.back().blend=Blend::additive;}
        }
    }
    if(state_.productionVisible&&state_.selectedPanel==3){
        for(std::size_t i=0;i<5;++i){const float y=428-51.f*i;
            image(f,"FormationIcon_0"+std::to_string(i+1)+".png",{292,y});image(f,"Btn_BuildMach.png",{282.5f,y-8.5f});
            if(state_.selectedFormation==i||pressed(0x40e,i)){image(f,"SelMach.png",{292,y});f.sprites.back().blend=Blend::additive;}
        }
    }
    if(state_.selectedPanel!=3)image(f,"Btn_UpgradeEquip.png",{293,74},1,4,(state_.selectedPanel==2?2:0)+(pressed(0x3fe)?1:0));
    text(f,state_.cellUpgradeLevel>=5?"FULL":std::to_string(state_.cellUpgradeCost)+" cell",{304,457.5f},7);
    if(state_.cellUpgradeAvailable)image(f,"CellUpgrade.png",{267,457.5f});
    number(f,"NumSet_Stage.png",std::to_string(state_.minerals),{286,471},6,1);
    number(f,"NumSet_Level.png",std::to_string(state_.maximumMinerals),{298,469},4.5f,0);
    for(std::size_t i=0;i<state_.queue.size();++i){
        const auto& row=state_.queue[i];const float y=428-36.f*i,progress=ratio(row.progress);
        image(f,"MakeMach_Mach.png",{228,y});
        if(machine_)machine_(f,row.machine,{228,480-y},.5f);
        image(f,"BuildGauge.png",{212+16*progress,y-13},1,1,0,{progress,1});
    }
    if(state_.paused)pause(f);
}
void GameHUD::pause(Frame& f)const{
    image(f,"SysMenu_Mid.png",{130,304},1,1,0,{1,6});
    image(f,"SysMenu_Top.png",{130,416});image(f,"SysMenu_Btm.png",{130,192});
    constexpr unsigned ids[]={0x416,0x417,0x419,0x41a};
    constexpr std::string_view labels[]={"Resume Game","Save & Exit Game","Help","Leader Board"};
    for(unsigned i=0;i<4;++i){const Vec2 p{130,384-24.f*i};image(f,"SysMenu_Btn.png",p,1,2,pressed(ids[i])?1:0);text(f,labels[i],p,10,{140/255.f,240/255.f,1,1});}
    image(f,"VolOption.png",{128,255},1,2,0);image(f,"VolOption.png",{128,232},1,2,1);
    image(f,"VolOption_Thumb.png",{92+130*ratio(state_.musicVolume),255});
    image(f,"VolOption_Thumb.png",{92+130*ratio(state_.effectsVolume),232});
    image(f,"Common_Btn_CLOSE.png",{198,200},1,2,pressed(0x416,1)?1:0);
}
bool GameHUD::touch(const Touch& t){
    if(state_.paused){
        if(t.phase==TouchPhase::began){
            for(unsigned i=0;i<2;++i){const float volume=ratio(i?state_.effectsVolume:state_.musicVolume),y=i?232:255;Button thumb;thumb.bounds=bounds({92+130*volume,y},32,23);
                if(thumb.contains(t.position)){slider_={{t.id,0x41b+i}};return true;}}
        }
        if(slider_&&slider_->first==t.id){
            if(t.phase==TouchPhase::moved){const float value=ratio((t.position.x-92)/130);actions_.push_back({slider_->second,0,value});if(slider_->second==0x41b)state_.musicVolume=value;else state_.effectsVolume=value;}
            if(t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled)slider_.reset();
            return true;
        }
    }
    // Child hit priority is reverse insertion order, as QobBase dispatches it.
    for(auto i=controls_.rbegin();i!=controls_.rend();++i){const auto result=i->button.touch(t);
        if(result.action==ButtonAction::pop){actions_.push_back({i->id,i->row,0});return true;}
        if(result.consumed)return true;
    }
    return state_.paused;
}
std::vector<HUDAction> GameHUD::takeActions(){auto a=std::move(actions_);actions_.clear();return a;}
}
