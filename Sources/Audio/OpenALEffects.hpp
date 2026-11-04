#pragma once
#include "Audio/PCM.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
namespace md::audio {
// The platform owns file decoding and audio focus. This object owns the same
// per-resource buffer/source reuse used by SoundManager in the original game.
class OpenALEffects {
public:
    OpenALEffects();
    ~OpenALEffects();
    OpenALEffects(const OpenALEffects&)=delete;
    OpenALEffects& operator=(const OpenALEffects&)=delete;
    bool valid()const;
    bool contains(std::string_view name)const;
    bool prepare(std::string name,const PCM&);
    void play(std::string_view name,float gain,float x=0,float y=0);
    void stop();
    void setActive(bool);
    std::size_t loaded()const;
    std::uint64_t playCount()const;
    std::uintptr_t currentContextIdentity()const;
    const std::string& error()const;
private:
    struct Impl;std::unique_ptr<Impl> impl_;
};
}
