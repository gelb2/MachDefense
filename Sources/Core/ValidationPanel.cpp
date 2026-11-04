#include "ValidationPanel.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
namespace md {
namespace {
std::string metrics(ResourceAccess& r){auto b=r.read("Generated/TrebuchetMS-Bold-48.tsv");return {b.begin(),b.end()};}
bool ascii(std::string_view s){return std::all_of(s.begin(),s.end(),[](unsigned char c){return c>=32&&c<=126;});}
bool same(Rect a,Rect b){return a.x==b.x&&a.y==b.y&&a.width==b.width&&a.height==b.height;}
}
ValidationPanel::ValidationPanel(ResourceAccess& r):resources_(r),font_(metrics(r)){}
void ValidationPanel::setState(ValidationPanelState s){
    if(s.rows.size()>7||!s.pageCount||s.page>=s.pageCount||!ascii(s.title))throw std::invalid_argument("Invalid validation panel page");
    if(!std::isfinite(s.entryOrigin.x)||!std::isfinite(s.entryOrigin.y))throw std::invalid_argument("Invalid validation entry position");
    for(const auto& row:s.rows)if(!ascii(row.label)||!ascii(row.value)||!ascii(row.actionLabel))throw std::invalid_argument("Validation panel requires ASCII labels");
    const bool preserve=state_.enabled==s.enabled&&state_.open==s.open&&state_.page==s.page&&state_.entryOrigin.x==s.entryOrigin.x&&state_.entryOrigin.y==s.entryOrigin.y;
    state_=std::move(s);configure(preserve);
    // An external close cancels the pressed controls but still consumes the
    // owning fingers until release, preventing a click through into the game.
    if(!preserve)captured_.reset();
}
void ValidationPanel::configure(bool preserve){
    auto old=std::move(controls_);controls_.clear();if(!state_.enabled)return;
    auto add=[&](unsigned id,std::size_t row,std::string label,Rect r,bool enabled=true){
        if(!id)return;
        Control c{id,row,std::move(label),{}};c.button.bounds=r;c.button.enabled=enabled;
        if(preserve)for(const auto& prior:old)if(prior.id==id&&prior.row==row&&same(prior.button.bounds,r)&&prior.button.enabled==enabled){c.button=prior.button;break;}
        controls_.push_back(std::move(c));
    };
    if(!state_.open){add(validationOpenCommand,0,"TEST",{state_.entryOrigin.x,state_.entryOrigin.y,34,18});return;}
    for(std::size_t i=0;i<state_.rows.size();++i){const auto& row=state_.rows[i];const float y=108+44.f*i;
        add(row.minusId,i,"-",{164,y,32,24},row.enabled);
        add(row.plusId,i,"+",{200,y,32,24},row.enabled);
        add(row.actionId,i,row.actionLabel,{236,y,48,24},row.enabled);
    }
    add(validationPreviousPageCommand,0,"PREV",{36,420,72,28},state_.page>0);
    add(validationCloseCommand,0,"CLOSE",{124,420,72,28});
    add(validationNextPageCommand,0,"NEXT",{212,420,72,28},state_.page+1<state_.pageCount);
}
void ValidationPanel::image(Frame& f,std::string_view n,Rect r,unsigned rows,unsigned tile)const{
    auto size=resources_.imageSize(n);Sprite s;s.image=n;s.sourcePx=Rect{0,size.y/rows*tile,size.x,size.y/rows};s.destination=r;f.sprites.push_back(std::move(s));
}
void ValidationPanel::text(Frame& f,std::string_view s,Vec2 p,float size,float width,Color color)const{
    if(s.empty())return;
    const auto measured=font_.width(s,size);if(measured>width&&measured>0)size*=width/measured;
    font_.draw(f,s,{p.x-font_.width(s,size)/2,p.y-size/2},size,color);
}
void ValidationPanel::append(Frame& f)const{
    if(!state_.enabled)return;
    if(state_.open){
        Sprite shade;shade.destination={0,0,f.logicalWidth,f.logicalHeight};shade.color={0,0,0,.65f};f.sprites.push_back(shade);
        image(f,"SysMenu_Mid.png",{32,48,256,384});
        image(f,"SysMenu_Top.png",{32,16,256,32});image(f,"SysMenu_Btm.png",{32,432,256,32});
        text(f,state_.title,{160,64},12,228,{140/255.f,240/255.f,1,1});
        text(f,std::to_string(state_.page+1)+" / "+std::to_string(state_.pageCount),{160,84},9,120);
        for(std::size_t i=0;i<state_.rows.size();++i){const auto& row=state_.rows[i];const float y=96+44.f*i;
            const Color color=row.enabled?Color{1,1,1,1}:Color{.5f,.5f,.5f,1};
            text(f,row.label,{160,y},9,236,color);
            text(f,row.value,{98,y+24},9,120,color);
        }
    }
    for(const auto& c:controls_){
        image(f,"SysMenu_Btn.png",c.button.bounds,2,c.button.pressed()?1:0);
        if(!c.button.enabled)f.sprites.back().color={.5f,.5f,.5f,1};
        const auto r=c.button.bounds;text(f,c.label,{r.x+r.width/2,r.y+r.height/2},state_.open?9:8,r.width-6,c.button.enabled?Color{140/255.f,240/255.f,1,1}:Color{.5f,.5f,.5f,1});
    }
}
bool ValidationPanel::touch(const Touch& t){
    const bool terminal=t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled;
    const bool tracked=swallowed_.contains(t.id);
    if(!state_.enabled){if(terminal)swallowed_.erase(t.id);return tracked;}
    if(!state_.open&&tracked&&captured_!=t.id){if(terminal)swallowed_.erase(t.id);return true;}
    if(state_.open&&t.phase==TouchPhase::began)swallowed_.insert(t.id);
    if(captured_&&*captured_!=t.id){
        const bool inside=std::any_of(controls_.begin(),controls_.end(),[&](const auto& c){return c.button.enabled&&c.button.contains(t.position);});
        if(inside&&t.phase==TouchPhase::began)swallowed_.insert(t.id);
        if(terminal)swallowed_.erase(t.id);
        return state_.open||tracked||inside;
    }
    for(auto& c:controls_){auto result=c.button.touch(t);
        if(result.action==ButtonAction::push){captured_=t.id;swallowed_.insert(t.id);return true;}
        if(result.action==ButtonAction::pop){events_.push_back({c.id,c.row});captured_.reset();swallowed_.erase(t.id);return true;}
        if(result.action==ButtonAction::cancel&&terminal){captured_.reset();swallowed_.erase(t.id);return true;}
        if(result.action==ButtonAction::release||result.action==ButtonAction::move||result.consumed)return true;
    }
    if(terminal){swallowed_.erase(t.id);if(captured_==t.id)captured_.reset();}
    return state_.open||tracked;
}
std::vector<ValidationEvent> ValidationPanel::takeEvents(){auto out=std::move(events_);events_.clear();return out;}
}
