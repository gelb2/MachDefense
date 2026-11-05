#include "Core/Simulation/Formation.hpp"
#include <cassert>
#include <iostream>
int main(){
    auto front=md::originalRetreatDestination(0,{10,200},1,0,200);assert(front&&front->x==10&&front->y==120);
    auto rear=md::originalRetreatDestination(0,{-7,100},1,0,200);assert(rear&&rear->x==-7&&rear->y==40);
    assert(!md::originalRetreatDestination(0,{0,80},1,0,200));
    assert(md::originalRetreatDestination(0,{0,80.01f},1,0,200));
    assert(!md::originalRetreatDestination(0,{0,200},0,0,200));
    assert(!md::originalRetreatDestination(0,{0,200},1,1,200));
    for(unsigned order=1;order<6;++order)assert(!md::originalRetreatDestination(order,{0,200},1,0,200));
    // Original has no nonnegative distance check: an actor ahead of the cached
    // front line also receives an order with this formula.
    auto ahead=md::originalRetreatDestination(0,{2,210},1,0,200);assert(ahead&&ahead->y==128);
    std::cout<<"Original retreat order boundaries and unused formation orders passed\n";
}
