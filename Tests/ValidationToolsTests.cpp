#include "SessionTestSupport.hpp"
#include "Core/Validation/ValidationTools.hpp"
int main(int argc,char** argv){assert(argc==2);Resources r;r.root=argv[1];Host h;GameSession ordinary(r,h,false);
    assert(!ordinary.applyValidationCommand(unsigned(ValidationCommand::newProfile)));assert(!ordinary.validationSave());assert(h.writes==0);
    GameSession s(r,h,true);auto command=[&](ValidationCommand c){assert(s.applyValidationCommand(unsigned(c)));};
    // Reach the test controls through the actual sprite button.
    tap(s,19,11);assert(has(s.frame(),"SysMenu_Mid.png"));
    tap(s,260,164);auto save=s.validationSave();assert(save&&save->active(0)&&save->name(0)=="TEST");assert(h.writes==0);
    tap(s,216,252);assert(s.validationSave()->credits(0)==1000); // Credits + actual button
    tap(s,260,384);assert(h.writes==1&&SaveData::decode(h.saved).credits(0)==1000);
    command(ValidationCommand::startMission);ticks(s);auto* w=s.validationWorld();assert(w&&w->stage()==0);
    tap(s,160,450);assert(!w->paused());
    tap(s,19,179);const double now=w->time();ticks(s,10);assert(w->paused()&&w->time()==now);
    command(ValidationCommand::cellsMinus);assert(w->cells()>=0);
    for(int i=0;i<20;++i)command(ValidationCommand::cellsMinus);assert(w->cells()==0);
    command(ValidationCommand::fillCells);assert(w->cells()==w->cellCapacity());
    command(ValidationCommand::creditMinus);assert(s.validationSave()->credits(0)==0);
    command(ValidationCommand::creditMinus);assert(s.validationSave()->credits(0)==0);
    for(unsigned category=0;category<6;++category){s.applyValidationCommand(110+category);assert(s.validationSave()->baseUpgrade(0,category)==1);s.applyValidationCommand(100+category);assert(s.validationSave()->baseUpgrade(0,category)==0);}
    // Select all catalog entries, grant, upgrade, and enforce the seven-slot limit.
    for(int i=0;i<40;++i){command(ValidationCommand::buildGrant);command(ValidationCommand::buildLevelPlus);command(ValidationCommand::buildInstall);command(ValidationCommand::buildNext);}
    auto owned=s.validationSave()->builds(0);assert(owned.size()>4);assert(std::count_if(owned.begin(),owned.end(),[](const auto& x){return x.onSlot;})<=7);
    for(int i=0;i<5;++i){command(ValidationCommand::attackGrant);command(ValidationCommand::attackLevelPlus);command(ValidationCommand::attackCountPlus);command(ValidationCommand::attackInstall);command(ValidationCommand::attackNext);}
    auto special=s.validationSave()->attacks(0);assert(special.size()==5);
    const auto before=w->actors().size();command(ValidationCommand::spawnMachine);assert(w->actors().size()==before+1);
    command(ValidationCommand::healthMinus);command(ValidationCommand::healthFull);assert(h.writes==1); // No implicit file writes.
    command(ValidationCommand::save);assert(h.writes==2);auto decoded=SaveData::decode(h.saved);assert(decoded.builds(0).size()==owned.size());assert(decoded.attacks(0).size()==5);
    command(ValidationCommand::win);assert(w->state()==WorldState::cleared&&h.writes==3);assert(!has(s.frame(),"SysMenu_Mid.png"));
    assert(s.validationSave()->clearCount(0,0)==1);checkImages(s.frame(),r);
    // A diagnostic command issued without opening TEST must affect the
    // actual battle profile, including slots other than the first one.
    Host second;auto profiles=SaveData::empty(2);profiles.createProfile("FIRST",1);profiles.createProfile("SECOND",2);profiles.setCredits(0,17);profiles.setCredits(1,29);second.saved=profiles.encode();
    GameSession secondSession(r,second,true);tap(secondSession,160,240);ticks(secondSession);tap(secondSession,160,220);ticks(secondSession);tap(secondSession,240,450);ticks(secondSession);
    assert(secondSession.validationWorld());secondSession.applyValidationCommand(unsigned(ValidationCommand::creditPlus));auto two=*secondSession.validationSave();assert(two.credits(0)==17&&two.credits(1)==1029);
    std::cout<<"Validation controls: ordinary gating, real UI, pause, bounded inventories, runtime values, explicit save and clear passed\n";
}
