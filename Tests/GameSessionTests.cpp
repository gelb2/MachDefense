#include "SessionTestSupport.hpp"
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
    ActorId enemy=0;for(const auto& a:world->actors())if(a->base&&!a->player)enemy=a->id;
    world->damage(enemy,100000000);s.tick();assert(h.writes==3);checkImages(s.frame(),r);
    auto saved=SaveData::decode(h.saved);assert(saved.stage(0)==1&&saved.clearCount(0,0)==1&&saved.credits(0)>=500);
    ticks(s,10);assert(h.writes==3);
    // Locate the original next-stage sprite rather than guessing its position.
    std::optional<Vec2> next;for(const auto& sprite:s.frame().sprites)if(sprite.image.find("NEXT")!=std::string::npos)next=Vec2{sprite.destination.x+sprite.destination.width/2,sprite.destination.y+sprite.destination.height/2};
    if(!next){for(const auto& sprite:s.frame().sprites)if(sprite.image.find("Next")!=std::string::npos)next=Vec2{sprite.destination.x+sprite.destination.width/2,sprite.destination.y+sprite.destination.height/2};}
    assert(next);tap(s,next->x,next->y);ticks(s);assert(s.validationWorld()->stage()==1);assert(h.writes==3);
    std::cout<<"GameSession: original menu/door/profile/help/production/pause/upgrade/result/next-stage, exact save points passed\n";
}
