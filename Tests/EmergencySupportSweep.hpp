#pragma once
#include "ViewportSweep.hpp"
#include "Core/Simulation/EmergencySupport.hpp"
#include <limits>
namespace md::validation {
inline std::string runEmergencySupportSweep(ResourceAccess& resources,const std::function<void(const Frame&,std::string_view)>& capture={}){
    auto require=[](bool ok,const char* text){if(!ok)throw std::runtime_error(text);};
    const auto m1=EmergencySupport::amounts(1,500,1),m17=EmergencySupport::amounts(17,1000,1.4f),m40=EmergencySupport::amounts(40,2300,1.5f),m80=EmergencySupport::amounts(80,4700,2.2f);
    require(m1[0].credits==20&&m1[1].cells==48&&m1[2].credits==10&&m1[2].cells==24,"Mission 1 budget");
    require(m17[0].credits==36&&m17[1].cells==67&&m17[2].credits==18&&m17[2].cells==33,"Mission 17 budget");
    require(m40[0].credits==59&&m40[1].cells==72&&m80[0].credits==99&&m80[1].cells==105,"Late mission budgets");
    EmergencySupport model;model.reset(1);model.advance(29.999,500,1);require(model.pending(0).empty(),"Early grant");
    model.advance(30,500,1);require(model.pending(0).credits==20&&model.remaining(0)==30,"30 second boundary");
    model.advance(44.999,500,1);require(model.pending(1).empty(),"Early cell grant");model.advance(45,500,1);require(model.pending(1).cells==48,"45 second boundary");
    model.advance(60,500,1);require(model.pending(0).credits==40&&model.pending(2).credits==10&&model.pending(2).cells==24,"60 second boundary");
    auto got=model.claim(2,100,0);require(got.credits==10&&got.cells==0&&model.pending(2).cells==24,"Mixed credit-only claim");
    got=model.claim(2,100,7);require(got.credits==0&&got.cells==7&&model.pending(2).cells==17,"Mixed partial cells repeated credit");
    got=model.claim(2,100,100);require(got.cells==17&&model.claim(2,100,100).empty(),"Double claim");
    for(unsigned seconds:{90,120,135,180,300,600,1800}){
        model.reset(1);model.advance(seconds,500,1);
        require(model.pending(0).credits==20*(seconds/30)&&model.pending(1).cells==48*(seconds/45)&&model.pending(2).credits==10*(seconds/60)&&model.pending(2).cells==24*(seconds/60),"Recurring accumulation");
    }
    model.reset(1);model.advance(45,100,1);require(model.pending(1).cells==10,"Capacity cap");model.advance(90,1000,1.4f);require(model.pending(1).cells==77,"Upgrades affect next delivery only");
    require(model.claim(0,3,0).credits==3&&model.pending(0).credits==57,"Credit integer headroom");
    model.reset(80);require(model.pending(0).empty()&&model.remaining(0)==30,"Mission restart reset");
    auto tap=[](GameSession& g,Vec2 p){g.touch({77,TouchPhase::began,p});g.touch({77,TouchPhase::ended,p});};
    auto ticks=[](GameSession& g,int n=160){while(n--)g.tick();};
    const std::array<Vec2,3> sizes{{{320,480},{420,912},{1024,1366}}};
    for(auto size:sizes){
        ViewportHost host;auto initial=SaveData::empty(123);initial.createProfile("SUPPLY",1);initial.writeWord(SaveData::slotStart+0xc,5);initial.setCredits(0,123);host.saved=initial.encode();
        GameSession g(resources,host,true);FrameLayout layout;layout.resize(size.x,size.y);g.resize(size.x,size.y);
        const auto center=layout.center(),br=layout.bottomRight();auto modal=[&](float x,float y){tap(g,{x+center.x,y+center.y});};
        modal(160,240);ticks(g);modal(160,140);ticks(g);tap(g,{240+br.x,450+br.y});ticks(g);
        auto* w=g.validationWorld();require(w&&w->state()==WorldState::running,"Battle start");
        for(const auto& a:w->actors())if(a->base)w->setValidationHealth(a->id,100000000);
        const auto writes=host.writes;const auto credits=g.validationSave()->credits(0);
        auto open=[&](){tap(g,{208.5f+br.x,443+br.y});require(w->paused(),"Support did not pause");};
        auto close=[&](){modal(160,418);};
        open();const auto time=w->time();ticks(g,300);require(w->time()==time,"Support clock advances while open");modal(160,152);require(g.validationSave()->credits(0)==credits,"Early UI claim");close();require(!w->paused(),"Support close did not resume");
        while(w->time()-w->startTime()<60.01){g.tick();require(w->state()==WorldState::running,"Test battle ended early");}
        w->setValidationCells(w->cellCapacity());g.tick();open();
        for(const auto& sprite:g.frame().sprites)if(sprite.image=="Common_Btn_CLOSE.png")require(sprite.sourcePx&&sprite.sourcePx->height==resources.imageSize(sprite.image).y/2,"Close button atlas crop");
        if(capture)capture(g.frame(),size.x==420?"support-iphone-ready":size.x==1024?"support-ipad-ready":"support-reference-ready");
        modal(160,152);require(g.validationSave()->credits(0)==credits+40,"Credit UI claim amount");modal(160,152);require(g.validationSave()->credits(0)==credits+40,"Double UI claim");
        modal(160,292);require(g.validationSave()->credits(0)==credits+50,"Mixed credit UI claim");
        require(w->cells()==w->cellCapacity(),"Cell capacity exceeded");
        w->setValidationCells(w->cellCapacity()-10);g.tick();modal(160,222);require(w->cells()==w->cellCapacity(),"Partial cell UI claim");
        w->setValidationCells(0);g.tick();modal(160,222);require(w->cells()==38,"Remaining cells lost");modal(160,292);require(w->cells()==62&&g.validationSave()->credits(0)==credits+50,"Mixed remainder wrong");
        ticks(g,300);if(capture)capture(g.frame(),size.x==420?"support-iphone-received":size.x==1024?"support-ipad-received":"support-reference-received");
        const auto pausedTime=w->time();g.setActive(false);ticks(g,100);g.setActive(true);close();require(w->paused()&&w->time()==pausedTime,"Background caused automatic resume");modal(130,96);require(!w->paused(),"Resume after support background");
        // Resize cancels an in-progress press. Its release cannot claim.
        while(w->time()-w->startTime()<90.01)g.tick();open();const auto before=g.validationSave()->credits(0);
        g.touch({77,TouchPhase::began,{160+center.x,152+center.y}});g.resize(size.x+10,size.y);g.touch({77,TouchPhase::ended,{160+center.x,152+center.y}});require(g.validationSave()->credits(0)==before,"Resize triggered stale claim");g.resize(size.x,size.y);
        modal(160,152);require(g.validationSave()->credits(0)==before+20,"Recurring credit UI delivery");close();
        require(host.writes==writes,"Support writes files during battle");
        const auto finalCredits=g.validationSave()->credits(0);tap(g,{284+br.x,435+br.y});modal(130,126); // Existing save-and-exit.
        ticks(g);require(host.writes==writes+1&&SaveData::decode(host.saved).credits(0)==finalCredits,"Claimed credits not saved at exit");
        require(host.saved.size()==SaveData::byteCount,"Save format changed");
        // Reload the same ordinary app save; normal builds expose the same support UI.
        GameSession normal(resources,host,false);tap(normal,{160,240});ticks(normal);tap(normal,{160,140});ticks(normal);tap(normal,{240,450});ticks(normal);tap(normal,{208.5f,443});
        bool dialog=false;for(const auto& sprite:normal.frame().sprites)dialog|=sprite.image=="SysMenu_Btn.png";require(dialog,"Normal app lacks support dialog");
    }
    return "Support: four mission budgets; 30/45/60 boundaries; 30-minute recurrence; capacity/research changes; partial/mixed/repeated claims; phone/pad input, pause, background, resize; zero new battle writes; original save and normal-app reload passed";
}
}
