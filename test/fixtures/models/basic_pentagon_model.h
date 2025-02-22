#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"

namespace PixelTheater {
namespace Fixtures {

// Basic pentagon model with 20 LEDs for testing core functionality
struct BasicPentagonModel : public ModelDefinition<20, 1> {
    // Required metadata
    static constexpr const char* NAME = "Basic Pentagon Model";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Single pentagon face for testing";
    static constexpr const char* MODEL_TYPE = "Pentagon";

    // Face type definition - just one pentagon face
    static constexpr FaceTypeData FACE_TYPES[] = {
        {
            .id = 0,
            .type = FaceType::Pentagon,
            .num_leds = 20,
            .edge_length_mm = 50.0f,
        }
    };

    // Face instances - top and bottom
    static constexpr FaceData FACES[] = {
        // id, type_id, rotation, x, y, z
        {0, 0, 0,  0.0f,  0.0f,  1.0f},  // Top face
        {1, 0, 2,  0.0f,  0.0f, -1.0f}   // Bottom face, rotated 144°
    };

    // Point geometry - define key points first
    static constexpr PointData POINTS[] = {
        // Center point
        {.id = 0, .face_id = 0, .x = 0.0f, .y = 0.0f, .z = 0.0f},
        
        // First ring (5 points)
        {.id = 1, .face_id = 0, .x = 10.0f, .y = 0.0f, .z = 0.0f},
        {.id = 2, .face_id = 0, .x = 3.09f, .y = 9.51f, .z = 0.0f},
        {.id = 3, .face_id = 0, .x = -8.09f, .y = 5.88f, .z = 0.0f},
        {.id = 4, .face_id = 0, .x = -8.09f, .y = -5.88f, .z = 0.0f},
        {.id = 5, .face_id = 0, .x = 3.09f, .y = -9.51f, .z = 0.0f},
        
        // Edges
        {.id = 6, .face_id = 0, .x = 10.0f, .y = 0.0f, .z = 0.0f},
        {.id = 7, .face_id = 0, .x = 3.09f, .y = 9.51f, .z = 0.0f},
        {.id = 8, .face_id = 0, .x = -8.09f, .y = 5.88f, .z = 0.0f},
        {.id = 9, .face_id = 0, .x = -8.09f, .y = -5.88f, .z = 0.0f},
        {.id = 10, .face_id = 0, .x = 3.09f, .y = -9.51f, .z = 0.0f},
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