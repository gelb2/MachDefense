#include "Core/Presentation/UpgradeDialog.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
namespace md {
namespace {
std::string metrics(ResourceAccess& r,std::string_view name){auto b=r.read("Generated/"+std::string(name)+"-48.tsv");return {b.begin(),b.end()};}
Rect rect(Vec2 p,float w,float h){return {p.x-w/2,480-p.y-h/2,w,h};}
constexpr std::array<std::string_view,6> baseNames={"BaseCannon","BaseDefense","BuildTime","CrResearch","CellResearch","CellStorage"};
constexpr float baseY[]={5,-27,-64,-96,-128,-160};
Vec2 grid(std::size_t i){return {32+46.f*(i%5),409.75f-62.f*(i/5)};}
}
UpgradeDialog::UpgradeDialog(ResourceAccess& r,DrawUnit u,DrawBase b):resources_(r),bold_(metrics(r,"TrebuchetMS-Bold")),regular_(metrics(r,"TrebuchetMS")),unit_(std::move(u)),base_(std::move(b)){}
void UpgradeDialog::setState(UpgradeDialogState s){
    if(s.kind!=state_.kind){selectedBase_=-1;unitScroll_=0;cancelUnitGesture();}
    if(s.units.size()!=state_.units.size())cancelUnitGesture();
    else for(std::size_t i=0;i<s.units.size();++i)if(s.units[i].name!=state_.units[i].name||s.units[i].level!=state_.units[i].level){cancelUnitGesture();break;}
    state_=std::move(s);
    if(state_.selectedUnit&&*state_.selectedUnit>=state_.units.size())state_.selectedUnit.reset();
    if(state_.selectedAttack&&*state_.selectedAttack>=state_.attacks.size())state_.selectedAttack.reset();
    if(selectedBase_>=0&&!state_.base[std::size_t(selectedBase_)].enabled)selectedBase_=-1;
    unitScroll_=std::clamp(unitScroll_,0.f,maximumUnitScroll());
    configure();
}
float UpgradeDialog::unitTopShift()const{return layout_.top-layout_.center().y;}
float UpgradeDialog::unitListHeight()const{
    const float content=state_.units.empty()?0.f:64.f+62.f*float((state_.units.size()-1)/5);
    return std::min(std::max(168.f,content),168.f+std::max(0.f,layout_.height-layout_.top-layout_.bottom-480.f));
}
float UpgradeDialog::maximumUnitScroll()const{
    const float content=state_.units.empty()?0.f:64.f+62.f*float((state_.units.size()-1)/5);
    return std::max(0.f,content-unitListHeight());
}
Rect UpgradeDialog::unitViewport()const{return {-4,38.25f+unitTopShift(),256,unitListHeight()};}
Vec2 UpgradeDialog::unitGrid(std::size_t i)const{auto p=grid(i);p.y+=unitScroll_-unitTopShift();return p;}
void UpgradeDialog::cancelUnitGesture(){if(unitGesture_)unitGesture_->cancelled=true;for(auto& c:controls_)c.button.reset();}
void UpgradeDialog::setUnitScroll(float value){
    if(!std::isfinite(value))return;
    unitScroll_=std::clamp(value,0.f,maximumUnitScroll());configure();
}
void UpgradeDialog::setLayout(FrameLayout value){layout_=value;cancelUnitGesture();unitScroll_=std::clamp(unitScroll_,0.f,maximumUnitScroll());configure();}
bool UpgradeDialog::scrollUnits(Vec2 point,float delta){
    const auto v=unitViewport();
    if(state_.kind!=UpgradeDialogState::Kind::units||!std::isfinite(delta)||point.x<v.x||point.x>=v.x+v.width||point.y<v.y||point.y>=v.y+v.height)return false;
    cancelUnitGesture();setUnitScroll(unitScroll_+delta);return true;
}
void UpgradeDialog::configure(){
    auto previous=std::move(controls_);controls_.clear();
    auto add=[&](unsigned id,std::size_t row,std::string_view name,Vec2 p,unsigned cols,unsigned rows,bool enabled=true){auto size=resources_.imageSize(name);Control c{id,row,{}};c.button.bounds=rect(p,size.x/cols*.5f,size.y/rows*.5f);c.button.enabled=enabled;for(const auto& old:previous)if(old.id==id&&old.row==row&&old.button.bounds.x==c.button.bounds.x&&old.button.bounds.y==c.button.bounds.y){auto b=old.button;b.enabled=enabled;c.button=b;break;}controls_.push_back(c);};
    if(state_.kind==UpgradeDialogState::Kind::base){
        auto p=state_.baseCenter;
        for(std::size_t i=0;i<6;++i)add(0x407,i,"Btn_BaseUpgrade_"+std::string(baseNames[i])+".png",{p.x+(i<2?36:0),p.y+baseY[i]},1,4,state_.base[i].enabled);
        add(0x408,0,"Btn_UpgradeBase_Upgrade.png",{p.x-28,p.y-200},1,4,selectedBase_>=0);
        add(0x413,0,"Btn_UpgradeBase_Close.png",{p.x+96,p.y-200},1,4);
    }else if(state_.kind==UpgradeDialogState::Kind::units){
        const float shift=unitTopShift()+unitListHeight()-168.f;
        for(std::size_t i=0;i<state_.units.size();++i)add(0x411,i,"Btn_MachBuild.png",unitGrid(i),2,1);
        add(0x3ff,0,"Common_Btn_CLOSE.png",{192,126.25f-shift},1,2);
        if(state_.selectedUnit){auto i=*state_.selectedUnit;const auto& u=state_.units[i];
            add(0x400,i,"UpgradeBtn_Upgrade.png",{198,183.25f-shift},1,4,u.canUpgrade);
            add(u.installed?0x402:0x401,i,u.installed?"UpgradeBtn_Uninstall.png":"UpgradeBtn_Install.png",{198,154.25f-shift},1,4,u.installed||u.canInstall);
        }
    }else if(state_.kind==UpgradeDialogState::Kind::attacks){
        for(std::size_t i=0;i<state_.attacks.size();++i){auto p=grid(i);p.x+=28;add(0x412,i,"Btn_MachBuild.png",p,2,1);}
        add(0x3ff,0,"Common_Btn_CLOSE.png",{220,126.25f},1,2);
        if(state_.selectedAttack){auto i=*state_.selectedAttack;const auto& a=state_.attacks[i];
            add(0x403,i,"UpgradeBtn_Upgrade.png",{226,183.25f},1,4,a.canUpgrade);
            add(0x404,i,"UpgradeBtn_Buy.png",{226,215.25f},1,4,a.canBuy);
            add(a.installed?0x406:0x405,i,a.installed?"UpgradeBtn_Uninstall.png":"UpgradeBtn_Install.png",{226,154.25f},1,4,a.installed||a.canInstall);
        }
    }
}
bool UpgradeDialog::pressed(unsigned id,std::size_t row)const{for(const auto& c:controls_)if(c.id==id&&c.row==row&&c.button.pressed())return true;return false;}
void UpgradeDialog::image(Frame& f,std::string_view name,Vec2 p,unsigned cols,unsigned rows,unsigned tile,Vec2 scale)const{
    const auto sz=resources_.imageSize(name);float w=sz.x/cols,h=sz.y/rows;Sprite s;s.image=name;s.sourcePx=Rect{float(tile%cols)*w,float(tile/cols)*h,w,h};s.destination=rect(p,w*.5f*scale.x,h*.5f*scale.y);f.sprites.push_back(std::move(s));
}
void UpgradeDialog::text(Frame& f,std::string_view t,Vec2 p,float size,float width,int align,bool regular,Color color)const{
    auto& font=regular?regular_:bold_;float x=p.x-width/2;if(align==1)x=p.x-font.width(t,size)/2;else if(align==2)x=p.x+width/2-font.width(t,size);
    auto start=f.sprites.size();font.draw(f,t,{x,480-p.y-4},size,color);if(regular)for(auto i=start;i<f.sprites.size();++i)f.sprites[i].image="Generated/TrebuchetMS-48.png";
}
void UpgradeDialog::append(Frame& f)const{if(state_.kind==UpgradeDialogState::Kind::base)base(f);else if(state_.kind==UpgradeDialogState::Kind::units)units(f);else if(state_.kind==UpgradeDialogState::Kind::attacks)attacks(f);}
void UpgradeDialog::base(Frame& f)const{
    const auto p=state_.baseCenter;
    if(base_)base_(f,state_.base,{p.x-140,480-(p.y-10)},.5f);
    for(std::size_t i=0;i<6;++i){const auto& row=state_.base[i];Vec2 at{p.x+(i<2?36:0),p.y+baseY[i]};
        image(f,"Btn_BaseUpgrade_"+std::string(baseNames[i])+".png",at,1,4,!row.enabled?2:(selectedBase_==int(i)?1:0));
        text(f,std::to_string(row.cost)+" cr",{at.x+6,at.y+1},8,32,2);
        text(f,"Lv. "+std::to_string(row.level+1)+"/"+std::to_string(row.maximumLevel),{at.x+47,at.y},6,32,0);
        if(!row.stat.empty())text(f,row.stat,{at.x+(i==5?92.5f:90),at.y},6,64,0);
    }
    image(f,"Btn_UpgradeBase_Upgrade.png",{p.x-28,p.y-200},1,4,selectedBase_<0?2:0);
    image(f,"Btn_UpgradeBase_Close.png",{p.x+96,p.y-200},1,4,0);
}
void UpgradeDialog::units(Frame& f)const{
    // GuiGame::handleButton phone root (-36,48), CommonDlg height 615*.5.
    const float top=unitTopShift(),extra=unitListHeight()-168.f,shift=top+extra;
    const auto clip=unitViewport();
    image(f,"CommonUI_Mid.png",{124,288-top-extra/2},1,1,0,{1,(307.5f+extra)/32});
    image(f,"CommonUI_Upgrade.png",{124,457.75f-top});image(f,"CommonUI_Btm.png",{124,118.25f-shift});
    image(f,"UpgradeEquip_MachInfo.png",{124,202.25f-shift});
    for(std::size_t i=0;i<state_.units.size();++i){const auto& u=state_.units[i];auto p=unitGrid(i);
        const float rowTop=480-p.y-32;
        if(rowTop+64<=clip.y||rowTop>=clip.y+clip.height)continue;
        const auto start=f.sprites.size();
        image(f,"Btn_MachBuild.png",p,2,1,0);
        if(unit_)unit_(f,u,false,{p.x,480-(p.y+10)},.5f);
        text(f,u.englishName,{p.x,p.y-4.5f},6.5f,64,1,false,{0,0,0,1});
        text(f,u.englishName,{p.x,p.y-5},6.5f,64);
        text(f,"Lv. "+std::to_string(u.level+1)+"/"+std::to_string(u.maximumLevel),{p.x-2,p.y+23},6,32,0);
        text(f,std::to_string(u.cost)+"cr",{p.x,p.y-20},7,32);
        if(u.installed||u.cost==0)image(f,"Btn_MachBuild.png",p,2,1,1);
        if(u.canUpgrade)image(f,"UpgradeEquip_UpgradeOn.png",{p.x,p.y-20.5f});
        if(state_.selectedUnit==i||pressed(0x411,i)){image(f,"UpgradeEquip_SelMach.png",p);f.sprites.back().blend=Blend::additive;}
        for(auto n=start;n<f.sprites.size();++n)f.sprites[n].clip=clip;
    }
    if(state_.selectedUnit)detail(f,*state_.selectedUnit);
    image(f,"Common_Btn_CLOSE.png",{192,126.25f-shift},1,2,pressed(0x3ff)?1:0);
}
void UpgradeDialog::detail(Frame& f,std::size_t i)const{
    const auto& u=state_.units[i];const Vec2 p{124,202.25f-unitTopShift()-(unitListHeight()-168.f)};image(f,"UpgradeMachBG.png",p);
    text(f,u.englishName+"  Lv. "+std::to_string(u.level+1),{p.x-31,p.y+34.5f},8,128,1,true);
    if(unit_){unit_(f,u,false,{p.x-64,480-(p.y+5)},.5f);if(u.hasNext)unit_(f,u,true,{p.x-2,480-(p.y+5)},.5f);}
    constexpr std::string_view names[]={"ATK.","Range","DEF.","SPD."};
    for(std::size_t n=0;n<4;++n){std::string value=std::string(names[n])+" : "+std::to_string(u.stats[n]);if(u.hasNext){int delta=u.nextStats[n]-u.stats[n];value+="("+std::string(delta>=0?"+":"")+std::to_string(delta)+")";}
        text(f,value,{p.x+(n%2?10:-60),p.y-(n<2?22:30)},6,64,0);
    }
    image(f,"UpgradeBtn_Upgrade.png",{p.x+74,p.y-19},1,4,u.canUpgrade?(pressed(0x400,i)?1:0):2);
    text(f,std::to_string(u.cost),{p.x+70,p.y-13},8,32,2);
    image(f,u.installed?"UpgradeBtn_Uninstall.png":"UpgradeBtn_Install.png",{p.x+74,p.y-48},1,4,u.installed||u.canInstall?(pressed(u.installed?0x402:0x401,i)?1:0):2);
}
void UpgradeDialog::attacks(Frame& f)const{
    // DlgUpgradeEquip 0x3778c uses the same five-column catalog; special
    // equipment root is (-8,48) in GuiGame, 28 pixels right of the unit root.
    image(f,"CommonUI_Mid.png",{152,288},1,1,0,{1,307.5f/32});
    image(f,"CommonUI_Upgrade.png",{152,457.75f});image(f,"CommonUI_Btm.png",{152,118.25f});
    image(f,"UpgradeEquip_MachInfo.png",{152,202.25f});
    for(std::size_t i=0;i<state_.attacks.size();++i){const auto& a=state_.attacks[i];auto p=grid(i);p.x+=28;
        image(f,"Btn_MachBuild.png",p,2,1,0);image(f,a.icon,{p.x,p.y+5.5f});
        text(f,"Lv. "+std::to_string(a.level+1)+"/"+std::to_string(a.maximumLevel),{p.x-2,p.y+24.5f},6,32,0);
        text(f,std::to_string(a.upgradeCost)+"cr",{p.x,p.y-20},7,32);
        if(a.installed||a.upgradeCost==0)image(f,"Btn_MachBuild.png",p,2,1,1);
        if(a.canUpgrade)image(f,"UpgradeEquip_UpgradeOn.png",{p.x,p.y-20.5f});
        if(state_.selectedAttack==i||pressed(0x412,i)){image(f,"UpgradeEquip_SelMach.png",p);f.sprites.back().blend=Blend::additive;}
    }
    if(state_.selectedAttack)attackDetail(f,*state_.selectedAttack);
    image(f,"Common_Btn_CLOSE.png",{220,126.25f},1,2,pressed(0x3ff)?1:0);
}
void UpgradeDialog::attackDetail(Frame& f,std::size_t i)const{
    // DlgUpgradeItem::initWithAttackSet 0x39f64. All prices/stock are supplied.
    const auto& a=state_.attacks[i];const Vec2 p{152,202.25f};image(f,"UpgradeMachBG.png",p);
    text(f,a.name+"  Lv. "+std::to_string(a.level+1),{p.x-31,p.y+34.5f},8,128,1,true);
    image(f,a.icon,{p.x-63.5f,p.y+6});if(a.hasNext&&!a.nextIcon.empty())image(f,a.nextIcon,{p.x-1.5f,p.y+6});
    image(f,"UpgradeBtn_Upgrade.png",{p.x+74,p.y-19},1,4,a.canUpgrade?(pressed(0x403,i)?1:0):2);
    text(f,std::to_string(a.upgradeCost),{p.x+70,p.y-13},8,32,2);
    image(f,"UpgradeBtn_Buy.png",{p.x+74,p.y+13},1,4,a.canBuy?(pressed(0x404,i)?1:0):2);
    text(f,std::to_string(a.buyCost),{p.x+70,p.y+19},8,32,2);
    text(f,"Count : "+std::to_string(a.count),{p.x-60,p.y-22},6,64,0);
    image(f,a.installed?"UpgradeBtn_Uninstall.png":"UpgradeBtn_Install.png",{p.x+74,p.y-48},1,4,a.installed||a.canInstall?(pressed(a.installed?0x406:0x405,i)?1:0):2);
}
bool UpgradeDialog::touch(const Touch& t){
    const bool finished=t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled;
    auto ignored=std::find(ignoredTouches_.begin(),ignoredTouches_.end(),t.id);
    if(ignored!=ignoredTouches_.end()){if(finished)ignoredTouches_.erase(ignored);return true;}
    if(unitGesture_&&unitGesture_->id==t.id&&t.phase!=TouchPhase::began){
        auto& g=*unitGesture_;const float dx=t.position.x-g.origin.x,dy=t.position.y-g.origin.y;
        if(!g.cancelled&&t.phase==TouchPhase::moved){
            if(!g.dragged&&std::hypot(dx,dy)>6){g.dragged=true;for(auto& c:controls_)c.button.reset();}
            if(g.dragged)setUnitScroll(g.initialScroll-dy);
        }
        if(finished){
            const auto v=unitViewport();
            if(t.phase==TouchPhase::ended&&!g.cancelled&&!g.dragged&&std::hypot(dx,dy)<=6&&state_.kind==UpgradeDialogState::Kind::units&&g.row&&
               t.position.x>=v.x&&t.position.x<v.x+v.width&&t.position.y>=v.y&&t.position.y<v.y+v.height){
                for(auto& c:controls_)if(c.id==0x411&&c.row==*g.row&&c.button.contains(t.position)){
                    state_.selectedUnit=c.row;actions_.push_back({0x411,c.row,0});break;
                }
            }
            cancelUnitGesture();unitGesture_.reset();configure();
        }
        return true;
    }
    if(state_.kind==UpgradeDialogState::Kind::hidden)return false;
    if(unitGesture_&&t.phase==TouchPhase::began){ignoredTouches_.push_back(t.id);return true;}
    const auto v=unitViewport();
    if(state_.kind==UpgradeDialogState::Kind::units&&t.phase==TouchPhase::began&&t.position.x>=v.x&&t.position.x<v.x+v.width&&t.position.y>=v.y&&t.position.y<v.y+v.height){
        unitGesture_=UnitGesture{t.id,t.position,unitScroll_,std::nullopt};
        for(auto it=controls_.rbegin();it!=controls_.rend();++it)if(it->id==0x411&&it->button.contains(t.position)){unitGesture_->row=it->row;it->button.touch(t);break;}
        return true;
    }
    for(auto it=controls_.rbegin();it!=controls_.rend();++it){if(it->id==0x411)continue;auto result=it->button.touch(t);
        if(result.action==ButtonAction::pop){auto id=it->id;auto row=it->row;
            if(id==0x407){selectedBase_=int(row);configure();}else if(id==0x411){state_.selectedUnit=row;configure();}else if(id==0x412){state_.selectedAttack=row;configure();}
            actions_.push_back({id,id==0x408?std::size_t(selectedBase_):row,0});return true;
        }if(result.consumed)return true;
    }
    if(state_.kind==UpgradeDialogState::Kind::base)return true;
    // Equipment is a child panel, not a full-screen modal. Original right
    // slot buttons and the bottom panel selectors remain reachable.
    if(state_.kind==UpgradeDialogState::Kind::units)return t.position.x>=-4&&t.position.x<252&&t.position.y>=6.25f+unitTopShift()&&t.position.y<377.75f+unitTopShift()+unitListHeight()-168.f;
    const float center=state_.kind==UpgradeDialogState::Kind::attacks?152.f:124.f;
    return t.position.x>=center-128&&t.position.x<center+128&&t.position.y>=6.25f&&t.position.y<377.75f;
}
std::vector<HUDAction> UpgradeDialog::takeActions(){auto a=std::move(actions_);actions_.clear();return a;}
}
