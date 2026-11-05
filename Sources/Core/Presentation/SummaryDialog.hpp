#pragma once
#include "Core/Presentation/GameHUD.hpp"
#include "Core/Session/Settlement.hpp"
namespace md {
struct SummaryState {
    bool visible=false,cleared=false;
    unsigned mission=0,kills=0,deaths=0,built=0;
    double elapsedSeconds=0;
    StageReward reward;
};
class SummaryDialog {
public:
    // Original reward unit/attack image is assembled by the catalog owner.
    using DrawReward=std::function<void(Frame&,const StageReward&,Vec2 topLeftCenter,float)>;
    SummaryDialog(ResourceAccess&,DrawReward={});
    void setState(SummaryState);
    void append(Frame&)const;
    bool touch(const Touch&);
    std::vector<HUDAction> takeActions();
private:
    ResourceAccess& resources_;SpriteFont font_;DrawReward reward_;SummaryState state_;
    struct Control{unsigned id;std::string image;Button button;};
    std::vector<Control> controls_;std::vector<HUDAction> actions_;
    float height()const;
    void image(Frame&,std::string_view,Vec2,unsigned rows=1,unsigned tile=0,Vec2 scale={1,1})const;
    void text(Frame&,std::string_view,Vec2,float,Color={160/255.f,1,1,1})const;
};
}
