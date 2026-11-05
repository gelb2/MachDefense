#pragma once
#include "Core/Contracts/Game.hpp"
#include "Audio/PCM.hpp"
#include <android/asset_manager.h>
#include <map>
namespace md::android {
class Assets final:public ResourceAccess {
    AAssetManager* manager_;std::map<std::string,Vec2> sizes_;
public:
    explicit Assets(AAssetManager* manager):manager_(manager){}
    Bytes read(std::string_view)override;
    Vec2 imageSize(std::string_view)override;
    std::vector<std::string> list(std::string_view);
    static std::string imagePath(std::string_view);
};
audio::PCM decodeWav(std::span<const std::uint8_t>);
}
