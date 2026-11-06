#pragma once
#include "Core/Presentation/Button.hpp"
#include "Core/Contracts/Game.hpp"
#include "Core/Presentation/SpriteFont.hpp"
#include "Core/Contracts/FrameLayout.hpp"
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
    unsigned level=0,maximumLevel=0,count=0;
    std::uint64_t upgradeCost=0;
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
    bool supportAvailable=false;
    bool baseUpgradeOpen=false; // Original turnOffButtons(0x413).
    unsigned selectedPanel=1; // GuiGame::setUiType: 1 build, 2 special, 3 formation
    std::vector<ProductionRow> production;
    std::vector<ProductionQueueRow> queue;
    std::vector<SpecialAttackRow> attacks; // Installed entries, in original saved order.
    std::vector<RadarMark> radarMarks;
    std::optional<Vec2> radarCamera; // World-space bottom and top.
    std::optional<std::size_t> selectedAttack,selectedFormation;
    std::string instruction;
    double gameTime=0;
};
struct HUDAction {
    unsigned buttonId=0; // Original notification ID, never a platform UI action.
    std::size_t row=0;
    float value=0;
    std::string machine{}; // Production identity at touch-up; session verifies it.
};
class GameHUD {
public:
    // The caller draws the original assembled uiMach at a top-left screen
    // center, with deviceScale=.5. No new silhouette/icon is substituted.
    using DrawMachine=std::function<void(Frame&,std::string_view,Vec2,float)>;
    explicit GameHUD(ResourceAccess&,DrawMachine={});
    void setState(GameHUDState);
    void setLayout(FrameLayout);
    void append(Frame&) const;
    bool touch(const Touch&);
    bool scrollProduction(Vec2 position,float deltaY);
    [[nodiscard]] float productionScrollOffset() const { return productionScroll_; }
    [[nodiscard]] float maximumProductionScroll() const;
    [[nodiscard]] Rect productionViewport() const;
    std::vector<HUDAction> takeActions();
private:
    FrameLayout layout_;
    Vec2 controlOffset(unsigned)const;
    ResourceAccess& resources_;
    SpriteFont font_;
    DrawMachine machine_;
    GameHUDState state_;
    std::string instruction_;
    double instructionStart_=0;
    bool hideInstruction_=false;
    Vec2 instructionScale_{1,1};
    struct Control { unsigned id; std::size_t row; Button button; };
    std::vector<Control> controls_;
    std::vector<HUDAction> actions_;
    std::optional<std::pair<std::uint64_t,unsigned>> slider_;
    float productionScroll_=0;
    struct ProductionGesture {
        std::uint64_t id=0;
        Vec2 origin;
        float initialScroll=0;
        std::optional<std::size_t> row;
        std::string machine;
        unsigned level=0;
        bool dragged=false,cancelled=false,tapAllowed=true;
    };
    std::optional<ProductionGesture> productionGesture_;
    std::vector<std::uint64_t> ignoredProductionTouches_;
    void cancelProductionGesture();
    void setProductionScroll(float);
    bool productionInteractive() const;
    void configure();
    bool pressed(unsigned,std::size_t=0) const;
    void image(Frame&,std::string_view,Vec2,unsigned columns=1,unsigned rows=1,unsigned tile=0,Vec2 scale={1,1}) const;
    void number(Frame&,std::string_view,std::string_view,Vec2,float pitch,float align) const;
    void text(Frame&,std::string_view,Vec2,float,Color={1,1,1,1}) const;
    void pause(Frame&) const;
};
}
