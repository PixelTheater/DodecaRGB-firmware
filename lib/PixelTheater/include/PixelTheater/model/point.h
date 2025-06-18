#pragma once
#include <cstdint>
#include <array>
#include "PixelTheater/core/math.h"  // For Vector3d
#include <cmath>    // For sqrtf, powf
#include "PixelTheater/limits.h" // For MAX_NEIGHBORS
#include "PixelTheater/model_def.h" // For Neighbor struct definition

// Forward declare Vertex struct (to avoid circular includes)
namespace PixelTheater {
    struct Vertex;
}

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

// Neighbor relationship (Now defined in model_def.h, keep a local alias/using for clarity if needed, or just use PixelTheater::ModelDefinition<...>::NeighborData::Neighbor)
// struct Neighbor {
//     uint16_t index;     // Point index
//     float distance;     // Distance in mm
// };
// Using the definition from model_def.h directly might be cleaner if used widely.
// For now, we'll assume model_def.h is included where needed.

// Maximum neighbors per point (from limits.h)
// static constexpr size_t MAX_NEIGHBORS = Limits::MAX_NEIGHBORS; // Use Limits::MAX_NEIGHBORS directly

// Point neighbor data (Now defined in model_def.h)
// struct NeighborData {
//     uint16_t point_index;
//     std::array<Neighbor, MAX_NEIGHBORS> neighbors;
// };

// Main Point class
class Point {
public:
    // Using declaration for the Neighbor struct from the context it will be used in (ModelDefinition template)
    // This avoids needing template parameters here, but relies on model_def.h being included.
    // Alternatively, define a local Neighbor struct identical to the one in ModelDefinition.
    // Let's define it locally for simplicity within this header.
    // Local Neighbor struct matching layout in ModelDefinition for simplicity
    struct Neighbor {
        uint16_t id;
        float distance;
    };

    Point() = default;
    Point(uint16_t id, uint8_t face_id, float x, float y, float z)
        : _id(id)
        , _face_id(face_id)
        , _x(x), _y(y), _z(z)
    {}
    // ADDED: Constructor for x, y, z only (default id/face_id to 0)
    Point(float x, float y, float z)
        : _id(0), _face_id(0), _x(x), _y(y), _z(z) {}

    // Accessors
    uint16_t id() const { return _id; }
    uint8_t face_id() const { return _face_id; }
    float x() const { return _x; }
    float y() const { return _y; }
    float z() const { return _z; }

    // Geometric calculations
    float distanceTo(const Point& other) const;
    bool isNeighbor(const Point& other) const;
    
    // Distance calculation to Vertex (implemented in point.cpp)
    float distanceTo(const Vertex& vertex) const;

    // Neighbor access
    const std::array<Neighbor, Limits::MAX_NEIGHBORS>& getNeighbors() const { return _neighbors; }

    // Internal setter for Model initialization
    void setNeighbors(const Point::Neighbor* neighbors_ptr, size_t count);

private:
    uint16_t _id{0};
    uint8_t _face_id{0};
    float _x{0}, _y{0}, _z{0};
    std::array<Neighbor, Limits::MAX_NEIGHBORS> _neighbors{}; // Initialize empty
};

} // namespace PixelTheater 