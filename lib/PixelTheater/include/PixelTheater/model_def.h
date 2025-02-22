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

    // Face instance data
    struct FaceData {
        uint8_t id;
        uint8_t type_id;
        uint8_t rotation;
        float x, y, z;  // Position (normal calculated from this)
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