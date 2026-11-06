#pragma once
#include "Core/Contracts/Game.hpp"
#include "Core/Session/SaveData.hpp"
namespace md {
class World;
class GameSession final:public Game {
public:
    GameSession(ResourceAccess&,HostServices&,bool validation);
    ~GameSession()override;
    const Frame& frame()const override;
    void tick()override;
    void touch(const Touch&)override;
    bool scrollProduction(Vec2 point,float delta);
    void setActive(bool)override;
    void resize(float,float,float safeTop=0,float safeBottom=0)override;
    int preferredFramesPerSecond()const override{return 60;}
    World* validationWorld();
    bool applyValidationCommand(unsigned);
    std::optional<SaveData> validationSave()const;
private:
    struct State;std::unique_ptr<State> state_;
};
}
