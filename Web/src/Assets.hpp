#pragma once
#include "Core/Contracts/Game.hpp"
#include "Audio/PCM.hpp"
#include <map>
namespace md::web {
class Assets:public ResourceAccess {
 std::map<std::string,Vec2> sizes;
public:
 Bytes read(std::string_view) override;
 Vec2 imageSize(std::string_view) override;
 static std::string imagePath(std::string_view n){return n.starts_with("Generated/")?std::string(n):"Images/"+std::string(n);}
};
audio::PCM decodeWav(std::span<const std::uint8_t>);
}
