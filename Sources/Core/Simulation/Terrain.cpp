#include "Core/Simulation/Battle.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace md {
TerrainSegment TerrainSegment::between(Vec2 origin,Vec2 second) {
    TerrainSegment line;line.origin=origin;line.second=second;
    line.direction={second.x-origin.x,second.y-origin.y};
    line.length=std::sqrt(line.direction.x*line.direction.x+line.direction.y*line.direction.y);
    if(line.length==0) throw std::runtime_error("Degenerate original terrain segment");
    line.direction.x/=line.length;line.direction.y/=line.length;
    line.normal={line.direction.y,-line.direction.x};line.cornerNormal=line.direction;
    return line;
}
void TerrainSegment::setPrevious(const TerrainSegment* previous) {
    cornerNormal=direction;
    if(previous) {cornerNormal.x+=previous->direction.x;cornerNormal.y+=previous->direction.y;}
    const float squared=cornerNormal.x*cornerNormal.x+cornerNormal.y*cornerNormal.y;
    const float size=std::sqrt(squared);
    if(size<1e-5f) {cornerNormal={};return;} // QVector::normalize 0x472d4.
    cornerNormal.x/=size;cornerNormal.y/=size;
}
float TerrainSegment::penetration(Vec2 position,float radius) const {
    const Vec2 delta{position.x-origin.x,position.y-origin.y};
    if(direction.x*delta.x+direction.y*delta.y>length+radius) return 0;
    if(cornerNormal.x*delta.x+cornerNormal.y*delta.y<0) return 0;
    const float projection=normal.x*delta.x+normal.y*delta.y;
    return std::max(0.f,static_cast<float>(static_cast<double>(radius)-std::abs(static_cast<double>(projection))));
}

}
