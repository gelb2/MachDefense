#include "Chunk.hpp"
#include <bit>
#include <stdexcept>

namespace md {
namespace {
std::uint32_t word(std::span<const std::uint8_t> bytes, std::size_t offset) {
    if (offset > bytes.size() || bytes.size()-offset < 4) throw std::runtime_error("Truncated QChunk word");
    return std::uint32_t(bytes[offset]) | std::uint32_t(bytes[offset+1])<<8 |
           std::uint32_t(bytes[offset+2])<<16 | std::uint32_t(bytes[offset+3])<<24;
}
Chunk read(std::span<const std::uint8_t> bytes, std::size_t& pos, unsigned depth) {
    if (depth > 128) throw std::runtime_error("QChunk nesting exceeds 128");
    Chunk c; c.sourceOffset = pos;
    c.declaredSize = word(bytes,pos); c.type = word(bytes,pos+4);
    auto length = word(bytes,pos+8); pos += 12;
    if (pos > bytes.size() || length > bytes.size()-pos) throw std::runtime_error("Truncated QChunk data");
    c.data.assign(bytes.begin()+pos,bytes.begin()+pos+length); pos += length;
    auto count=word(bytes,pos); pos+=4;
    if (count > (bytes.size()-pos)/16) throw std::runtime_error("Truncated QChunk children");
    c.children.reserve(count);
    for (std::uint32_t i=0;i<count;++i) c.children.push_back(read(bytes,pos,depth+1));
    c.consumedSize=pos-c.sourceOffset; return c;
}
}
Chunk Chunk::parse(std::span<const std::uint8_t> bytes) {
    std::size_t pos=0; auto root=read(bytes,pos,0);
    if (pos!=bytes.size()) throw std::runtime_error("Trailing bytes after QChunk root");
    return root;
}
std::string Chunk::text(std::size_t offset,std::size_t capacity) const {
    if (offset>data.size() || capacity>data.size()-offset) throw std::runtime_error("QChunk string outside data");
    std::size_t size=0; while(size<capacity && data[offset+size]) ++size;
    return std::string(reinterpret_cast<const char*>(data.data()+offset),size);
}
std::uint8_t Chunk::byte(std::size_t offset) const { return data.at(offset); }
std::uint32_t Chunk::uint32(std::size_t offset) const { return word(data,offset); }
float Chunk::real32(std::size_t offset) const { return std::bit_cast<float>(uint32(offset)); }
}
