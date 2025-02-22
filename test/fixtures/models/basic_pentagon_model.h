#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"

namespace PixelTheater {
namespace Fixtures {

// Basic pentagon model with 20 LEDs for testing core functionality
struct BasicPentagonModel : public ModelDefinition<20, 3> {
    // Required metadata
    static constexpr const char* NAME = "Basic Pentagon Model";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Single pentagon face for testing";
    static constexpr const char* MODEL_TYPE = "Pentagon";

    static constexpr size_t LED_COUNT = 20;
    static constexpr size_t FACE_COUNT = 3;  // Using 3 faces for testing

    // Face type definitions
    static constexpr std::array<FaceTypeData, 1> FACE_TYPES{{
        {
            .id = 0,
            .type = FaceType::Pentagon,
            .num_leds = 20,
            .edge_length_mm = 50.0f
        }
    }};

    // Face instances
    static constexpr std::array<FaceData, FACE_COUNT> FACES{{
        {.id = 0, .type_id = 0},  // Face 0: type 0
        {.id = 1, .type_id = 0},  // Face 1: type 0
        {.id = 2, .type_id = 0}   // Face 2: type 0
    }};

    // Point geometry - define key points
    static constexpr PointData POINTS[] = {
        // Center point
        {0, 0,   0.0f,  0.0f,  1.0f},  // Center
        // Ring points (5)
        {1, 0,   1.0f,  0.0f,  1.0f},
        {2, 0,  -0.5f,  0.87f, 1.0f},
        {3, 0,  -0.5f, -0.87f, 1.0f},
        {4, 0,   0.31f, 0.95f, 1.0f},
        {5, 0,  -0.81f, 0.59f, 1.0f},
        // More points to reach 20...
        {6, 0,   0.0f,  0.0f,  1.0f},
        {7, 0,   0.0f,  0.0f,  1.0f},
        {8, 0,   0.0f,  0.0f,  1.0f},
        {9, 0,   0.0f,  0.0f,  1.0f},
        {10, 0,  0.0f,  0.0f,  1.0f},
        {11, 0,  0.0f,  0.0f,  1.0f},
        {12, 0,  0.0f,  0.0f,  1.0f},
        {13, 0,  0.0f,  0.0f,  1.0f},
        {14, 0,  0.0f,  0.0f,  1.0f},
        {15, 0,  0.0f,  0.0f,  1.0f},
        {16, 0,  0.0f,  0.0f,  1.0f},
        {17, 0,  0.0f,  0.0f,  1.0f},
        {18, 0,  0.0f,  0.0f,  1.0f},
        {19, 0,  0.0f,  0.0f,  1.0f},
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