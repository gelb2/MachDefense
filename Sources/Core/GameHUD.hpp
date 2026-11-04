#pragma once
#include "Button.hpp"
#include "Game.hpp"
#include "SpriteFont.hpp"
#include <array>
#include <functional>

namespace md {
// Values come from the world/save tables. HUD does not charge credits, build
// units, advance a queue, or persist data.
struct ProductionRow {
    std::string machine, englishName;
    unsigned mineralCost=0, upgradeCost=0, level=0, maximumLevel=0;
    unsigned capacity=0, occupied=0;
    bool enabled=true, upgrading=false;
};
struct SpecialAttackRow {
    std::string name,icon;
    unsigned level=0,maximumLevel=0,count=0,upgradeCost=0;
    bool upgradeAvailable=false;
};
struct ProductionQueueRow { std::string machine; float progress=0; };
struct RadarMark { Vec2 worldPosition; unsigned tile=0; };
struct GameHUDState {
    unsigned score=0, credits=0, missionNumber=1, minerals=0, maximumMinerals=0;
    unsigned cellUpgradeLevel=0,cellUpgradeCost=0;
    float mainPanelOffsetY=0; // 0 normal, 260 at original phone base-upgrade endpoint.
    float playerBaseHealth=1, enemyBaseHealth=1, musicVolume=.8f, effectsVolume=.8f;
    bool paused=false, productionVisible=true, upgradeMode=false, cellUpgradeAvailable=false;
    bool baseUpgradeOpen=false; // Original turnOffButtons(0x413).
    unsigned selectedPanel=1; // GuiGame::setUiType: 1 build, 2 special, 3 formation
    std::vector<ProductionRow> production;
    std::vector<ProductionQueueRow> queue;
    std::vector<SpecialAttackRow> attacks; // Installed entries, in original saved order.
    std::vector<RadarMark> radarMarks;
    std::optional<Vec2> radarCamera; // World-space bottom and top.
    std::optional<std::size_t> selectedAttack,selectedFormation;
};
struct HUDAction {
    unsigned buttonId=0; // Original notification ID, never a platform UI action.
    std::size_t row=0;
    float value=0;
};
class GameHUD {
public:
    // The caller draws the original assembled uiMach at a top-left screen
    // center, with deviceScale=.5. No new silhouette/icon is substituted.
    using DrawMachine=std::function<void(Frame&,std::string_view,Vec2,float)>;
    explicit GameHUD(ResourceAccess&,DrawMachine={});
    void setState(GameHUDState);
    void append(Frame&) const;
    bool touch(const Touch&);
    std::vector<HUDAction> takeActions();
private:
    ResourceAccess& resources_;
    SpriteFont font_;
    DrawMachine machine_;
    GameHUDState state_;
    struct Control { unsigned id; std::size_t row; Button button; };
    std::vector<Control> controls_;
    std::vector<HUDAction> actions_;
    std::optional<std::pair<std::uint64_t,unsigned>> slider_;
    void configure();
    bool pressed(unsigned,std::size_t=0) const;
    void image(Frame&,std::string_view,Vec2,unsigned columns=1,unsigned rows=1,unsigned tile=0,Vec2 scale={1,1}) const;
    void number(Frame&,std::string_view,std::string_view,Vec2,float pitch,float align) const;
    void text(Frame&,std::string_view,Vec2,float,Color={1,1,1,1}) const;
    void pause(Frame&) const;
};
}
