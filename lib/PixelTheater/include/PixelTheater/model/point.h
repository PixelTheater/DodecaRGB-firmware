#pragma once
#include <cstdint>
#include <array>
#include "../core/math.h"  // For Vector3d

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
    // Core position data
    float x() const { return _x; }
    float y() const { return _y; }
    float z() const { return _z; }

    // Face information
    uint8_t face() const { return _face; }
    uint8_t index() const { return _index; }  // Index within face
    uint16_t id() const { return _id; }       // Global point ID

    // Geometric calculations
    float distanceTo(const Point& other) const;
    bool isNeighbor(const Point& other) const;

    // Construction
    Point() = default;
    Point(float x, float y, float z, uint8_t face, uint8_t index, uint16_t id = 0)
        : _x(x), _y(y), _z(z), _face(face), _index(index), _id(id) {}

private:
    float _x{0}, _y{0}, _z{0};
    uint8_t _face{0};
    uint8_t _index{0};  // Index within face
    uint16_t _id{0};    // Global point ID
};

} // namespace PixelTheater 