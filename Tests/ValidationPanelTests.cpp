#include "ValidationPanel.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace md;
namespace {
struct Resources final:ResourceAccess {
    std::filesystem::path root;std::map<std::string,Vec2> sizes;
    explicit Resources(std::filesystem::path p):root(std::move(p)){}
    Bytes read(std::string_view path)override{std::ifstream f(root/std::string(path),std::ios::binary);if(!f)throw std::runtime_error("Missing test resource");return {std::istreambuf_iterator<char>(f),{}};}
    Vec2 imageSize(std::string_view name)override{
        auto key=std::string(name);if(sizes.contains(key))return sizes[key];auto bytes=read("Images/"+key);assert(bytes.size()>24&&bytes[0]==137);
        auto u=[&](unsigned i){return std::uint32_t(bytes[i])<<24|std::uint32_t(bytes[i+1])<<16|std::uint32_t(bytes[i+2])<<8|bytes[i+3];};return sizes[key]={float(u(16)),float(u(20))};
    }
};
void tap(ValidationPanel& p,Vec2 at,std::uint64_t id=1){assert(p.touch({id,TouchPhase::began,at}));assert(p.touch({id,TouchPhase::ended,at}));}
void event(ValidationPanel& p,unsigned id,std::size_t row=0){auto events=p.takeEvents();assert(events.size()==1&&events[0].commandId==id&&events[0].row==row);}
void validate(const Frame& f,Resources& r){for(const auto& s:f.sprites){
    assert(s.destination.width>=0&&s.destination.height>=0);
    if(s.image.empty())continue;
    const auto path=s.image.starts_with("Generated/")?r.root/s.image:r.root/"Images"/s.image;assert(std::filesystem::exists(path));
    if(s.sourcePx&&!s.image.starts_with("Generated/")){const auto size=r.imageSize(s.image);const auto c=*s.sourcePx;assert(c.x>=0&&c.y>=0&&c.x+c.width<=size.x&&c.y+c.height<=size.y);}
}}
}
int main(int argc,char** argv){
    Resources r(argc>1?argv[1]:"Resources");ValidationPanel p(r);Frame f;
    p.append(f);assert(f.sprites.empty());assert(!p.touch({1,TouchPhase::began,{12,10}}));
    ValidationPanelState s;s.enabled=true;s.pageCount=5;s.rows={{"Mission","1 / 80",100,101,102,true,"GO"},{"Credits","125000",110,111,0,true,""},{"Victory","Ready",0,0,120,false,"WIN"}};p.setState(s);
    assert(!p.touch({2,TouchPhase::began,{200,200}}));tap(p,{12,10});event(p,validationOpenCommand);
    // Pointer leaving the entry button cannot activate it or reach the game.
    assert(p.touch({3,TouchPhase::began,{12,10}}));assert(p.touch({3,TouchPhase::moved,{100,100}}));assert(p.touch({3,TouchPhase::ended,{100,100}}));assert(p.takeEvents().empty());
    assert(p.touch({20,TouchPhase::began,{12,10}}));assert(p.touch({21,TouchPhase::began,{12,10}}));
    assert(p.touch({21,TouchPhase::ended,{12,10}}));assert(p.takeEvents().empty());
    assert(p.touch({20,TouchPhase::ended,{12,10}}));event(p,validationOpenCommand);
    s.open=true;p.setState(s);f={};p.append(f);validate(f,r);
    assert(p.touch({4,TouchPhase::began,{1,479}}));assert(p.touch({4,TouchPhase::ended,{1,479}}));assert(p.takeEvents().empty());
    tap(p,{180,120});event(p,100);tap(p,{216,120});event(p,101);tap(p,{260,120});event(p,102);
    tap(p,{216,164});event(p,111,1);tap(p,{260,208});assert(p.takeEvents().empty());
    // A refresh of the displayed value does not drop an active press.
    assert(p.touch({5,TouchPhase::began,{216,120}}));s.rows[0].value="2 / 80";p.setState(s);assert(p.touch({5,TouchPhase::ended,{216,120}}));event(p,101);
    // Drag cancellation follows the original QobButton procedure.
    assert(p.touch({6,TouchPhase::began,{260,120}}));assert(p.touch({6,TouchPhase::moved,{10,120}}));assert(p.touch({6,TouchPhase::ended,{10,120}}));assert(p.takeEvents().empty());
    tap(p,{72,434});assert(p.takeEvents().empty());tap(p,{248,434});event(p,validationNextPageCommand);
    s.page=4;p.setState(s);tap(p,{248,434});assert(p.takeEvents().empty());tap(p,{72,434});event(p,validationPreviousPageCommand);
    // A second modal touch held while closing cannot fall through afterward.
    assert(p.touch({7,TouchPhase::began,{1,479}}));tap(p,{160,434},8);event(p,validationCloseCommand);s.open=false;p.setState(s);
    assert(p.touch({7,TouchPhase::ended,{160,200}}));assert(!p.touch({9,TouchPhase::ended,{160,200}}));assert(p.takeEvents().empty());
    s.open=true;p.setState(s);assert(p.touch({10,TouchPhase::began,{216,120}}));s.open=false;p.setState(s);assert(p.touch({10,TouchPhase::ended,{216,120}}));assert(p.takeEvents().empty());
    s.entryOrigin={2,170};p.setState(s);assert(!p.touch({11,TouchPhase::began,{12,10}}));tap(p,{12,178});event(p,validationOpenCommand);
    // Disabled regular builds expose neither the entry nor a modal.
    s.enabled=false;s.open=true;p.setState(s);f={};p.append(f);assert(f.sprites.empty());assert(!p.touch({12,TouchPhase::began,{180,120}}));
    bool rejected=false;s.rows.resize(8);try{p.setState(s);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
    s.rows.resize(7);s.enabled=true;s.open=true;s.page=0;p.setState(s);f={};p.append(f);validate(f,r);
    std::cout<<"ValidationPanel entry, modal capture, commands, disabled rows, pagination, drag cancellation and original asset bounds passed\n";
}
