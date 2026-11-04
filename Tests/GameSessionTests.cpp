#include "SessionTestSupport.hpp"
#include "ViewportSweep.hpp"
int main(int argc,char** argv){assert(argc==2);Resources r;r.root=argv[1];Host h;GameSession s(r,h,true);
    assert(has(s.frame(),"Title.jpg"));tap(s,160,240);ticks(s);assert(has(s.frame(),"iPon_SelSlot_BG@2x.jpg"));
    tap(s,240,450);assert(h.writes==1&&SaveData::decode(h.saved).name(0)=="PLAYER");
    tap(s,160,140);ticks(s);assert(has(s.frame(),"iPon_SelStage_Fore@2x.png"));assert(h.writes==2);
    tap(s,240,450);ticks(s);auto* world=s.validationWorld();assert(world&&world->stage()==0);assert(has(s.frame(),"iPon_HelpScrMain_us@2x.jpg"));
    tap(s,160,450);assert(!has(s.frame(),"iPon_HelpScrMain_us@2x.jpg"));assert(!world->paused());checkImages(s.frame(),r);
    // Original production previews face up; world actors retain their own aim.
    unsigned previewParts=0;
    for(const auto& sprite:s.frame().sprites){const auto& b=sprite.destination;
        if(b.x>272&&b.x+b.width<318&&b.y>35&&b.y+b.height<70&&std::abs(sprite.rotationDegrees+90)<.001f)++previewParts;
    }
    assert(previewParts>0);
    // The original radar draws both bases before any production or combat.
    assert(has(s.frame(),"RaderMark.png"));
    assert(std::count_if(s.frame().meshes.begin(),s.frame().meshes.end(),[](const auto& m){return m.image=="MovePath.png";})==4);
    const auto initial=world->actors().size();tap(s,292,52);ticks(s,220);assert(world->actors().size()>initial);assert(h.writes==2);
    // Menu pause, inactive transition and resume all stop world time.
    tap(s,284,435);assert(world->paused());const auto time=world->time();ticks(s,20);assert(world->time()==time);
    tap(s,130,96);assert(!world->paused());s.setActive(false);ticks(s,20);assert(world->time()==time);s.setActive(true);assert(world->paused());tap(s,130,96);assert(!world->paused());
    tap(s,284,469);ticks(s,60);assert(world->paused());assert(has(s.frame(),"Btn_BaseUpgrade_BaseCannon.png"));checkImages(s.frame(),r);
    // Native save is unchanged during battle, pause and upgrade inspection.
    assert(h.writes==2);
    // Close the base dialog through its original close control.
    const auto& frame=s.frame();bool closed=false;for(const auto& sprite:frame.sprites)if(sprite.image=="Btn_UpgradeBase_Close.png"){tap(s,sprite.destination.x+sprite.destination.width/2,sprite.destination.y+sprite.destination.height/2);closed=true;break;}
    assert(closed);ticks(s,60);assert(!world->paused());
    // Closing either equipment dialog removes its sprites and resumes play.
    for(const bool specials:{false,true}){
        if(specials)tap(s,160,445);
        tap(s,294,403);assert(world->paused());assert(has(s.frame(),"CommonUI_Upgrade.png"));
        tap(s,specials?220:192,353.75f);assert(!world->paused());
        assert(!has(s.frame(),"CommonUI_Upgrade.png"));
        const auto resumedAt=world->time();ticks(s,2);assert(world->time()>resumedAt);
        assert(h.writes==2);
    }
    ActorId enemy=0;for(const auto& a:world->actors())if(a->base&&!a->player)enemy=a->id;
    world->damage(enemy,100000000);s.tick();assert(h.writes==3);checkImages(s.frame(),r);
    auto saved=SaveData::decode(h.saved);assert(saved.stage(0)==1&&saved.clearCount(0,0)==1&&saved.credits(0)>=500);
    ticks(s,10);assert(h.writes==3);
    // Locate the original next-stage sprite rather than guessing its position.
    std::optional<Vec2> next;for(const auto& sprite:s.frame().sprites)if(sprite.image.find("NEXT")!=std::string::npos)next=Vec2{sprite.destination.x+sprite.destination.width/2,sprite.destination.y+sprite.destination.height/2};
    if(!next){for(const auto& sprite:s.frame().sprites)if(sprite.image.find("Next")!=std::string::npos)next=Vec2{sprite.destination.x+sprite.destination.width/2,sprite.destination.y+sprite.destination.height/2};}
    const auto loadingSounds=h.soundCalls["Title_TitleIn.wav"];
    assert(next);tap(s,next->x,next->y);assert(h.soundCalls["Title_TitleIn.wav"]==loadingSounds+1);assert(has(s.frame(),"StageClear01.png"));assert(world->paused());
    ticks(s,40);const auto art=std::find_if(s.frame().sprites.begin(),s.frame().sprites.end(),[](const auto& x){return x.image=="StageClear01.png";});
    assert(art!=s.frame().sprites.end()&&art->color.a>.8f&&art->destination.width>=320&&art->destination.height>=480);
    ticks(s,160);assert(!has(s.frame(),"StageClear01.png"));assert(s.validationWorld()->stage()==1);assert(!s.validationWorld()->paused());assert(h.writes==3);assert(h.soundCalls["Title_TitleIn.wav"]==loadingSounds+1);
    // A returning profile keeps its accumulated score visible in combat.
    Host returning;auto progress=SaveData::empty(123);progress.createProfile("RETURN",1);
    progress.writeWord(SaveData::slotStart+0x2054,274);progress.writeWord(SaveData::slotStart+0xc,5);returning.saved=progress.encode();
    GameSession resumed(r,returning,true);tap(resumed,160,240);ticks(resumed);tap(resumed,160,140);ticks(resumed);tap(resumed,240,450);ticks(resumed);
    std::string score;const auto digitWidth=r.imageSize("NumSet_Cr.png").x/16;
    for(const auto& glyph:resumed.frame().sprites)if(glyph.image=="NumSet_Cr.png"&&glyph.destination.y<430&&glyph.destination.y>415)
        score+=char('0'+std::lround(glyph.sourcePx->x/digitWidth));
    assert(score=="274");assert(returning.writes==1);
    // Production availability and the original placement instruction are visible
    // through the same buttons used by ordinary play, without changing credits.
    Host placementHost;auto placementSave=SaveData::empty(123);placementSave.createProfile("PLACE",1);
    placementSave.writeWord(SaveData::slotStart+0xc,5);
    const std::vector<OwnedBuild> placementBuilds{{"RifleTurret",0,true},{"MachineGun",0,true}};
    placementSave.setBuilds(0,placementBuilds);placementHost.saved=placementSave.encode();
    GameSession placementGame(r,placementHost,true);tap(placementGame,160,240);ticks(placementGame);tap(placementGame,160,140);ticks(placementGame);tap(placementGame,240,450);ticks(placementGame);
    auto* placementWorld=placementGame.validationWorld();assert(placementWorld);
    placementWorld->setValidationCells(199);ticks(placementGame,1);
    auto covers=[&](){return std::count_if(placementGame.frame().sprites.begin(),placementGame.frame().sprites.end(),[](const auto& x){return x.image=="BuildMach_Cover.png";});};
    assert(covers()>=1);tap(placementGame,292,52);assert(!has(placementGame.frame(),"MessageBox.png"));assert(placementWorld->productionRows().empty());
    placementWorld->setValidationCells(500);ticks(placementGame,1);assert(covers()==0);
    const auto cash=placementGame.validationSave()->credits(0);tap(placementGame,292,52);
    assert(has(placementGame.frame(),"MessageBox.png"));assert(!placementWorld->paused());
    ticks(placementGame,40);const auto banner=std::find_if(placementGame.frame().sprites.begin(),placementGame.frame().sprites.end(),[](const auto& x){return x.image=="MessageBox.png";});
    assert(banner!=placementGame.frame().sprites.end()&&std::abs(banner->destination.y+banner->destination.height/2-24)<.01f);
    const auto cells=placementWorld->cells();tap(placementGame,150,260);assert(placementWorld->productionRows().size()==1);assert(std::abs(placementWorld->cells()-(cells-200))<.01f);
    assert(placementGame.validationSave()->credits(0)==cash);assert(placementHost.writes==1);
    ticks(placementGame,1);unsigned shrinkingGlyphs=0;for(const auto& glyph:placementGame.frame().sprites)
        if(glyph.image.starts_with("Generated/")&&glyph.destination.y<30&&glyph.destination.height<12)++shrinkingGlyphs;
    assert(shrinkingGlyphs>0);ticks(placementGame,80);assert(!has(placementGame.frame(),"MessageBox.png"));
    // The same turret is dimmed at its one-item build limit, even with funds.
    placementWorld->setValidationCells(500);ticks(placementGame,1);assert(covers()>=1);
    tap(placementGame,294,403);assert(has(placementGame.frame(),"CommonUI_Upgrade.png"));assert(covers()==0);
    std::cout<<md::validation::runViewportSweep(r)<<"\n";
    std::cout<<"GameSession: original menu/door/profile/help/production/pause/upgrade/result/next-stage, exact save points passed\n";
}
