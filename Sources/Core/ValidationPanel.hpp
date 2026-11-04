#pragma once
#include "Button.hpp"
#include "Game.hpp"
#include "SpriteFont.hpp"
#include <unordered_set>

namespace md {
inline constexpr unsigned validationOpenCommand=0xffff0001;
inline constexpr unsigned validationCloseCommand=0xffff0002;
inline constexpr unsigned validationPreviousPageCommand=0xffff0003;
inline constexpr unsigned validationNextPageCommand=0xffff0004;
struct ValidationRow {
    std::string label,value;
    unsigned minusId=0,plusId=0,actionId=0;
    bool enabled=true;
    std::string actionLabel="SET";
};
struct ValidationPanelState {
    bool enabled=false,open=false;
    std::string title="TEST MENU";
    unsigned page=0,pageCount=1;
    std::vector<ValidationRow> rows; // At most seven rows on the current page.
    Vec2 entryOrigin{2,2}; // Top-left logical coordinates; supplied per screen.
};
struct ValidationEvent {unsigned commandId=0;std::size_t row=0;};
// Cheat-only presentation. The caller owns every game value and command.
class ValidationPanel {
public:
    explicit ValidationPanel(ResourceAccess&);
    void setState(ValidationPanelState);
    void append(Frame&)const;
    bool touch(const Touch&);
    std::vector<ValidationEvent> takeEvents();
private:
    ResourceAccess& resources_;SpriteFont font_;ValidationPanelState state_;
    struct Control {unsigned id;std::size_t row;std::string label;Button button;};
    std::vector<Control> controls_;std::vector<ValidationEvent> events_;
    std::optional<std::uint64_t> captured_;
    std::unordered_set<std::uint64_t> swallowed_;
    void configure(bool preserve);
    void image(Frame&,std::string_view,Rect,unsigned rows=1,unsigned tile=0)const;
    void text(Frame&,std::string_view,Vec2,float,float width,Color={1,1,1,1})const;
};
}
