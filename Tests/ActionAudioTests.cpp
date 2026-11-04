#include "SessionTestSupport.hpp"
#include <functional>

static void tapImage(GameSession& game,std::string_view name) {
    for(const auto& sprite:game.frame().sprites)if(sprite.image==name){
        const auto b=sprite.destination;tap(game,b.x+b.width/2,b.y+b.height/2);return;
    }
    throw std::runtime_error("Missing control: "+std::string(name));
}
int main(int argc,char** argv) {
    assert(argc==2);Resources r;r.root=argv[1];Host h;
    auto save=SaveData::empty(123);save.createProfile("SOUND",1);save.setCredits(0,1000000);
    save.writeWord(SaveData::slotStart+0xc,5);h.saved=save.encode();
    GameSession game(r,h,true);tap(game,160,240);ticks(game);tap(game,160,140);ticks(game);tap(game,240,450);ticks(game);
    auto* world=game.validationWorld();assert(world);h.soundCalls.clear();
    auto expect=[&](std::string_view cue,const std::function<void()>& run){
        const auto n=h.soundCalls[std::string(cue)];run();assert(h.soundCalls[std::string(cue)]==n+1);
    };
    // Panel-specific cues replace the blanket confirm sound.
    expect("UIType_SpAttack.wav",[&]{tap(game,161,443);});
    expect("UIType_BuildMach.wav",[&]{tap(game,115.5f,443);});
    assert(h.soundCalls["UI_Btn_OK.wav"]==0);
    // Accepted queue item sounds once, rejected queue does not sound.
    world->setValidationCells(10000);game.tick();
    expect("BuildMach.wav",[&]{tap(game,292,52);});
    const auto starts=h.soundCalls["BuildMach.wav"];world->setValidationCells(0);game.tick();tap(game,292,52);assert(h.soundCalls["BuildMach.wav"]==starts);
    // Base upgrade gets its own cue after a successful purchase.
    tap(game,284,469);ticks(game,60);tapImage(game,"Btn_BaseUpgrade_BaseCannon.png");
    expect("UpgradeBase.wav",[&]{tapImage(game,"Btn_UpgradeBase_Upgrade.png");});
    tapImage(game,"Btn_UpgradeBase_Close.png");ticks(game,60);
    // Unit upgrade and slot installation cues use the original dedicated sounds.
    tap(game,294,403);tap(game,44,68);
    expect("UpgradeMach.wav",[&]{tapImage(game,"UpgradeBtn_Upgrade.png");});
    expect("Uninstall.wav",[&]{tapImage(game,"UpgradeBtn_Uninstall.png");});
    expect("Install.wav",[&]{tapImage(game,"UpgradeBtn_Install.png");});
    tap(game,192,353.75f);assert(!world->paused());
    tap(game,161,443);tap(game,294,403);tap(game,72,68);
    expect("BuySpAttack.wav",[&]{tapImage(game,"UpgradeBtn_Buy.png");});
    expect("UpgradeSpAttack.wav",[&]{tapImage(game,"UpgradeBtn_Upgrade.png");});
    expect("Uninstall.wav",[&]{tapImage(game,"UpgradeBtn_Uninstall.png");});
    expect("Install.wav",[&]{tapImage(game,"UpgradeBtn_Install.png");});
    assert(h.soundCalls["UI_Btn_OK.wav"]==0);
    assert(h.writes==1); // Audio feedback does not persist transient actions.
    {
        struct PositionedHost:Host {
            Vec2 position{};unsigned positioned=0;
            void playSoundAt(std::string_view cue,Vec2 p,float gain)override {position=p;++positioned;playSound(cue,gain,0);}
        } positioned;
        auto state=SaveData::empty(1);state.createProfile("POSITION",1);
        World w(r,positioned,state,0,BattleGeometry::phone(),123);w.start(0);
        const auto center=w.screenToWorld({160,240});
        const auto id=w.validationSpawn("E_Tank_01",false,{center.x+100,center.y+400});
        w.damage(id,1);assert(positioned.positioned==1);
        assert(positioned.position.x==1&&positioned.position.y==2);
        w.actor(id)->position.x=center.x-100;w.damage(id,1);
        assert(std::isnan(positioned.position.x)&&positioned.position.y==2);
        assert(positioned.writes==0);
    }
    std::cout<<"ActionAudio: panels, queue acceptance, base/unit/special upgrade and slot cues passed\n";
}
