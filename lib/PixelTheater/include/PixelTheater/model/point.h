#pragma once
#include <cstdint>
#include <array>
#include "PixelTheater/core/math.h"  // For Vector3d

namespace PixelTheater {

// Core point data structures
struct PointData {
    float x;
    float y; 
    float z;
    uint16_t point_id;  // 0-based point ID
    uint8_t face;       // 0-based face number
    uint8_t face_index; // Index within face (0-103)
};

// Neighbor relationship
struct Neighbor {
    uint16_t index;     // Point index
    float distance;     // Distance in mm
};

// Maximum neighbors per point (from points.cpp)
static constexpr size_t MAX_NEIGHBORS = 7;

// Point neighbor data
struct NeighborData {
    uint16_t point_index;
    std::array<Neighbor, MAX_NEIGHBORS> neighbors;
};

// Main Point class
class Point {
public:
    Point() = default;
    Point(uint16_t id, uint8_t face_id, float x, float y, float z)
        : _id(id)
        , _face_id(face_id)
        , _x(x), _y(y), _z(z)
    {}

    // Accessors
    uint16_t id() const { return _id; }
    uint8_t face_id() const { return _face_id; }
    float x() const { return _x; }
    float y() const { return _y; }
    float z() const { return _z; }

    // Geometric calculations
    float distanceTo(const Point& other) const;
    bool isNeighbor(const Point& other) const;

private:
    uint16_t _id{0};
    uint8_t _face_id{0};
    float _x{0}, _y{0}, _z{0};
};

} // namespace PixelTheater 