#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace md {
struct Vec2 { float x=0, y=0; };
struct Rect { float x=0, y=0, width=0, height=0; };
struct Color { float r=1, g=1, b=1, a=1; };
enum class Blend { alpha, premultipliedAlpha, additive, opaque };
// Presentation coordinates have a top-left origin. World coordinates remain
// the core's responsibility. An empty image draws a solid color rectangle.
struct Sprite {
    std::string image;
    std::optional<Rect> sourcePx;
    Rect destination;
    float rotationDegrees=0;
    Vec2 pivot{.5f,.5f};
    Color color;
    Blend blend=Blend::alpha;
    bool nearest=false;
    bool flipX=false, flipY=false;
    std::optional<Rect> clip;
};
struct Frame {
    float logicalWidth=320, logicalHeight=480;
    Color background{0,0,0,1};
    std::vector<Sprite> sprites;
};
enum class TouchPhase { began, moved, ended, cancelled };
struct Touch { std::uint64_t id=0; TouchPhase phase=TouchPhase::began; Vec2 position; };
}
