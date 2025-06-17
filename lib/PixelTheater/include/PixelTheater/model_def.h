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
        char name[16];           // Group name (e.g., "ring0", "edge1")
        uint8_t face_type_id;    // Which face type this group belongs to
        uint8_t led_count;       // Number of LEDs in this group
        uint16_t led_indices[32]; // LED indices within the face (increased limit)
    };

    // Edge geometry data
    struct EdgeData {
        uint8_t face_id;         // Face this edge belongs to
        uint8_t edge_index;      // Index within the face (0-4 for pentagon)
        struct Point3D {
            float x, y, z;
        };
        Point3D start_vertex;    // Start point of edge
        Point3D end_vertex;      // End point of edge
        uint8_t connected_face_id; // ID of connected face (-1 if none)
    };

    // Hardware metadata
    struct HardwareData {
        char led_type[16];       // e.g., "WS2812B"
        char color_order[8];     // e.g., "GRB"
        float led_diameter_mm;   // Physical LED diameter
        float led_spacing_mm;    // Average LED spacing
        uint16_t max_current_per_led_ma; // Maximum current per LED
        uint16_t avg_current_per_led_ma; // Average current per LED
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