#pragma once
#include "RenderTypes.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace md {
using Bytes=std::vector<std::uint8_t>;
class ResourceAccess {
public:
    virtual ~ResourceAccess()=default;
    // Relative to the prepared resource root; unavailable files throw.
    virtual Bytes read(std::string_view relativePath)=0;
    virtual Vec2 imageSize(std::string_view imageName)=0;
};
class HostServices {
public:
    virtual ~HostServices()=default;
    virtual std::optional<Bytes> readSave()=0;
    // Only an explicit core save point may invoke this operation.
    virtual bool writeSave(std::span<const std::uint8_t> bytes)=0;
    virtual void playSound(std::string_view name,float gain=1,float pan=0)=0;
    virtual void playMusic(std::string_view name,float gain=1)=0;
    virtual void stopMusic()=0;
    virtual void requestName(std::string initial,std::size_t maximumBytes,
                             std::function<void(std::optional<std::string>)> completion)=0;
};
class Game {
public:
    virtual ~Game()=default;
    virtual const Frame& frame() const=0;
    // One game update per scheduled callback; the host never performs catch-up ticks.
    virtual void tick()=0;
    virtual void touch(const Touch& event)=0;
    virtual void setActive(bool active)=0;
    virtual void resize(float availableWidth,float availableHeight)=0;
    virtual int preferredFramesPerSecond() const=0;
};
// Implemented by the reconstructed game, not by the platform adapter.
std::unique_ptr<Game> createGame(ResourceAccess& resources,HostServices& host,bool validation);
}
