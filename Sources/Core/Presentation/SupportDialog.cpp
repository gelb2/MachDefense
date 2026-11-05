#include "Core/Presentation/SupportDialog.hpp"
#include <algorithm>
namespace md {
namespace {
std::string metrics(ResourceAccess& r){auto b=r.read("Generated/TrebuchetMS-Bold-48.tsv");return {b.begin(),b.end()};}
std::string amount(SupportAmount a){std::string s;if(a.credits)s=std::to_string(a.credits)+" CR";if(a.cells){if(!s.empty())s+=" + ";s+=std::to_string(a.cells)+" CELLS";}return s.empty()?"NONE":s;}
}
SupportDialog::SupportDialog(ResourceAccess& r):resources_(r),font_(metrics(r)){
    for(unsigned i=0;i<3;++i)buttons_[i].bounds={52,136.f+70*i,216,32};buttons_[3].bounds={128,402,64,32};
}
void SupportDialog::open(){visible_=true;actions_.clear();for(auto& b:buttons_)b.reset();message("COMMAND RESERVE\nDeliveries arrive during combat. Unclaimed supplies remain here until this mission ends.");}
void SupportDialog::close(){visible_=false;actions_.clear();for(auto& b:buttons_)b.reset();}
void SupportDialog::update(const EmergencySupport& s,std::int64_t credits,std::int64_t cells){
    static constexpr std::string_view names[]={"CREDITS","CELLS","MIXED SUPPLY"};
    for(unsigned i=0;i<3;++i){
        const auto pending=s.pending(i),available=s.available(i,credits,cells);const auto seconds=s.remaining(i);
        if(!labelsReady_||pending!=pending_[i]||available!=available_[i]){
            rowLabels_[i]=std::string(names[i])+" / "+amount(pending);
            claimLabels_[i]=!available.empty()?"CLAIM "+amount(available):pending.empty()?"WAITING FOR SUPPLIES":"STORAGE FULL";
        }
        if(!labelsReady_||seconds!=seconds_[i])countdownLabels_[i]="Next delivery in "+std::to_string(seconds)+"s";
        pending_[i]=pending;available_[i]=available;seconds_[i]=seconds;
        const bool enabled=!available.empty();if(!enabled)buttons_[i].reset();buttons_[i].enabled=enabled;
    }
    labelsReady_=true;
}
void SupportDialog::message(std::string_view text){
    lines_.clear();std::string line,word;
    auto flush=[&](){if(word.empty())return;const auto next=line.empty()?word:line+" "+word;if(font_.width(next,9)>232&&!line.empty()){lines_.push_back(line);line=word;}else line=next;word.clear();};
    for(char c:text){if(c==' '||c=='\n'){flush();if(c=='\n'){lines_.push_back(line);line.clear();}}else word+=c;}flush();if(!line.empty())lines_.push_back(line);typeTicks_=0;
}
void SupportDialog::received(std::size_t row){
    static constexpr std::string_view messages[]={
        "GENERAL J\nWelcome back, Commander. This front has waited since 2011. Command has released emergency credits.",
        "ADMIRAL B\nYour return is noted, Commander. The supply reserve is yours. Keep your machines moving.",
        "MARSHAL P\nYou have returned to a war left unfinished in 2011. Credits and cells are authorized. Hold the line."};
    if(row<3)message(messages[row]);
}
void SupportDialog::tick(){if(visible_&&typeTicks_<4096)++typeTicks_;}
void SupportDialog::sprite(Frame& f,std::string_view name,Rect destination,unsigned rows,unsigned tile)const{const auto size=resources_.imageSize(name);Sprite s;s.image=name;s.sourcePx=Rect{0,size.y/rows*tile,size.x,size.y/rows};s.destination=destination;f.sprites.push_back(std::move(s));}
void SupportDialog::text(Frame& f,std::string_view s,Vec2 p,float size,Color color)const{font_.draw(f,s,p,size,color);}
void SupportDialog::append(Frame& f)const{
    if(!visible_)return;
    Sprite shade;shade.destination={0,0,320,480};shade.color={0,0,0,.65f};f.sprites.push_back(shade);
    sprite(f,"CommonUI_Top.png",{32,64,256,32});sprite(f,"CommonUI_Mid.png",{32,96,256,304});sprite(f,"CommonUI_Btm.png",{32,400,256,32});
    const std::string_view title="EMERGENCY RESUPPLY";text(f,title,{160-font_.width(title,14)/2,84},14);
    for(unsigned i=0;i<3;++i){const float y=120+70.f*i;const auto color=buttons_[i].enabled?Color{.94f,1,1,1}:Color{.5f,.6f,.6f,1};
        text(f,rowLabels_[i],{52,y},9);
        sprite(f,"SysMenu_Btn.png",buttons_[i].bounds,2,buttons_[i].pressed()?1:0);
        const auto& label=claimLabels_[i];
        text(f,label,{160-font_.width(label,11)/2,y+26},11,color);
        text(f,countdownLabels_[i],{52,y+54},8);
    }
    unsigned count=typeTicks_/2;float y=334;bool cursor=false;
    for(const auto& line:lines_){const auto n=std::min<std::size_t>(line.size(),count);const auto shown=std::string_view(line).substr(0,n);text(f,shown,{44,y},9);count-=unsigned(n);
        if(n<line.size()){if((typeTicks_/24)%2==0)text(f,"_",{44+font_.width(shown,9),y},9);cursor=true;break;}y+=12;
    }
    if(!cursor&&(typeTicks_/24)%2==0)text(f,"_",{44,y},9);
    sprite(f,"Common_Btn_CLOSE.png",buttons_[3].bounds,2,buttons_[3].pressed()?1:0);
}
bool SupportDialog::touch(const Touch& t){if(!visible_)return false;for(unsigned i=0;i<4;++i){const auto r=buttons_[i].touch(t);if(r.action==ButtonAction::pop){actions_.push_back(i);return true;}if(r.consumed)return true;}if(t.phase==TouchPhase::ended&&t.position.y>=330&&t.position.y<398)typeTicks_=4096;return true;}
std::vector<unsigned> SupportDialog::takeActions(){auto a=std::move(actions_);actions_.clear();return a;}
}
