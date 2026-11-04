#include "SessionTestSupport.hpp"
#include <sstream>
void pressSprite(GameSession& s,std::string_view name){std::optional<Vec2> p;for(const auto& x:s.frame().sprites)if(x.image==name)p=Vec2{x.destination.x+x.destination.width/2,x.destination.y+x.destination.height/2};assert(p);tap(s,p->x,p->y);ticks(s);}
void destroyBase(GameSession& s,bool player){auto* w=s.validationWorld();ActorId id=0;for(const auto& a:w->actors())if(a->base&&a->player==player)id=a->id;assert(id);w->damage(id,w->actor(id)->hp);s.tick();}
int main(int argc,char** argv){assert(argc==2);Resources r;r.root=argv[1];Host h;
    auto initial=SaveData::empty(1);assert(initial.createProfile("CAMPAIGN",1)==0);initial.writeWord(SaveData::slotStart+0xc,5);h.saved=initial.encode();
    for(unsigned stage=0;stage<80;++stage){
        // Relaunch from the actual previous stage's serialized save, then use
        // ordinary profile/mission controls. No unlock-all or mission-jump cheat.
        GameSession s(r,h,true);tap(s,160,240);ticks(s);tap(s,160,140);ticks(s);tap(s,240,450);ticks(s);
        assert(s.validationWorld()&&s.validationWorld()->stage()==stage);const auto start=SaveData::decode(h.saved);const int writes=h.writes;
        assert(start.stage(0)==stage);assert(start.clearCount(0,stage)==0);
        // Force failure only to exercise the original result/retry UI branch.
        destroyBase(s,true);assert(s.validationWorld()->state()==WorldState::failed);assert(h.writes==writes+1);
        auto failed=SaveData::decode(h.saved);assert(failed.stage(0)==stage&&failed.clearCount(0,stage)==0);assert(failed.credits(0)==start.credits(0));
        pressSprite(s,"SummaryBtn_Retry.png");assert(s.validationWorld()->state()==WorldState::running&&s.validationWorld()->stage()==stage);
        // Force enemy-base destruction: progression/settlement coverage, not
        // evidence of natural combat completion or original runtime parity.
        const auto before=*s.validationSave();destroyBase(s,false);assert(s.validationWorld()->state()==WorldState::cleared);
        auto saved=SaveData::decode(h.saved);assert(saved.clearCount(0,stage)==1);assert(saved.stage(0)==std::min(79u,stage+1));
        assert(saved.credits(0)==before.credits(0)+500+int(stage)*25);
        const int finishedWrites=h.writes;ticks(s,10);assert(h.writes==finishedWrites);checkImages(s.frame(),r);
        // Other profile slots stay empty throughout campaign progression.
        for(std::size_t slot=1;slot<5;++slot)assert(!saved.active(slot));
        if(stage<79){pressSprite(s,"SummaryBtn_NextStage.png");assert(s.validationWorld()->stage()==stage+1);}
        else{assert(has(s.frame(),"SummaryBtn_Retry.png"));pressSprite(s,"SummaryBtn_Retry.png");assert(s.validationWorld()->stage()==79);destroyBase(s,false);assert(SaveData::decode(h.saved).clearCount(0,79)==2);}
        std::cout<<"mission "<<stage+1<<": entry/fail/retry/clear/reward/unlock/save/relaunch path passed\n";
    }
    auto final=SaveData::decode(h.saved);for(unsigned i=0;i<80;++i)assert(final.clearCount(0,i)>=1);
    std::cout<<"Campaign: 80 stage state-transition/save checks passed; forced base destruction, not natural combat verification\n";
}
