#pragma once
#include "Core/Contracts/RenderTypes.hpp"
#include <optional>

namespace md {
enum class ButtonAction { none, push, pop, release, cancel, move };
struct ButtonResult { ButtonAction action=ButtonAction::none; bool moved=false; bool consumed=false; };
// QobButton::onTap:State:ID: (WeMade 0x400c0). Bounds use the original
// lower-exclusive/upper-inclusive comparison. Touch ownership survives a drag
// outside, allowing the same finger to press the button again on re-entry.
class Button {
public:
    Rect bounds{};
    bool enabled=true,shown=true;
    [[nodiscard]] bool pressed() const { return pressed_; }
    [[nodiscard]] bool contains(Vec2 p) const {
        return p.x>bounds.x && p.x<=bounds.x+bounds.width &&
               p.y>bounds.y && p.y<=bounds.y+bounds.height;
    }
    void reset() { held_.reset();pressed_=false; }
    ButtonResult touch(const Touch& t) {
        if(!enabled||!shown)return {};
        const bool inside=contains(t.position),mine=held_&&*held_==t.id;
        if(t.phase==TouchPhase::cancelled){if(!mine)return {};reset();return {ButtonAction::cancel,false,false};}
        if(t.phase==TouchPhase::ended&&mine){reset();return {inside?ButtonAction::pop:ButtonAction::cancel,false,false};}
        if(inside&&(t.phase==TouchPhase::began||(t.phase==TouchPhase::moved&&mine&&!pressed_))){
            held_=t.id;pressed_=true;return {ButtonAction::push,t.phase==TouchPhase::moved,true};
        }
        if(mine&&!inside&&pressed_){pressed_=false;return {ButtonAction::release,t.phase==TouchPhase::moved,false};}
        return {mine&&t.phase==TouchPhase::moved?ButtonAction::move:ButtonAction::none,mine&&t.phase==TouchPhase::moved,false};
    }
private:
    std::optional<std::uint64_t> held_;
    bool pressed_=false;
};
}
