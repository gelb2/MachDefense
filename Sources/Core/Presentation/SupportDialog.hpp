#pragma once
#include "Core/Presentation/GameHUD.hpp"
#include "Core/Simulation/EmergencySupport.hpp"
namespace md {
class SupportDialog {
public:
    explicit SupportDialog(ResourceAccess&);
    void open();void close();bool visible()const{return visible_;}
    void update(const EmergencySupport&,std::int64_t creditRoom,std::int64_t cellRoom);
    void received(std::size_t row);
    void tick();void append(Frame&)const;
    bool touch(const Touch&);
    // 0..2 claim, 3 close. Session owns resource changes.
    std::vector<unsigned> takeActions();
private:
    ResourceAccess& resources_;SpriteFont font_;bool visible_=false;
    std::array<Button,4> buttons_;
    std::array<SupportAmount,3> pending_{},available_{};
    std::array<unsigned,3> seconds_{};
    std::array<std::string,3> rowLabels_,claimLabels_,countdownLabels_;
    bool labelsReady_=false;
    std::vector<unsigned> actions_;
    std::vector<std::string> lines_;unsigned typeTicks_=0;
    void message(std::string_view);
    void sprite(Frame&,std::string_view,Rect,unsigned rows=1,unsigned tile=0)const;
    void text(Frame&,std::string_view,Vec2,float,Color={.94f,1,1,1})const;
};
}
