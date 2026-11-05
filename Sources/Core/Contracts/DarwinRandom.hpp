#pragma once
#include <array>
#include <bit>
#include <cstdint>

namespace md {
// Original Darwin random()/srandom() sequence used by EncTbl and saveDataFile.
// Local reference: Compatibility/check-random.py; no host libc/global RNG state.
class DarwinRandom {
    std::array<std::uint32_t, 31> state_{};
    unsigned front_ = 3, rear_ = 0;
public:
    explicit DarwinRandom(std::uint32_t seed = 1) {
        state_[0] = seed;
        for (unsigned i=1; i<state_.size(); ++i) {
            std::int64_t v = std::bit_cast<std::int32_t>(state_[i-1]);
            if (v == 0) v = 123459876;
            v = 16807*(v%127773) - 2836*(v/127773);
            if (v < 0) v += 2147483647;
            state_[i] = static_cast<std::uint32_t>(v);
        }
        for (unsigned i=0; i<310; ++i) next();
    }
    std::uint32_t next() {
        state_[front_] += state_[rear_];
        const auto value = state_[front_] >> 1;
        front_ = (front_+1)%31; rear_ = (rear_+1)%31;
        return value;
    }
};
}
