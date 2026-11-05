#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <optional>
#include <vector>

namespace md {
struct OwnedBuild {std::string name;std::uint32_t level=0;bool onSlot=false;};
struct OwnedAttack {std::string name;std::uint32_t level=0,count=0;bool onSlot=false;};
// WeMade GlobalInfo slotData: 0x1b3e0, openDataFile 0x1b404, saveDataFile 0x1b5ac.
// Unknown bytes are retained; no C++ object memory is serialized.
class SaveData {
public:
    static constexpr std::size_t byteCount=0xa600, slotCount=5;
    static constexpr std::size_t slotStart=0x408, slotStride=0x2064;
    static SaveData decode(std::span<const std::uint8_t> encoded);
    static SaveData fromPlaintext(std::span<const std::uint8_t> bytes);
    static SaveData empty(std::uint32_t seed);
    std::optional<std::size_t> createProfile(std::string_view name, std::uint32_t timestamp);
    void deleteProfile(std::size_t slot);
    void selectProfile(std::size_t slot,std::uint32_t timestamp);
    std::uint32_t stage(std::size_t slot) const;
    unsigned baseUpgrade(std::size_t slot,unsigned category) const;
    void setBaseUpgrade(std::size_t slot,unsigned category,unsigned level);
    std::vector<OwnedBuild> builds(std::size_t slot) const;
    std::vector<OwnedAttack> attacks(std::size_t slot) const;
    void setBuilds(std::size_t slot,std::span<const OwnedBuild>);
    void setAttacks(std::size_t slot,std::span<const OwnedAttack>);
    int clearCount(std::size_t slot,unsigned mission) const;
    void setClearCount(std::size_t slot,unsigned mission,unsigned count);
    std::vector<std::uint8_t> encode() const;
    std::span<const std::uint8_t> plaintext() const { return bytes_; }
    bool active(std::size_t slot) const;
    std::string name(std::size_t slot) const;
    std::int32_t credits(std::size_t slot) const;
    void setCredits(std::size_t slot, std::int32_t value);
    std::uint32_t readWord(std::size_t offset) const;
    void writeWord(std::size_t offset, std::uint32_t value);
private:
    std::array<std::uint8_t,byteCount> bytes_{};
    static std::size_t slotOffset(std::size_t slot);
    void putString(std::size_t offset,std::size_t capacity,std::string_view value);
};
}
