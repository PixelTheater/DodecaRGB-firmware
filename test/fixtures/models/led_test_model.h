#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"

namespace PixelTheater {
namespace Fixtures {

// Minimal model for testing LED operations
struct LedTestModel : public ModelDefinition<8, 1> {  // 8 LEDs, 1 face
    static constexpr const char* NAME = "LED Test Model";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Model for testing LED operations";
    static constexpr const char* MODEL_TYPE = "Test";

    // Simple face type with center + ring
    static constexpr FaceTypeData FACE_TYPES[] = {
        {
            .id = 0,
            .type = FaceType::Circle,
            .num_leds = 8,        // 1 center + 7 ring
            .edge_length_mm = 10.0f,
            .num_centers = 1,
            .num_rings = 1,
            .num_edges = 0
        }
    };

    // Single face
    static constexpr FaceData FACES[] = {
        {0, 0, 0,  0.0f,  0.0f,  1.0f}  // Single face at origin
    };

    // Points in a simple circular pattern
    static constexpr PointData POINTS[] = {
        // Center point
        {0, 0,  0.0f,  0.0f,  1.0f},  // Center LED
        // Ring points in a circle
        {1, 0,  1.0f,  0.0f,  1.0f},
        {2, 0,  0.7f,  0.7f,  1.0f},
        {3, 0,  0.0f,  1.0f,  1.0f},
        {4, 0, -0.7f,  0.7f,  1.0f},
        {5, 0, -1.0f,  0.0f,  1.0f},
        {6, 0, -0.7f, -0.7f,  1.0f},
        {7, 0,  0.0f, -1.0f,  1.0f}
    };

    // Simple neighbor relationships
    static constexpr NeighborData NEIGHBORS[] = {
        // Center connects to all ring points
        {0, {{1, 1.0f}, {2, 1.0f}, {3, 1.0f}, {4, 1.0f}, 
             {5, 1.0f}, {6, 1.0f}, {7, 1.0f}}}
    };
};

}} // namespace PixelTheater::Fixtures 