#include "PixelTheater/limits.h"
#include "PixelTheater/model/point.h"
#include "PixelTheater/model/face.h"  // For Vertex struct
#include <cmath>
#include <algorithm> // For std::min
#include <cstring>   // For std::memcpy

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

void Point::setNeighbors(const Point::Neighbor* neighbors_ptr, size_t count) {
    // Copy the provided neighbor data into the internal array
    size_t num_to_copy = std::min(count, Limits::MAX_NEIGHBORS);
    // Use memcpy since Point::Neighbor and ModelDefinition::NeighborData::Neighbor
    // have identical layout (uint16_t id, float distance)
    if (num_to_copy > 0 && neighbors_ptr != nullptr) {
        std::memcpy(_neighbors.data(), neighbors_ptr, num_to_copy * sizeof(Point::Neighbor));
    }

    // Optional: Fill remaining slots with a sentinel value if needed
    for(size_t i = num_to_copy; i < Limits::MAX_NEIGHBORS; ++i) {
        _neighbors[i].id = 0xFFFF; // Example sentinel ID
        _neighbors[i].distance = -1.0f;
    }
}

float Point::distanceTo(const Vertex& vertex) const {
    float dx = _x - vertex.x;
    float dy = _y - vertex.y;
    float dz = _z - vertex.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

} // namespace PixelTheater 