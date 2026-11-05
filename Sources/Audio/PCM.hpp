#pragma once
#include <cstdint>
#include <vector>
namespace md::audio {
// Original AudioFile payload. No resampling, channel folding or 8->16 conversion.
struct PCM {
    std::vector<std::uint8_t> bytes;
    unsigned sampleRate=0,channels=0,bitsPerSample=0;
};
}
