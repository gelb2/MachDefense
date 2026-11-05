#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace md {
// QChunk serialized tree: size/type/dataSize, bytes, child count, children.
// The shipping MCB files contain stale size values; children define traversal.
struct Chunk {
    std::uint32_t declaredSize = 0, type = 0;
    std::size_t sourceOffset = 0, consumedSize = 0;
    std::vector<std::uint8_t> data;
    std::vector<Chunk> children;
    static Chunk parse(std::span<const std::uint8_t> bytes);
    [[nodiscard]] std::string text(std::size_t offset, std::size_t capacity) const;
    [[nodiscard]] std::uint8_t byte(std::size_t offset) const;
    [[nodiscard]] std::uint32_t uint32(std::size_t offset) const;
    [[nodiscard]] float real32(std::size_t offset) const;
};
}
