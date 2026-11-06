#pragma once
#include "Core/Contracts/Game.hpp"
#include "Core/Session/SaveData.hpp"
#include "Core/Presentation/SpriteFont.hpp"
#include "Core/Presentation/Button.hpp"
#include "Core/Contracts/DarwinRandom.hpp"
#include "Core/Contracts/FrameLayout.hpp"
#include <array>
#include <functional>
#include <optional>

namespace md {
struct MissionRequest { std::size_t slot=0; unsigned mission=0; };
class Menu {
public:
    enum class Screen { title, profiles, missions };
    Menu(ResourceAccess&,HostServices&,SaveData&,bool deferredTransitions=false);
    void setProfilePreparation(std::function<void(std::size_t)> prepare){prepareProfile_=std::move(prepare);}
    void tick();
    void setLayout(const FrameLayout&);
    void touch(const Touch&);
    const Frame& frame() const { return frame_; }
    std::optional<MissionRequest> takeMissionRequest();
    void showProfiles();
    void showMissions(std::size_t slot);
    std::optional<Screen> takeScreenRequest();
    void applyScreen(Screen);
    Screen screen() const { return screen_; }
    std::size_t selectedProfile() const { return slot_; }
private:
    enum class Dialog { none, deleteProfile, full, saveFailed, support, invalidName, restoreConfirm, restoreError, restoreInterrupted, restored };
    ResourceAccess& resources_;
    HostServices& host_;
    SaveData& save_;
    std::function<void(std::size_t)> prepareProfile_;
    SpriteFont font_;
    DataTable maps_;
    Frame frame_;
    FrameLayout layout_;
    void expandFrame();
    Screen screen_=Screen::title;
    bool deferredTransitions_=false;
    std::optional<Screen> requestedScreen_;
    Dialog dialog_=Dialog::none;
    std::size_t slot_=0;
    unsigned selectedMission_=0;
    DarwinRandom titleRandom_{1};
    float titleTime_=0;
    bool titleVisible_=false,titleCycleSound_=false;
    float cam_=0,baseY_=0,velocity_=0,lastDragY_=0;
    bool namePending_=false;
    std::optional<std::uint64_t> dragId_;
    std::optional<MissionRequest> mission_;
    std::array<Button,5> profiles_{};
    std::array<Button,80> stages_{};
    std::array<Button,3> bottom_{};
    std::array<Button,2> transfer_{};
    std::optional<SaveData> restoreCandidate_;
    std::string transferError_;
    void beginTransfer(bool exporting);
    void closeTransfer();
    void drawTransfer();
    Rect transferBounds() const;
    bool transferOpen_=false,transferExport_=false;
    std::array<Button,2> transferActions_{};
    std::array<Button,2> dialogButtons_{};
    void rebuild();
    void resetInput();
    void selectProfile(std::size_t);
    void beginMission();
    bool persist();
    void newProfile();
    void sprite(std::string image,Vec2 center,float width,float height,unsigned tile=0,unsigned splitY=1);
    void text(std::string_view,Vec2 center,float width,float height,float size,Color color={},int align=0);
    void number(std::string_view,Vec2 position,bool right=false);
    void bottom(unsigned index,std::string image,unsigned splitY=2);
    void drawDialog();
};
}
