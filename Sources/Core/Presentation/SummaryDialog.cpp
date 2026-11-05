#include "Core/Presentation/SummaryDialog.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <utility>
namespace md {
namespace {std::string metrics(ResourceAccess& r){auto b=r.read("Generated/TrebuchetMS-Bold-48.tsv");return {b.begin(),b.end()};}}
SummaryDialog::SummaryDialog(ResourceAccess& r,DrawReward d):resources_(r),font_(metrics(r)),reward_(std::move(d)){}
float SummaryDialog::height()const{return state_.cleared&&state_.mission<79?160.f:256.f;}
void SummaryDialog::setState(SummaryState state){
    const auto previous=std::move(controls_);controls_.clear();state_=std::move(state);if(!state_.visible)return;
    const bool next=state_.cleared&&state_.mission<79;
    const unsigned ids[]={next?0x3f9u:0x3fau,0x3f8,0x41a};
    const std::string names[]={next?"SummaryBtn_NextStage.png":"SummaryBtn_Retry.png","SummaryBtn_SelectStage.png","SummaryBtn_LeaderBoard.png"};
    const float y=300-height()/2-8;
    for(unsigned i=0;i<3;++i){auto size=resources_.imageSize(names[i]);Control c{ids[i],names[i],{}};c.button.bounds={198-64.f*i-size.x*.25f,480-y-size.y*.0625f,size.x*.5f,size.y*.125f};
        for(const auto& old:previous)if(old.id==c.id&&old.button.bounds.y==c.button.bounds.y)c.button=old.button;
        controls_.push_back(c);
    }
}
void SummaryDialog::image(Frame& f,std::string_view name,Vec2 p,unsigned rows,unsigned tile,Vec2 scale)const{auto sz=resources_.imageSize(name);float h=sz.y/rows;Sprite s;s.image=name;s.sourcePx=Rect{0,tile*h,sz.x,h};s.destination={p.x-sz.x*.25f*scale.x,480-p.y-h*.25f*scale.y,sz.x*.5f*scale.x,h*.5f*scale.y};f.sprites.push_back(std::move(s));}
void SummaryDialog::text(Frame& f,std::string_view s,Vec2 p,float size,Color c)const{font_.draw(f,s,{p.x-font_.width(s,size)/2,480-p.y-8},size,c);}
void SummaryDialog::append(Frame& f)const{
    if(!state_.visible)return;
    const float h=height(),top=300+h/2+16,bottom=300-h/2-16;
    image(f,"CommonUI_Mid.png",{130,300},1,0,{1,h/32});
    // CommonDlg::init falls back to Top when CommonUI_Victory is missing in
    // the source bundle. Preserve that observed branch for defeat/final stage.
    image(f,state_.cleared&&state_.mission<79?"CommonUI_StageClear.png":"CommonUI_Top.png",{130,top});
    image(f,"CommonUI_Btm.png",{130,bottom});
    for(unsigned i=0;i<controls_.size();++i)image(f,controls_[i].image,{198-64.f*i,bottom+8},4,controls_[i].button.pressed()?1:0);
    text(f,"Kill count : "+std::to_string(state_.kills),{130,top-40},8);
    text(f,"Dead count : "+std::to_string(state_.deaths),{130,top-50},8);
    const unsigned seconds=std::isfinite(state_.elapsedSeconds)&&state_.elapsedSeconds>0?unsigned(std::min(state_.elapsedSeconds,double(0x7fffffff))):0;
    char time[80];std::snprintf(time,sizeof(time),"Play Time : %u' %02u\"",seconds/60,seconds%60);text(f,time,{130,top-60},8);
    if(state_.cleared){
        text(f,"Stage Clear Bonus!",{130,top-80},12);text(f,std::to_string(state_.reward.credits)+" cr",{130,top-92.5f},12,{1,100/255.f,100/255.f,1});
        if(!state_.reward.build.empty()){
            text(f,"You've got a new Mach.",{130,top-110},12);
            if(reward_)reward_(f,state_.reward,{90,480-(top-135)},.5f);
            // Original QobText is left aligned in a 128px logical box.
            font_.draw(f,state_.reward.build,{126,480-(top-135)-8},10,{1,100/255.f,100/255.f,1});
        }else if(!state_.reward.attack.empty()){
            text(f,state_.reward.attack,{130,top-155},10,{1,100/255.f,100/255.f,1});
            if(reward_)for(unsigned i=0;i<state_.reward.attackCount;++i){auto one=state_.reward;one.attackCount=1;const float x=130-20.f*(state_.reward.attackCount-1)+40.f*i;reward_(f,one,{x,480-(top-130)},.5f);}
        }
    }
}
bool SummaryDialog::touch(const Touch& t){if(!state_.visible)return false;for(auto& c:controls_){auto r=c.button.touch(t);if(r.action==ButtonAction::pop){actions_.push_back({c.id,0,0});return true;}if(r.consumed)return true;}return true;}
std::vector<HUDAction> SummaryDialog::takeActions(){auto a=std::move(actions_);actions_.clear();return a;}
}
