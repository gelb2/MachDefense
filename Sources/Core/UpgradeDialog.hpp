#pragma once
#include "GameHUD.hpp"
namespace md {
struct BaseUpgradeRow {
    unsigned level=0,maximumLevel=0,cost=0;
    bool enabled=false;
    // Original displayed stat (Atk/Def/Size), supplied by the world/catalog.
    std::string stat;
};
struct UpgradeUnit {
    std::string name,englishName;
    unsigned level=0,maximumLevel=0,cost=0;
    bool installed=false,canUpgrade=false,canInstall=false,hasNext=false;
    std::array<int,4> stats{},nextStats{}; // Atk,Rng,Def,Spd as original GobHvMach accessors.
};
struct UpgradeAttack {
    std::string name,icon,nextIcon;
    unsigned level=0,maximumLevel=0,count=0,buyCost=0,upgradeCost=0;
    bool installed=false,canUpgrade=false,canBuy=false,canInstall=false,hasNext=false;
};
struct UpgradeDialogState {
    enum class Kind { hidden,base,units,attacks } kind=Kind::hidden;
    std::array<BaseUpgradeRow,6> base;
    std::vector<UpgradeUnit> units;
    std::optional<std::size_t> selectedUnit;
    std::vector<UpgradeAttack> attacks;
    std::optional<std::size_t> selectedAttack;
    // Base root follows the original opening gameUI animation.
    Vec2 baseCenter{160,224};
};
class UpgradeDialog {
public:
    using DrawUnit=std::function<void(Frame&,const UpgradeUnit&,bool next,Vec2 topLeftCenter,float)>;
    using DrawBase=std::function<void(Frame&,const std::array<BaseUpgradeRow,6>&,Vec2 topLeftCenter,float)>;
    UpgradeDialog(ResourceAccess&,DrawUnit={},DrawBase={});
    void setState(UpgradeDialogState);
    void append(Frame&)const;
    bool touch(const Touch&);
    std::vector<HUDAction> takeActions();
private:
    ResourceAccess& resources_;SpriteFont bold_,regular_;DrawUnit unit_;DrawBase base_;
    UpgradeDialogState state_;int selectedBase_=-1;
    struct Control{unsigned id;std::size_t row;Button button;};
    std::vector<Control> controls_;std::vector<HUDAction> actions_;
    void configure();
    void image(Frame&,std::string_view,Vec2,unsigned columns=1,unsigned rows=1,unsigned tile=0,Vec2 scale={1,1})const;
    void text(Frame&,std::string_view,Vec2,float,float width,int align=1,bool regular=false,Color color={240/255.f,1,1,1})const;
    bool pressed(unsigned,std::size_t=0)const;
    void attacks(Frame&)const;void attackDetail(Frame&,std::size_t)const;
    void base(Frame&)const;void units(Frame&)const;void detail(Frame&,std::size_t)const;
};
}
