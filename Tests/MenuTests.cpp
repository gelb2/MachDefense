#include "Menu.hpp"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
using namespace md;
namespace {
struct Resources:ResourceAccess {
    std::filesystem::path root;
    explicit Resources(std::filesystem::path p):root(std::move(p)){}
    Bytes read(std::string_view p)override{
        std::ifstream file(root/std::string(p),std::ios::binary);
        if(!file)throw std::runtime_error("Missing resource: "+std::string(p));
        return {std::istreambuf_iterator<char>(file),{}};
    }
    std::map<std::string,Vec2> sizes;
    Vec2 imageSize(std::string_view n)override{
        if(const auto found=sizes.find(std::string(n));found!=sizes.end())return found->second;
        auto b=read("Images/"+std::string(n));
        if(n.ends_with(".jpg"))return sizes[std::string(n)]={640,960}; // These menu tests use the two 640x960 JPEGs.
        if(b.size()<24||b[0]!=137||b[1]!='P')throw std::runtime_error("Invalid menu PNG");
        auto u32=[&](int at){return std::uint32_t(b[at])<<24|std::uint32_t(b[at+1])<<16|std::uint32_t(b[at+2])<<8|b[at+3];};
        return sizes[std::string(n)]={float(u32(16)),float(u32(20))};
    }
};
struct Host:HostServices{
    int writes=0;bool fail=false;std::optional<std::string> name="PLAYER";
    std::size_t nameLimit=0;Bytes saved;
    std::optional<Bytes> readSave()override{return {};}
    bool writeSave(std::span<const std::uint8_t> value)override{++writes;saved.assign(value.begin(),value.end());return !fail;}
    void playSound(std::string_view,float,float)override{}
    void playMusic(std::string_view,float)override{}
    void stopMusic()override{}
    void requestName(std::string,std::size_t max,std::function<void(std::optional<std::string>)> cb)override{nameLimit=max;cb(name);}
};
void tap(Menu& m,float x,float y){m.touch({1,TouchPhase::began,{x,y}});m.touch({1,TouchPhase::ended,{x,y}});}
bool hasImage(const Frame& f,std::string_view n){return std::any_of(f.sprites.begin(),f.sprites.end(),[&](const Sprite& s){return s.image==n;});}
}
int main(int argc,char** argv){
    Resources r(argc>1?argv[1]:"Resources");Host h;auto s=SaveData::empty(123);Menu m(r,h,s);
    assert(m.screen()==Menu::Screen::title);assert(!hasImage(m.frame(),"Main_TapToStart.png"));
    for(int i=0;i<20;++i)m.tick();assert(hasImage(m.frame(),"Main_TapToStart.png"));
    for(int i=0;i<60;++i)m.tick();assert(!hasImage(m.frame(),"Main_TapToStart.png"));assert(h.writes==0);
    tap(m,160,240);assert(m.screen()==Menu::Screen::profiles);
    h.name="TOO LONG NAME";tap(m,240,450);assert(!s.active(0));assert(h.writes==0);assert(h.nameLimit==10);
    tap(m,160,295);h.name="PLAYER";tap(m,240,450);
    assert(s.active(0));assert(s.name(0)=="PLAYER");assert(h.writes==1);
    assert(SaveData::decode(h.saved).name(0)=="PLAYER");
    const auto glyph=[&](float x,float y){return std::find_if(m.frame().sprites.begin(),m.frame().sprites.end(),[&](const Sprite& sp){return sp.image=="NumSet01b.png"&&sp.destination.x==x&&sp.destination.y==y;});};
    const auto slash=glyph(90,138);assert(slash!=m.frame().sprites.end()&&slash->sourcePx->x==88);
    const auto colon=glyph(144,138);assert(colon!=m.frame().sprites.end()&&colon->sourcePx->x==96);
    const auto stageNumber=glyph(230,137);assert(stageNumber!=m.frame().sprites.end()&&stageNumber->sourcePx->x==8);

    for(int i=0;i<100;++i)m.tick();assert(h.writes==1);
    tap(m,160,140);assert(m.screen()==Menu::Screen::missions);assert(h.writes==2);
    for(int i=0;i<100;++i)m.tick();assert(h.writes==2);
    for(const auto& sprite:m.frame().sprites)if(!sprite.image.empty()){
        auto path=sprite.image.starts_with("Generated/")?r.root/sprite.image:r.root/"Images"/sprite.image;
        assert(std::filesystem::exists(path));
        if(sprite.sourcePx&&!sprite.image.starts_with("Generated/")){
            const auto size=r.imageSize(sprite.image);const auto crop=*sprite.sourcePx;
            assert(crop.x>=0&&crop.y>=0&&crop.width>0&&crop.height>0);
            assert(crop.x+crop.width<=size.x&&crop.y+crop.height<=size.y);
        }
    }
    // A locked Stage 2 row must not issue a mission request.
    const auto& f=m.frame();const auto stage=std::find_if(f.sprites.begin(),f.sprites.end(),[](const Sprite& sp){return sp.image=="SelStage_Btn_Stage.png"&&sp.sourcePx&&sp.sourcePx->y==128;});
    assert(stage!=f.sprites.end());tap(m,160,stage->destination.y+16);assert(!m.takeMissionRequest());
    tap(m,240,450);auto mission=m.takeMissionRequest();assert(mission&&mission->slot==0&&mission->mission==0);
    assert(!m.takeMissionRequest());
    m.showMissions(0);for(int i=0;i<100;++i)m.tick();
    const float initialY=m.frame().sprites[1].destination.y;
    m.touch({2,TouchPhase::began,{160,120}});m.touch({2,TouchPhase::moved,{20,160}});
    m.touch({2,TouchPhase::ended,{20,160}});assert(!m.takeMissionRequest());
    m.tick();assert(m.frame().sprites[1].destination.y!=initialY);assert(h.writes==2);
    m.showProfiles();tap(m,160,450);tap(m,208,295);assert(s.active(0));assert(h.writes==2); // Cancel deletion.
    tap(m,160,450);h.fail=true;tap(m,112,295);assert(s.active(0));assert(h.writes==3); // Failed write restores memory.
    tap(m,160,295);h.fail=false;tap(m,160,450);tap(m,112,295);assert(!s.active(0));assert(h.writes==4);
    assert(!SaveData::decode(h.saved).active(0));
    // The WeMade port requests missing CompleteMark.png. It does not draw
    // SelStage_CompleteMark.png, which the source version had substituted at
    // x=320 and clipped. Completion is represented by unlocked rows, while
    // the selected row keeps its original selection frame.
    Host parityHost;auto paritySave=SaveData::empty(456);assert(paritySave.createProfile("PARITY",456));
    for(unsigned progress:{1u,4u,80u}){
        paritySave.writeWord(SaveData::slotStart+0x38,progress);
        Menu parity(r,parityHost,paritySave);parity.showMissions(0);
        for(unsigned tick=0;tick<120;++tick)parity.tick();
        assert(!hasImage(parity.frame(),"SelStage_CompleteMark.png"));
        unsigned selected=0,unlocked=0;
        for(const auto& sprite:parity.frame().sprites)if(sprite.image=="SelStage_Btn_Stage.png"){
            if(sprite.sourcePx->y==192){++selected;assert(sprite.destination.x==32&&sprite.destination.width==256);}
            if(sprite.sourcePx->y==0)++unlocked;
        }
        assert(selected==1&&unlocked==std::min(progress+1,80u));
        tap(parity,240,450);auto selectedRequest=parity.takeMissionRequest();
        assert(selectedRequest&&selectedRequest->mission==std::min(progress,79u));
        assert(paritySave.stage(0)==progress&&parityHost.writes==0);
    }
    for(const auto dims: {Vec2{420,912},Vec2{1024,1366}}){
        Host resizedHost;auto resizedSave=SaveData::empty(789);Menu resized(r,resizedHost,resizedSave);
        FrameLayout layout;layout.resize(dims.x,dims.y,24,20);resized.setLayout(layout);
        const auto c=layout.center();const float buttonY=layout.height-layout.bottom-30;
        tap(resized,layout.width/2,layout.height/2);
        tap(resized,240+c.x,buttonY);assert(resizedSave.active(0)&&resizedHost.writes==1);
        tap(resized,160+c.x,buttonY);tap(resized,208+c.x,295+c.y);assert(resizedSave.active(0));
        tap(resized,160+c.x,140+c.y);assert(resized.screen()==Menu::Screen::missions);
        tap(resized,160+c.x,buttonY);tap(resized,160+c.x,295+c.y); // Support modal dismisses.
        tap(resized,80+c.x,buttonY);assert(resized.screen()==Menu::Screen::profiles);
        tap(resized,160+c.x,buttonY);tap(resized,112+c.x,295+c.y);assert(!resizedSave.active(0));
        assert(resizedHost.writes==3);
    }
    std::cout<<"PASS menus: original geometry resources, blink timing, name validation, create/select/delete, locked mission, save failure rollback, no tick writes, completed-port parity\n";
}
