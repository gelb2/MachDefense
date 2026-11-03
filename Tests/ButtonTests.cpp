#include "Button.hpp"
#include <cassert>
#include <iostream>
int main(){
    md::Button b; b.bounds={10,20,30,40};
    using P=md::TouchPhase;using A=md::ButtonAction;
    assert(!b.contains({10,30})&&!b.contains({20,20}));
    assert(b.contains({40,60})&&!b.contains({40.01f,60}));
    assert(b.touch({1,P::began,{20,30}}).action==A::push&&b.pressed());
    assert(b.touch({2,P::ended,{20,30}}).action==A::none&&b.pressed());
    assert(b.touch({1,P::moved,{60,30}}).action==A::release&&!b.pressed());
    assert(b.touch({1,P::moved,{20,30}}).action==A::push&&b.pressed());
    assert(b.touch({1,P::ended,{20,30}}).action==A::pop&&!b.pressed());
    assert(b.touch({1,P::ended,{20,30}}).action==A::none);
    b.touch({3,P::began,{20,30}});assert(b.touch({3,P::cancelled,{20,30}}).action==A::cancel);
    b.enabled=false;assert(b.touch({4,P::began,{20,30}}).action==A::none);
    b.enabled=true;b.shown=false;assert(b.touch({4,P::began,{20,30}}).action==A::none);
    std::cout<<"Original button bounds, finger ownership, drag, release and cancellation passed\n";
}
