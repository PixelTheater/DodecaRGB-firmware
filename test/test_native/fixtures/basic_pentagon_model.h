#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"  // For FaceType enum
#include "PixelTheater/model/region_type.h"  // For RegionType enum
#include "PixelTheater/model/face.h"

namespace PixelTheater {
namespace Fixtures {

// Basic model with two pentagon faces for testing core functionality
struct BasicPentagonModel : public ModelDefinition<40, 2> {
    // Required metadata
    static constexpr char NAME[] = "Basic Pentagon Model";
    static constexpr char VERSION[] = "1.0";
    static constexpr char DESCRIPTION[] = "Two pentagon faces for basic tests";
    static constexpr char MODEL_TYPE[] = "Pentagon";  // For generator strategy

    // Face type definition - one pentagon type
    static constexpr FaceTypeData FACE_TYPES[] = {
        {
            0,                    // id
            FaceType::Pentagon,   // type
            20,                   // num_leds
            60.0f,               // edge_length_mm
            1,                    // num_centers
            3,                    // num_rings
            5                     // num_edges
        }
    };

    // Face instances - top and bottom
    static constexpr FaceData FACES[] = {
        // id, type_id, rotation, x, y, z
        {0, 0, 0,  0.0f,  0.0f,  1.0f},  // Top face
        {1, 0, 2,  0.0f,  0.0f, -1.0f}   // Bottom face, rotated 144°
    };

    // Points - minimal set for testing
    static constexpr PointData POINTS[] = {
        // id, face_id, x, y, z
        // Face 0 (top)
        {0,  0,  0.0f,  0.0f,  1.0f},  // Center
        {1,  0,  1.0f,  0.0f,  1.0f},  // Ring 1
        {2,  0,  0.3f,  0.9f,  1.0f},  // Ring 1
        {3,  0, -0.8f,  0.6f,  1.0f},  // Ring 1
        {4,  0, -0.8f, -0.6f,  1.0f},  // Ring 1
        {5,  0,  0.3f, -0.9f,  1.0f},  // Ring 1

        // Face 1 (bottom)
        {20, 1,  0.0f,  0.0f, -1.0f},  // Center
        {21, 1,  1.0f,  0.0f, -1.0f},  // Ring 1
        // ... more points
    };

    // Region definitions
    static constexpr RegionData REGIONS[] = {
        // id, face_id, type, {led_ids}
        // Face 0 regions
        {0, 0, RegionType::Center, {0}},                    // Center
        {1, 0, RegionType::Ring, {1, 2, 3, 4, 5}},         // Ring 1
        {2, 0, RegionType::Edge, {6, 7, 8, 9, 10}},         // Edge 1
        {3, 0, RegionType::Edge, {11, 12, 13, 14, 15}},    // Edge 2
        {4, 0, RegionType::Edge, {16, 17, 18, 19, 20}},    // Edge 3
        
        // Face 1 regions
        {2, 1, RegionType::Center, {20}},                   // Center
        {3, 1, RegionType::Ring, {21, 22, 23, 24, 25}},    // Ring 1
        {4, 1, RegionType::Edge, {26, 27, 28, 29, 30}},    // Edge 1
        {5, 1, RegionType::Edge, {31, 32, 33, 34, 35}},    // Edge 2
        {6, 1, RegionType::Edge, {36, 37, 38, 39, 40}},    // Edge 3
        
    };

    // Neighbor relationships
    static constexpr NeighborData NEIGHBORS[] = {
        // id, {neighbors}
        {0, {{1, 30.0f}, {2, 30.0f}, {3, 30.0f}, {4, 30.0f}, {5, 30.0f}}},  // Center to ring
        {1, {{0, 30.0f}, {2, 30.0f}, {5, 30.0f}}},  // Ring point connections
        // ... more neighbors
    };
};

} // namespace Fixtures
} // namespace PixelTheater 