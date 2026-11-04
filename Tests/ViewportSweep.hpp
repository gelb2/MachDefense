#pragma once
#include "Core/Session/GameSession.hpp"
#include "Core/Simulation/World.hpp"
#include "Core/Contracts/FrameLayout.hpp"
#include <functional>
#include <sstream>
#include <stdexcept>
namespace md::validation {
struct ViewportHost:HostServices {
    Bytes saved;unsigned writes=0;
    std::optional<Bytes> readSave()override{return saved;}
    bool writeSave(std::span<const std::uint8_t> b)override{saved.assign(b.begin(),b.end());++writes;return true;}
    void playSound(std::string_view,float,float)override{}
    void playMusic(std::string_view,float)override{}
    void stopMusic()override{}
    void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{throw std::runtime_error("Unexpected name dialog");}
};
inline std::string runViewportSweep(ResourceAccess& r,const std::function<void(const Frame&,std::string_view)>& capture={}){
    auto require=[](bool ok,const char* what){if(!ok)throw std::runtime_error(what);};
    auto ticks=[](GameSession& g,int n=160){while(n--)g.tick();};
    auto tap=[](GameSession& g,Vec2 p){g.touch({91,TouchPhase::began,p});g.touch({91,TouchPhase::ended,p});};
    auto fixture=SaveData::empty(123);fixture.createProfile("VIEWPORT",1);fixture.writeWord(SaveData::slotStart+0xc,5);
    fixture.setCredits(0,20000);
    fixture.setBuilds(0,std::vector<OwnedBuild>{{"RifleTurret",0,true},{"MachineGun",0,true}});
    struct Size {float w,h,top,bottom;const char* name;};
    const Size sizes[]={{320,480,0,0,"reference"},{390,844,59,34,"iphone"},{420,912,62,34,"iphone-air"},{1024,1366,24,20,"ipad-air"},{768,1024,24,20,"ipad"}};
    std::vector<std::tuple<ActorId,Vec2,int>> reference;Bytes referenceSave;unsigned referenceRandom=0;
    unsigned conditions=0;
    for(const auto size:sizes){
        ViewportHost host;host.saved=fixture.encode();GameSession game(r,host,true);
        FrameLayout layout;layout.resize(size.w,size.h,size.top,size.bottom);
        const auto menuCenter=layout.center();
        game.resize(size.w,size.h,size.top,size.bottom);
        require(std::abs(game.frame().logicalWidth/game.frame().logicalHeight-size.w/size.h)<.0001f,"Title still letterboxed");
        if(capture)capture(game.frame(),std::string(size.name)+"-title");
        tap(game,{160+menuCenter.x,240+menuCenter.y});ticks(game);
        require(std::any_of(game.frame().sprites.begin(),game.frame().sprites.end(),[](const Sprite& s){return s.image=="Common_Btn_NEW.png";}),"Expanded title input missed");
        if(capture)capture(game.frame(),std::string(size.name)+"-profiles");
        tap(game,{160+menuCenter.x,140+menuCenter.y});ticks(game);
        require(std::any_of(game.frame().sprites.begin(),game.frame().sprites.end(),[](const Sprite& s){return s.image=="iPon_SelStage_Fore@2x.png";}),"Expanded profile input missed");
        if(capture)capture(game.frame(),std::string(size.name)+"-missions");
        tap(game,{240+menuCenter.x,layout.height-layout.bottom-30});ticks(game);
        auto* world=game.validationWorld();require(world!=nullptr,"World did not open");
        const auto before=game.validationSave()->encode();const auto writes=host.writes;
        game.resize(size.w,size.h,size.top,size.bottom);
        require(std::abs(game.frame().logicalWidth/game.frame().logicalHeight-size.w/size.h)<.0001f,"Frame aspect does not fill display");
        require(game.validationSave()->encode()==before&&host.writes==writes,"Resize changed save");
        // Tall phones retain scale 1; wider layouts fit terrain and units together.
        const auto a=world->worldToScreen({0,-400}),b=world->worldToScreen({20,-380});
        const float expectedScale=(layout.width-64)/256;
        require(std::abs(b.x-a.x-20*expectedScale)<.001f&&std::abs(b.y-a.y+20*expectedScale)<.001f,"World presentation scale mismatch");
        for(const Vec2 p:{Vec2{-100,-400},Vec2{100,400},Vec2{0,0}}){const auto back=world->screenToWorld(world->worldToScreen(p));require(std::abs(back.x-p.x)<.001f&&std::abs(back.y-p.y)<.001f,"World input inverse mismatch");}
        const auto terrain=std::find_if(game.frame().sprites.begin(),game.frame().sprites.end(),[](const auto& s){return s.image.ends_with("_01.jpg");});
        require(terrain!=game.frame().sprites.end(),"Missing terrain");
        require(std::abs(terrain->destination.x)<.001f&&std::abs(terrain->destination.width-(layout.width-64))<.001f,"Terrain leaves a gap before production HUD");
        if(capture)capture(game.frame(),std::string(size.name)+"-bottom");
        ticks(game,600);
        std::vector<std::tuple<ActorId,Vec2,int>> snapshot;for(const auto& a:world->actors())snapshot.emplace_back(a->id,a->position,a->hp);
        const auto random=world->randomSource().next();
        auto comparable=*game.validationSave();comparable.writeWord(SaveData::slotStart+4,1); // Profile selection records wall-clock time.
        if(reference.empty()){reference=snapshot;referenceSave=comparable.encode();referenceRandom=random;}
        else {require(snapshot.size()==reference.size(),"Viewport changed spawn count");for(std::size_t i=0;i<snapshot.size();++i){auto [id,p,hp]=snapshot[i];auto [rid,rp,rhp]=reference[i];require(id==rid&&hp==rhp&&p.x==rp.x&&p.y==rp.y,"Viewport changed combat state");}require(referenceSave==comparable.encode()&&random==referenceRandom,"Viewport changed progress or random stream");}
        const auto br=layout.bottomRight(),tr=layout.topRight(),center=layout.center();
        tap(game,{284+br.x,435+br.y});require(world->paused(),"Moved menu button missed");
        tap(game,{130+center.x,96+center.y});require(!world->paused(),"Moved resume button missed");
        tap(game,{284+br.x,469+br.y});ticks(game,60);require(world->paused(),"Moved base upgrade button missed");
        const auto findSprite=[&](std::string_view name)->const Sprite&{for(const auto& sprite:game.frame().sprites)if(sprite.image==name)return sprite;throw std::runtime_error("Missing dialog sprite");};
        const auto& firstBaseRow=findSprite("Btn_BaseUpgrade_BaseCannon.png");
        const auto& panel=findSprite("iPon_GameUI_BG@2x.png");
        require(std::abs(firstBaseRow.destination.y+firstBaseRow.destination.height/2-panel.destination.y-157)<.01f,"Base upgrade rows drift from sliding HUD");
        require(std::abs(firstBaseRow.destination.x+firstBaseRow.destination.width/2-(196+br.x))<.01f,"Base upgrade rows use wrong horizontal anchor");
        // The metal backing must cover every x at the bottom when the drawer opens.
        if(layout.width!=320||layout.height!=480)for(float x=0;x<layout.width;x+=8){bool covered=false;for(const auto& sprite:game.frame().sprites)if(sprite.image=="GameUI_BtmL.png"||sprite.image=="GameUI_BtmR.png"){const auto r=sprite.destination;covered|=x>=r.x&&x<r.x+r.width&&r.y+r.height>=layout.height-.01f;}require(covered,"Open base drawer leaves bottom or middle gap");}
        if(capture)capture(game.frame(),std::string(size.name)+"-base-upgrade");
        const auto previousLevel=game.validationSave()->baseUpgrade(0,0);const auto previousCredits=game.validationSave()->credits(0);
        tap(game,{firstBaseRow.destination.x+firstBaseRow.destination.width/2,firstBaseRow.destination.y+firstBaseRow.destination.height/2});
        const auto buy=findSprite("Btn_UpgradeBase_Upgrade.png").destination;
        tap(game,{buy.x+buy.width/2,buy.y+buy.height/2});
        require(game.validationSave()->baseUpgrade(0,0)==previousLevel+1&&game.validationSave()->credits(0)<previousCredits,"Moved base upgrade purchase failed");
        bool closed=false;for(const auto& s:game.frame().sprites)if(s.image=="Btn_UpgradeBase_Close.png"){tap(game,{s.destination.x+s.destination.width/2,s.destination.y+s.destination.height/2});closed=true;break;}
        require(closed,"Missing base dialog close");ticks(game,60);require(!world->paused(),"Moved base dialog close missed");
        world->setValidationCells(500);game.tick();tap(game,{292+tr.x,52+tr.y});
        const Vec2 target{50,-430};const auto screen=world->worldToScreen(target);tap(game,screen);require(world->productionCount()==1,"Expanded world placement failed");
        ticks(game,1500);bool placed=false;for(const auto& a:world->actors())if(a->player&&!a->removed&&a->name=="Tur_Gun_01"){placed=true;require(std::abs(a->position.x-target.x)<=4&&std::abs(a->position.y-target.y)<=4,"Turret placed at wrong world point");}if(!placed){std::ostringstream detail;detail<<"Placed turret missing: "<<size.name<<" queue="<<world->productionCount();for(const auto& a:world->actors())if(a->player)detail<<" "<<a->name<<" type="<<a->type<<" hp="<<a->hp;throw std::runtime_error(detail.str());}
        // Radar reaches the top with the same physical world bounds.
        const Vec2 radar{18,layout.top+30};tap(game,radar);ticks(game,60);
        if(capture)capture(game.frame(),std::string(size.name)+"-top");
        game.setActive(false);game.setActive(true);require(world->paused(),"Foreground did not pause");tap(game,{130+center.x,96+center.y});require(!world->paused(),"Resume after foreground failed");
        require(host.writes==writes,"Viewport actions wrote unexpected save");++conditions;
    }
    return "Viewport: "+std::to_string(conditions)+" aspect/safe-area cases; 600-tick combat/save/RNG identity; relocated HUD, modal, turret placement, radar and foreground passed";
}
}
