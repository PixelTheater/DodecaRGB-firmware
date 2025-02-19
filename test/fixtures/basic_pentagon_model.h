#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"
#include "PixelTheater/model/region_type.h"

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
            .num_centers = 1,
            .num_rings = 3,
            .num_edges = 5
        }
    };

    // Face instance - single face at origin
    static constexpr FaceData FACES[] = {
        {
            .id = 0,
            .type_id = 0,
            .rotation = 0,
            .x = 0.0f,
            .y = 0.0f,
            .z = 1.0f
        }
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
        
        // Additional points will be added...
    };

    // Region definitions
    static constexpr RegionData REGIONS[] = {
        // Center region
        {
            .id = 0,
            .face_id = 0,
            .type = RegionType::Center,
            .led_ids = {0}  // Center LED
        },
        
        // First ring region
        {
            .id = 1,
            .face_id = 0,
            .type = RegionType::Ring,
            .led_ids = {1, 2, 3, 4, 5}  // First ring LEDs
        },
        
        // Additional regions will be added...
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