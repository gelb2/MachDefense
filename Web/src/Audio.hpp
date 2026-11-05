#pragma once
#include "Assets.hpp"
#include "Audio/OpenALEffects.hpp"
namespace md::web {
// Only resource decoding is web-specific. The existing shared effect player is reused.
class Audio:public audio::OpenALEffects {
public:
 void prepare(Assets&);
};
}
