#include "PixelTheater/model/point.h"
#include <cmath>

namespace PixelTheater {

float Point::distanceTo(const Point& other) const {
    // Use same calculation as LED_Point::distance_to
    return std::sqrt(
        (_x - other._x)*(_x - other._x) + 
        (_y - other._y)*(_y - other._y) + 
        (_z - other._z)*(_z - other._z)
    );
}

bool Point::isNeighbor(const Point& other) const {
    // Use MAX_NEIGHBORS from points.h for consistency
    static constexpr float NEIGHBOR_THRESHOLD = 30.0f; // Based on existing points.cpp data
    
    // Points are neighbors if they're within threshold
    // and either on same face or adjacent faces
    return (distanceTo(other) < NEIGHBOR_THRESHOLD) &&
           (_face == other._face || std::abs(_face - other._face) == 1);
}

} // namespace PixelTheater 