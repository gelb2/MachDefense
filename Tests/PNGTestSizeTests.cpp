#include "PNGTestSize.hpp"
#include <cassert>
#include <vector>

int main() {
    const std::vector<std::uint8_t> png{
        137,80,78,71,13,10,26,10,
        0,0,0,13,73,72,68,82,0,0,0,37,0,0,0,19,8,6,0,0,0,0,0,0,0
    };
    auto cgbi=png;
    cgbi.insert(cgbi.begin()+8,{0,0,0,4,67,103,66,73,0,0,0,0,0,0,0,0});
    for(const auto& image:{png,cgbi}) {
        const auto size=mdtest::pngSize(image);assert(size.x==37 && size.y==19);
        auto truncated=image;truncated.pop_back();bool failed=false;
        try {mdtest::pngSize(truncated);}catch(const std::runtime_error&){failed=true;}
        assert(failed);
    }
    const std::vector<std::uint8_t> jpeg{255,216,255,192,0,8,8,0,19,0,37,1,255,217};
    auto size=mdtest::rasterSize(jpeg);assert(size.x==37 && size.y==19);
    bool rejected=false;try{mdtest::pngSize(jpeg);}catch(const std::runtime_error&){rejected=true;}
    assert(rejected);
}
