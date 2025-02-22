#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"

namespace PixelTheater {
namespace Fixtures {

// Basic pentagon model with 5 LEDs per face for testing core functionality
struct BasicPentagonModel : public ModelDefinition<15, 3> {  // 15 total LEDs (5 per face * 3 faces)
    // Required metadata
    static constexpr const char* NAME = "Basic Pentagon Model";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Three faces with 5 LEDs each for testing";
    static constexpr const char* MODEL_TYPE = "Pentagon";

    static constexpr size_t LED_COUNT = 15;
    static constexpr size_t FACE_COUNT = 3;

    // Face type definitions - each face has 5 LEDs
    static constexpr std::array<FaceTypeData, 1> FACE_TYPES{{
        {
            .id = 0,
            .type = FaceType::Pentagon,
            .num_leds = 5,  // Reduced from 20 to 5
            .edge_length_mm = 50.0f
        }
    }};

    // Face instances
    static constexpr std::array<FaceData, FACE_COUNT> FACES{{
        {.id = 0, .type_id = 0},  // Face 0: LEDs 0-4
        {.id = 1, .type_id = 0},  // Face 1: LEDs 5-9
        {.id = 2, .type_id = 0}   // Face 2: LEDs 10-14
    }};

    // Point geometry - define all points with correct face assignments
    static constexpr PointData POINTS[] = {
        // Face 0 points
        {0,  0, 0.0f,  0.0f,  1.0f},
        {1,  0, 1.0f,  0.0f,  1.0f},
        {2,  0, 0.5f,  0.87f, 1.0f},
        {3,  0, -0.5f, 0.87f, 1.0f},
        {4,  0, -1.0f, 0.0f,  1.0f},
        
        // Face 1 points 
        {5,  1, 0.0f,  0.0f,  0.0f},
        {6,  1, 1.0f,  0.0f,  0.0f},
        {7,  1, 0.5f,  0.87f, 0.0f},
        {8,  1, -0.5f, 0.87f, 0.0f},
        {9,  1, -1.0f, 0.0f,  0.0f},

        // Face 2 points
        {10, 2, 0.0f,  0.0f,  -1.0f},
        {11, 2, 1.0f,  0.0f,  -1.0f},
        {12, 2, 0.5f,  0.87f, -1.0f},
        {13, 2, -0.5f, 0.87f, -1.0f},
        {14, 2, -1.0f, 0.0f,  -1.0f}
    };

    // Define neighbor relationships
    static constexpr NeighborData NEIGHBORS[] = {
        // Center point neighbors
        {
            .point_id = 0,
            .neighbors = {
                {.id = 1, .distance = 10.0f},
                {.id = 2, .distance = 10.0f},
                {.id = 3, .distance = 10.0f},
                {.id = 4, .distance = 10.0f},
                {.id = 5, .distance = 10.0f}
            }
        },
        // Additional neighbor data will be added...
    };
};

}} // namespace PixelTheater::Fixtures 