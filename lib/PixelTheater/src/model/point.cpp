#include "PixelTheater/limits.h"
#include "PixelTheater/model/point.h"
#include <cmath>

namespace PixelTheater {

float Point::distanceTo(const Point& other) const {
    float dx = _x - other._x;
    float dy = _y - other._y;
    float dz = _z - other._z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

bool Point::isNeighbor(const Point& other) const {
    // Points are neighbors if they're within threshold
    return (distanceTo(other) < Limits::NEIGHBOR_THRESHOLD);
}

} // namespace PixelTheater 