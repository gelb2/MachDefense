#pragma once
#include "Core/Contracts/Game.hpp"
#include <array>
#include <optional>

namespace md {
// GameMain start/tick/changeScreen: WeMade 0x6028/0x67e4/0x768c.
// Positions keep the original bottom-left coordinate system until append().
class Transition {
public:
    explicit Transition(ResourceAccess&,HostServices&,bool originalIPadLayout=false);
    bool request(unsigned targetScreen);
    // A returned screen must replace the underlying screen in this tick,
    // while the four doors are closed (original refreshScreen, state 4).
    std::optional<unsigned> tick();
    void append(Frame&) const;
    bool visible() const { return visible_; }
    bool canChangeScreen() const { return state_>6; }
    unsigned state() const { return state_; }
    const std::array<Vec2,4>& positions() const { return positions_; }
    float centerScale() const { return centerScale_; }
private:
    ResourceAccess& resources_;
    HostServices& host_;
    bool ipad_=false,visible_=false;
    unsigned state_=8,target_=1;
    std::array<Vec2,4> positions_{}; // top, bottom, left, right
    std::array<float,4> start_{},destination_{};
    float checkX_=80,centerScale_=1.5f;
    static float approach(float from,float to,float divisor);
};
}
