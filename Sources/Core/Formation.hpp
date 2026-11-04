#pragma once
#include "RenderTypes.hpp"
#include <optional>
namespace md {
// GobWorld::setOrder: 0x17418. Orders other than 0 return immediately.
// DlgFormation's five icon handlers are no-ops in WeMade 1.0.3.
std::optional<Vec2> originalRetreatDestination(unsigned order,Vec2 position,int hp,
                                              int machineType,float topPlayerPosition);
}
