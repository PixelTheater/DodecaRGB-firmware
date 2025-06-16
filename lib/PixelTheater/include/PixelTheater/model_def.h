#pragma once
#include "model/face_type.h"
#include "limits.h"  // For MAX_LEDS_PER_REGION
#include <array>

namespace PixelTheater {

template<uint16_t NumLeds, uint8_t NumFaces>
struct ModelDefinition {
    // Required constants
    static constexpr uint16_t LED_COUNT = NumLeds;
    static constexpr uint8_t FACE_COUNT = NumFaces;

    // Face type properties
    struct FaceTypeData {
        uint8_t id;
        FaceType type;
        uint16_t num_leds;
        float edge_length_mm;
    };

    // LED Group data for accessing YAML-defined LED groups at runtime
    struct LedGroupData {
        uint8_t face_type_id;
        uint8_t group_id;        // Index within face type
        uint8_t led_count;
        uint16_t led_indices[16]; // Maximum 16 LEDs per group (adjustable)
    };

    // Face instance data
    struct FaceData {
        uint8_t id;
        uint8_t type_id;
        uint8_t rotation;
        float x, y, z;  // Position (normal calculated from this)
        uint8_t geometric_id;  // Geometric position (for remapping support)
        // Array of x,y,z coordinates for each vertex
        struct Vertex {
            float x, y, z;
        };
        Vertex vertices[Limits::MAX_EDGES_PER_FACE];  // Instead of std::array
    };

    // Point geometry
    struct PointData {
        uint16_t id;
        uint8_t face_id;
        float x, y, z;
    };

    // Point neighbor data
    struct NeighborData {
        uint16_t point_id;
        struct Neighbor {
            uint16_t id;
            float distance;
        };
        static constexpr size_t MAX_NEIGHBORS = Limits::MAX_NEIGHBORS;
        Neighbor neighbors[MAX_NEIGHBORS];
    };
};

} // namespace PixelTheater 