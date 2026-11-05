#include "SessionTestSupport.hpp"

namespace {
struct FailingHost:Host {
    bool fail=false;unsigned attempts=0;
    bool writeSave(std::span<const std::uint8_t> bytes)override{
        ++attempts;if(fail)return false;
        return Host::writeSave(bytes);
    }
};
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void pressImage(GameSession& session,std::string_view name){
    std::optional<Vec2> point;
    for(const auto& sprite:session.frame().sprites)if(sprite.image==name)point=Vec2{sprite.destination.x+sprite.destination.width/2,sprite.destination.y+sprite.destination.height/2};
    require(point.has_value(),"Expected original button is missing");tap(session,point->x,point->y);
}
void openProfiles(GameSession& session){tap(session,160,240);ticks(session);require(has(session.frame(),"iPon_SelSlot_BG@2x.jpg"),"Profile screen did not open");}
void openBattle(GameSession& session){openProfiles(session);tap(session,160,140);ticks(session);tap(session,240,450);ticks(session);if(has(session.frame(),"iPon_HelpScrMain_us@2x.jpg"))tap(session,160,450);require(session.validationWorld()!=nullptr,"Battle did not open");}
Bytes initialSave(){auto save=SaveData::empty(17);save.createProfile("EXISTING",1);save.setCredits(0,10000);save.writeWord(SaveData::slotStart+0xc,5);return save.encode();}
}
int main(int argc,char** argv){try{
    require(argc==2,"Pass Resources directory");Resources resources;resources.root=argv[1];std::vector<std::string> failures;
    {
        // The host models the platform's failed atomic replacement: the prior
        // persisted byte sequence remains readable. No real user file is used.
        FailingHost host;host.saved=initialSave();const auto original=host.saved;GameSession session(resources,host,true);openProfiles(session);
        host.fail=true;tap(session,240,450);require(host.attempts==1&&host.saved==original,"Create failure replaced old save");
        require(!session.validationSave()->active(1)&&session.validationSave()->name(0)=="EXISTING","Failed profile creation not rolled back");require(has(session.frame(),"MessageBox.png"),"Create failure message missing");
        tap(session,160,140);ticks(session,5);require(host.attempts==1&&!session.validationWorld(),"Failure dialog allowed profile click through");
        pressImage(session,"Common_Btn_OK.png");host.fail=false;tap(session,240,450);
        require(host.attempts==2&&host.writes==1&&SaveData::decode(host.saved).name(1)=="PLAYER","Create retry did not persist");
        const auto twoProfiles=host.saved;host.fail=true;tap(session,160,140);tap(session,160,140);ticks(session);
        require(host.saved==twoProfiles&&host.attempts==3&&has(session.frame(),"MessageBox.png"),"Select failure changed save or left menu");
        pressImage(session,"Common_Btn_OK.png");host.fail=false;tap(session,160,140);ticks(session);require(has(session.frame(),"iPon_SelStage_Fore@2x.png"),"Select retry did not advance");
        pressImage(session,"Common_Btn_BACK.png");ticks(session);
        const auto beforeDelete=host.saved;host.fail=true;tap(session,160,450);pressImage(session,"Common_Btn_YES.png");
        require(host.saved==beforeDelete&&session.validationSave()->active(0)&&session.validationSave()->active(1),"Delete failure lost a profile");
        pressImage(session,"Common_Btn_OK.png");host.fail=false;tap(session,160,450);pressImage(session,"Common_Btn_YES.png");
        const auto afterDelete=SaveData::decode(host.saved);unsigned active=0;for(std::size_t i=0;i<5;++i)active+=afterDelete.active(i);require(active==1,"Delete retry did not persist exactly one deletion");
        GameSession relaunched(resources,host,true);require(std::ranges::equal(relaunched.validationSave()->plaintext(),afterDelete.plaintext()),"Relaunch did not load recovered profile state");
    }
    {
        FailingHost host;host.saved=initialSave();GameSession session(resources,host,true);openBattle(session);auto* world=session.validationWorld();
        tap(session,284,469);ticks(session,60);pressImage(session,"Btn_BaseUpgrade_BaseCannon.png");pressImage(session,"Btn_UpgradeBase_Upgrade.png");
        require(session.validationSave()->baseUpgrade(0,0)==1,"Upgrade fixture did not use real buttons");const auto credits=session.validationSave()->credits(0);
        pressImage(session,"Btn_UpgradeBase_Close.png");ticks(session,60);tap(session,284,435);require(world->paused(),"Pause failed");
        const auto previous=host.saved;const auto attempts=host.attempts;host.fail=true;tap(session,130,120);
        require(host.saved==previous&&host.attempts==attempts+1&&has(session.frame(),"MessageBox.png"),"Save and exit failure not surfaced");
        const auto time=world->time();ticks(session,180);require(session.validationWorld()==world&&world->time()==time,"Failed save exited or resumed battle");
        require(session.validationSave()->baseUpgrade(0,0)==1&&session.validationSave()->credits(0)==credits,"Failed save discarded unsaved upgrade");
        pressImage(session,"Common_Btn_OK.png");host.fail=false;tap(session,130,120);ticks(session);
        require(has(session.frame(),"Title.jpg")&&!session.validationWorld(),"Successful save retry did not exit");
        auto saved=SaveData::decode(host.saved);require(saved.baseUpgrade(0,0)==1&&saved.credits(0)==credits,"Successful retry lost upgrade or credits");
        GameSession relaunched(resources,host,true);openBattle(relaunched);require(relaunched.validationSave()->baseUpgrade(0,0)==1&&relaunched.validationSave()->credits(0)==credits,"Upgrade lost after relaunch");
    }
    for(const std::string_view resultButton:{"SummaryBtn_NextStage.png","SummaryBtn_Retry.png","SummaryBtn_SelectStage.png"}){
        FailingHost host;host.saved=initialSave();GameSession session(resources,host,true);openBattle(session);const auto previous=host.saved;host.fail=true;
        const bool cleared=resultButton!="SummaryBtn_Retry.png";
        auto* world=session.validationWorld();ActorId enemy=0;for(const auto& actor:world->actors())if(actor->base&&actor->player==!cleared)enemy=actor->id;
        // Force only the end-of-stage branch; combat difficulty is not tested.
        world->damage(enemy,world->actor(enemy)->hp);session.tick();require(host.saved==previous&&has(session.frame(),"MessageBox.png"),"Result failure did not preserve old save");
        const auto rewardCredits=session.validationSave()->credits(0);require(session.validationSave()->clearCount(0,0)==int(cleared),"In-memory result missing");
        pressImage(session,"Common_Btn_OK.png");const auto failedAttempts=host.attempts;pressImage(session,resultButton);ticks(session);
        require(host.attempts==failedAttempts+1&&host.saved==previous&&session.validationWorld()==world&&world->state()==(cleared?WorldState::cleared:WorldState::failed)&&has(session.frame(),"MessageBox.png"),"Repeated result save failure allowed transition");
        require(session.validationSave()->clearCount(0,0)==int(cleared)&&session.validationSave()->credits(0)==rewardCredits,"Save retry duplicated settlement");
        pressImage(session,"Common_Btn_OK.png");const auto attempts=host.attempts;host.fail=false;pressImage(session,resultButton);ticks(session);
        if(host.attempts==attempts)failures.push_back("Result Next Stage did not retry failed result save");
        else {
        auto saved=SaveData::decode(host.saved);require(saved.clearCount(0,0)==int(cleared)&&saved.stage(0)==unsigned(cleared)&&saved.credits(0)==rewardCredits,"Retried result missing or reward duplicated");
        GameSession relaunched(resources,host,true);openBattle(relaunched);require(relaunched.validationWorld()->stage()==unsigned(cleared),"Result progress lost after relaunch");
        }
    }
    {
        FailingHost host;host.saved=initialSave();GameSession session(resources,host,true);
        tap(session,19,11);require(has(session.frame(),"SysMenu_Mid.png"),"TEST panel did not open");
        const auto original=host.saved;const auto credits=session.validationSave()->credits(0);host.fail=true;
        tap(session,260,384);require(host.saved==original&&host.attempts==1&&has(session.frame(),"MessageBox.png"),"TEST save failure missing");
        std::size_t message=0,panel=0;for(std::size_t i=0;i<session.frame().sprites.size();++i){const auto& sprite=session.frame().sprites[i];if(sprite.image=="MessageBox.png")message=i;if(sprite.image=="SysMenu_Btm.png")panel=i;}
        if(message<panel)failures.push_back("TEST save error is drawn behind the TEST panel");
        tap(session,216,252);if(session.validationSave()->credits(0)!=credits)failures.push_back("TEST controls accept input through save error modal");
        pressImage(session,"Common_Btn_OK.png");
        if(has(session.frame(),"MessageBox.png"))failures.push_back("TEST panel consumes save error OK button");
        else {host.fail=false;tap(session,260,384);require(host.writes==1&&SaveData::decode(host.saved).credits(0)==credits,"TEST retry failed or modal click changed data");}
    }
    if(!failures.empty()){std::string message;for(const auto& failure:failures)message+=failure+"; ";throw std::runtime_error(message);}
    std::cout<<"Save failures: create/select/delete rollback, modal input, battle save-and-exit retry, result retry, preserved bytes and relaunch passed\n";
}catch(const std::exception& e){std::cerr<<"SaveFailure: "<<e.what()<<'\n';return 1;}}
