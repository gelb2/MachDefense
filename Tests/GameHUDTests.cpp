#include <array>
#include "Core/Presentation/GameHUD.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace md;
namespace {
struct Resources:ResourceAccess {
    std::filesystem::path root;std::map<std::string,Vec2> sizes;
    explicit Resources(std::filesystem::path p):root(std::move(p)){}
    Bytes read(std::string_view p)override{std::ifstream f(root/std::string(p),std::ios::binary);if(!f)throw std::runtime_error("missing "+std::string(p));return {std::istreambuf_iterator<char>(f),{}};}
    Vec2 imageSize(std::string_view n)override{
        auto key=std::string(n);if(sizes.contains(key))return sizes.at(key);auto b=read("Images/"+key);assert(b.size()>=24&&b[0]==137);
        auto u=[&](int i){return std::uint32_t(b[i])<<24|std::uint32_t(b[i+1])<<16|std::uint32_t(b[i+2])<<8|b[i+3];};return sizes[key]={float(u(16)),float(u(20))};
    }
};
void tap(GameHUD& h,float x,float y){h.touch({1,TouchPhase::began,{x,y}});h.touch({1,TouchPhase::ended,{x,y}});}
const Sprite& find(const Frame& f,std::string_view image){auto i=std::find_if(f.sprites.begin(),f.sprites.end(),[&](const auto& s){return s.image==image;});assert(i!=f.sprites.end());return *i;}
void validate(const Frame& f,Resources& r){for(const auto& s:f.sprites){assert(std::filesystem::exists(s.image.starts_with("Generated/")?r.root/s.image:r.root/"Images"/s.image));if(s.sourcePx&&!s.image.starts_with("Generated/")){auto z=r.imageSize(s.image);auto c=*s.sourcePx;assert(c.x>=0&&c.y>=0&&c.x+c.width<=z.x&&c.y+c.height<=z.y);}}}
}
int main(int argc,char** argv){
    Resources r(argc>1?argv[1]:"Resources");unsigned icons=0;
    GameHUD h(r,[&](Frame&,std::string_view name,Vec2 pos,float scale){assert(name=="MachineGun");assert(pos.x==292||pos.x==228);assert(scale==.5f);++icons;});
    GameHUDState state;state.score=1234;state.credits=9876;state.minerals=321;state.maximumMinerals=500;
    state.playerBaseHealth=.5f;state.enemyBaseHealth=.25f;
    state.production.push_back({"MachineGun","Machine Gun",137,400,2,10,3,1,true,false});
    state.queue.push_back({"MachineGun",.25f});h.setState(state);
    Frame f;h.append(f);validate(f,r);assert(icons==2);
    const auto& gauge=find(f,"BuildGauge.png");assert(gauge.destination.width==8&&gauge.destination.x==212&&gauge.destination.y==63);
    const auto& hp=find(f,"EnemyGauge.png");assert(hp.destination.x==31.5f&&hp.destination.width==40.5f);
    const auto& panel=find(f,"GameUI_MachSlot.png");assert(panel.destination.x==192&&panel.destination.y==-28);
    // Radar keeps world positions at 1/8 scale, including units outside the
    // current camera. The original +/-128 map cutoff is independent of it.
    state.radarMarks={{{0,-470},2},{{0,487},3},{{80,0},1},{{0,1025},1}};
    state.radarCamera=Vec2{-512,-92};h.setState(state);Frame radar;h.append(radar);
    unsigned marks=0;for(const auto& s:radar.sprites)if(s.image=="RaderMark.png"){
        ++marks;assert(s.destination.width==8&&s.destination.height==8);
        if(s.sourcePx->x==16){assert(s.destination.x==14);assert(s.destination.y==136.75f);}
    }
    assert(marks==3&&radar.meshes.size()==4);
    const auto initialTop=radar.meshes[1].vertices[0].position.y;
    state.radarCamera=Vec2{-212,208};h.setState(state);radar={};h.append(radar);
    assert(radar.meshes[1].vertices[0].position.y<initialTop);
    state.radarCamera.reset();state.radarMarks.clear();h.setState(state);
    // Production price 137 is present, with the original 14px text reduced to 7px.
    std::vector<unsigned> priceCodes;
    for(const auto& s:f.sprites)if(s.image.starts_with("Generated/")&&s.destination.y<40&&s.destination.y>30){const auto crop=*s.sourcePx;priceCodes.push_back(unsigned(crop.y/80)*16+unsigned(crop.x/80)+32);assert(std::abs(s.destination.width-80*7.f/48)<.0001f);}
    assert((priceCodes==std::vector<unsigned>{'1','3','7'}));
    h.touch({1,TouchPhase::began,{292,52}});Frame pressed;h.append(pressed);assert(find(pressed,"SelMach.png").blend==Blend::additive);
    h.touch({1,TouchPhase::ended,{292,52}});auto actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x40d&&actions[0].row==0);
    // Dragging out cancels; expensive production is not silently charged by the HUD.
    h.touch({2,TouchPhase::began,{292,52}});h.touch({2,TouchPhase::moved,{100,150}});h.touch({2,TouchPhase::ended,{100,150}});assert(h.takeActions().empty());assert(state.credits==9876&&state.minerals==321);
    state.production[0].enabled=false;h.setState(state);tap(h,292,52);assert(h.takeActions().empty());
    tap(h,284,446);actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x415);
    state.mainPanelOffsetY=260;h.setState(state);f={};h.append(f);
    assert(find(f,"iPon_GameUI_BG@2x.png").destination.y==94);
    tap(h,284,186);assert(h.takeActions().at(0).buttonId==0x415);
    state.mainPanelOffsetY=0;state.paused=true;h.setState(state);f={};h.append(f);validate(f,r);
    assert(find(f,"SysMenu_Mid.png").destination.height==192);
    tap(h,130,96);actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x416);
    tap(h,130,120);actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x417);
    tap(h,130,144);assert(h.takeActions().at(0).buttonId==0x419);
    tap(h,130,168);assert(h.takeActions().at(0).buttonId==0x41a);
    tap(h,198,280);assert(h.takeActions().at(0).buttonId==0x416);
    h.touch({7,TouchPhase::began,{196,225}});h.touch({7,TouchPhase::moved,{500,225}});h.touch({7,TouchPhase::ended,{500,225}});
    actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x41b&&actions[0].value==1);
    // Pausing consumes world touches and hides production hit testing.
    assert(h.touch({8,TouchPhase::began,{292,52}}));assert(h.touch({8,TouchPhase::ended,{292,52}}));assert(h.takeActions().empty());
    state.paused=false;state.selectedPanel=2;state.attacks.push_back({"AirStrike","Icon02001.png",1,10,3,240,true});state.selectedAttack=0;h.setState(state);
    f={};h.append(f);validate(f,r);assert(find(f,"Icon02001.png").destination.x==292-r.imageSize("Icon02001.png").x/4);assert(find(f,"SelMach.png").blend==Blend::additive);
    tap(h,292,52);actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x40e&&actions[0].row==0);
    tap(h,293,406);assert(h.takeActions().at(0).buttonId==0x3fe);
    h.touch({90,TouchPhase::began,{292,52}});
    actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x40e&&actions[0].row==0);
    h.touch({90,TouchPhase::ended,{292,52}});assert(h.takeActions().empty());
    state.selectedPanel=3;state.selectedFormation=4;h.setState(state);f={};h.append(f);validate(f,r);
    for(unsigned i=0;i<5;++i){tap(h,292,52+51.f*i);actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x40e&&actions[0].row==i);}
    assert(std::none_of(f.sprites.begin(),f.sprites.end(),[](const auto& s){return s.image=="Btn_UpgradeEquip.png";}));
    state.selectedPanel=1;state.upgradeMode=true;state.production[0].enabled=true;h.setState(state);
    f={};h.append(f);
    auto metricBytes=r.read("Generated/TrebuchetMS-Bold-48.tsv");SpriteFont font({reinterpret_cast<const char*>(metricBytes.data()),metricBytes.size()});
    std::vector<const Sprite*> costGlyphs;
    for(const auto& s:f.sprites)if(s.image.starts_with("Generated/")&&std::abs(s.destination.y-58.5f)<.001f)costGlyphs.push_back(&s);
    assert(costGlyphs.size()==5); // 400cr, right aligned at original x=271.
    assert(std::abs(costGlyphs.front()->destination.x-(271-font.width("400cr",6)-1))<.001f);
    tap(h,284,446);assert(h.takeActions().empty());tap(h,284,469);assert(h.takeActions().empty());
    tap(h,161,443);assert(h.takeActions().at(0).buttonId==0x40b);
    tap(h,292,52);assert(h.takeActions().at(0).buttonId==0x40d);
    state.baseUpgradeOpen=true;h.setState(state);tap(h,292,52);assert(h.takeActions().empty());tap(h,161,443);assert(h.takeActions().empty());
    tap(h,284,469);assert(h.takeActions().at(0).buttonId==0x413);
    // The layer-5 slot background must never cover layer-9 QuickBtn art.
    // Exercise each panel and the entire base-upgrade movement range.
    state.paused=false;state.upgradeMode=false;state.baseUpgradeOpen=false;
    for(unsigned panel=1;panel<=3;++panel)for(float offset:{0.f,130.f,260.f}){
        state.selectedPanel=panel;state.mainPanelOffsetY=offset;h.setState(state);f={};h.append(f);validate(f,r);
        const auto slotIndex=std::size_t(&find(f,"GameUI_MachSlot.png")-f.sprites.data());
        const auto hudIndex=std::size_t(&find(f,"iPon_GameUI_BG@2x.png")-f.sprites.data());
        assert(slotIndex<hudIndex);unsigned quickCount=0;
        for(std::size_t i=0;i<f.sprites.size();++i)if(f.sprites[i].image=="Btn_BuildMachBG.png"||f.sprites[i].image=="Btn_UpgradeEquip.png"||f.sprites[i].image=="FormationIcon_01.png")assert(i<hudIndex);
        for(std::size_t i=0;i<f.sprites.size();++i)if(f.sprites[i].image=="iPon_QuickBtn@2x.png"){
            assert(i>slotIndex);assert(f.sprites[i].sourcePx->width==r.imageSize("iPon_QuickBtn@2x.png").x/2);++quickCount;
        }
        assert(quickCount==2);
        if(offset==0){
            tap(h,284,446);assert(h.takeActions().at(0).buttonId==0x415);
            tap(h,284,469);assert(h.takeActions().at(0).buttonId==0x413);
        }
    }
    // Original MachButton right-column tiles contain the glow. Hold, cancel,
    // select and clear each lower button without creating a new effect asset.
    const auto lowerTile=[&](const Frame& frame,unsigned index){
        unsigned count=0;for(const auto& sprite:frame.sprites)if(sprite.image=="MachButton.png"){
            if(count++==index)return unsigned(sprite.sourcePx->y/64)*2+unsigned(sprite.sourcePx->x/64);
        }assert(false);return 0u;
    };
    state.mainPanelOffsetY=0;state.selectedPanel=1;h.setState(state);f={};h.append(f);
    assert(lowerTile(f,0)==1&&lowerTile(f,1)==2&&lowerTile(f,2)==6);
    for(unsigned index=1;index<=2;++index){
        const Vec2 pos{index==1?161.f:208.5f,443};
        h.touch({100+index,TouchPhase::began,pos});
        for(unsigned frame=0;frame<20;++frame){state.gameTime+=1./60;h.setState(state);f={};h.append(f);assert(lowerTile(f,index)==(index==1?3u:7u));}
        h.touch({100+index,TouchPhase::moved,{20,300}});f={};h.append(f);assert(lowerTile(f,index)==(index==1?2u:6u));
        h.touch({100+index,TouchPhase::cancelled,{20,300}});assert(h.takeActions().empty());
    }
    state.selectedPanel=2;h.setState(state);f={};h.append(f);assert(lowerTile(f,0)==0&&lowerTile(f,1)==3);
    h.touch({110,TouchPhase::began,{115.5f,443}});f={};h.append(f);assert(lowerTile(f,0)==1);
    h.touch({110,TouchPhase::ended,{115.5f,443}});actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x409);
    // Keep the original cells, production buttons and touch anchors inside
    // the safe area while covering the decorative sidebar up to screen y=0.
    for(const auto dimensions:std::vector<std::array<float,4>>{
        {420,912,62,34},{390,844,59,34},{1024,1366,24,20},
        {768,1024,24,20},{360,780,24,0},{320,480,0,0}}){
        FrameLayout layout;layout.resize(dimensions[0],dimensions[1],dimensions[2],dimensions[3]);
        h.setLayout(layout);state.paused=false;state.upgradeMode=false;
        state.baseUpgradeOpen=false;state.productionVisible=true;
        state.mainPanelOffsetY=0;state.selectedPanel=1;h.setState(state);
        f={};h.append(f);validate(f,r);
        const Vec2 topPoint{layout.width-8,1};
        bool covered=false,topExtended=false;
        for(const auto& sprite:f.sprites)if(sprite.image=="GameUI_MachSlot.png"){
            const auto d=sprite.destination;
            topExtended|=sprite.sourcePx&&sprite.sourcePx->height==2&&std::abs(d.y)<.001f;
            covered|=topPoint.x>=d.x&&topPoint.x<d.x+d.width&&topPoint.y>=d.y&&topPoint.y<d.y+d.height;
        }
        assert(covered&&"Original HUD must cover the exposed top-right strip");
        assert(topExtended==(layout.top>28)); // iPad/reference/Galaxy keep their original draw commands.
        const auto& button=find(f,"Btn_BuildMachBG.png");
        assert(std::abs(button.destination.y+button.destination.height/2-(52+layout.top))<.001f);
        tap(h,292+layout.topRight().x,52+layout.top);
        actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x40d&&actions[0].row==0);
        tap(h,287+layout.topRight().x,15+layout.top);
        actions=h.takeActions();assert(actions.size()==1&&actions[0].buttonId==0x3fd);
    }
    std::cout<<"GameHUD original assets, prices, gauges, production touch, pause, lower glow tiles and special push checks passed\n";
}
