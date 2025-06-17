#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"

// Generated on: 2025-06-18 01:19:27
// Generated using: generate_model.py --model-dir /Users/foz/Documents/PlatformIO/Projects/DodecaRGB-firmware/test/fixtures/models/basic_pentagon_model --yes
// Model source: Unknown
// Author: Test Suite

namespace PixelTheater {
namespace Models {

// Simple 3-face pentagon model for testing core functionality
struct BasicPentagonModel : public ModelDefinition<15, 3> {

    // Required metadata
    static constexpr const char* NAME = "BasicPentagonModel";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Simple 3-face pentagon model for testing core functionality";
    static constexpr const char* MODEL_TYPE = "Pentagon";
    static constexpr const char* GENERATED_DATE = "2025-06-18 01:19:27";

    static constexpr size_t LED_COUNT = 15;
    static constexpr size_t FACE_COUNT = 3;
    static constexpr float SPHERE_RADIUS = 395.034f;

    // Hardware metadata from YAML configuration
    static constexpr HardwareData HARDWARE = {
        .led_type = "WS2812B",
        .color_order = "GRB",
        .led_diameter_mm = 5.0f,
        .led_spacing_mm = 10.0f,
        .max_current_per_led_ma = 50,
        .avg_current_per_led_ma = 20
    };

    // LED Groups defined in YAML
    static constexpr std::array<LedGroupData, 4> LED_GROUPS{{
        {
            .name = "center",
            .face_type_id = 0,
            .led_count = 1,
            .led_indices = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        },
        {
            .name = "ring1",
            .face_type_id = 0,
            .led_count = 4,
            .led_indices = {1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        },
        {
            .name = "edge0",
            .face_type_id = 0,
            .led_count = 2,
            .led_indices = {1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        },
        {
            .name = "edge1",
            .face_type_id = 0,
            .led_count = 2,
            .led_indices = {2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        }
    }};

    // Face type definitions with vertex geometry
    static constexpr std::array<FaceTypeData, 1> FACE_TYPES{{
        {
            .id = 0,
            .type = FaceType::Pentagon,
            .num_leds = 5,
            .edge_length_mm = 50.0f
        }
    }};

    // Face instances with transformed vertices
    static constexpr std::array<FaceData, FACE_COUNT> FACES{{
        {.id = 0, .type_id = 0, .rotation = 0, .geometric_id = 0,
            .vertices = {
                {.x = -200.000f, .y = 0.000f, .z = 262.000f},
                {.x = -61.803f, .y = -190.211f, .z = 262.000f},
                {.x = 161.803f, .y = -117.557f, .z = 262.000f},
                {.x = 161.803f, .y = 117.557f, .z = 262.000f},
                {.x = -61.803f, .y = 190.211f, .z = 262.000f}
            }
        },
        {.id = 1, .type_id = 0, .rotation = 0, .geometric_id = 1,
            .vertices = {
                {.x = -99.947f, .y = -307.933f, .z = 61.904f},
                {.x = 100.055f, .y = -307.938f, .z = -61.700f},
                {.x = 261.857f, .y = -190.376f, .z = 61.904f},
                {.x = 161.854f, .y = -117.712f, .z = 261.899f},
                {.x = -61.753f, .y = -190.366f, .z = 261.899f}
            }
        },
        {.id = 2, .type_id = 0, .rotation = 0, .geometric_id = 2,
            .vertices = {
                {.x = 323.786f, .y = -0.000f, .z = -61.700f},
                {.x = 261.976f, .y = 190.211f, .z = 61.904f},
                {.x = 161.966f, .y = 117.557f, .z = 261.899f},
                {.x = 161.966f, .y = -117.557f, .z = 261.899f},
                {.x = 261.976f, .y = -190.211f, .z = 61.904f}
            }
        }
    }};

    // Edge geometry and face relationships
    static constexpr std::array<EdgeData, 15> EDGES{{
        {
            .face_id = 0,
            .edge_index = 0,
            .start_vertex = {.x = -200.000f, .y = 0.000f, .z = 262.000f},
            .end_vertex = {.x = -61.803f, .y = -190.211f, .z = 262.000f},
            .connected_face_id = 255
        },
        {
            .face_id = 0,
            .edge_index = 1,
            .start_vertex = {.x = -61.803f, .y = -190.211f, .z = 262.000f},
            .end_vertex = {.x = 161.803f, .y = -117.557f, .z = 262.000f},
            .connected_face_id = 1
        },
        {
            .face_id = 0,
            .edge_index = 2,
            .start_vertex = {.x = 161.803f, .y = -117.557f, .z = 262.000f},
            .end_vertex = {.x = 161.803f, .y = 117.557f, .z = 262.000f},
            .connected_face_id = 2
        },
        {
            .face_id = 0,
            .edge_index = 3,
            .start_vertex = {.x = 161.803f, .y = 117.557f, .z = 262.000f},
            .end_vertex = {.x = -61.803f, .y = 190.211f, .z = 262.000f},
            .connected_face_id = 255
        },
        {
            .face_id = 0,
            .edge_index = 4,
            .start_vertex = {.x = -61.803f, .y = 190.211f, .z = 262.000f},
            .end_vertex = {.x = -200.000f, .y = 0.000f, .z = 262.000f},
            .connected_face_id = 255
        },
        {
            .face_id = 1,
            .edge_index = 0,
            .start_vertex = {.x = -99.947f, .y = -307.933f, .z = 61.904f},
            .end_vertex = {.x = 100.055f, .y = -307.938f, .z = -61.700f},
            .connected_face_id = 255
        },
        {
            .face_id = 1,
            .edge_index = 1,
            .start_vertex = {.x = 100.055f, .y = -307.938f, .z = -61.700f},
            .end_vertex = {.x = 261.857f, .y = -190.376f, .z = 61.904f},
            .connected_face_id = 255
        },
        {
            .face_id = 1,
            .edge_index = 2,
            .start_vertex = {.x = 261.857f, .y = -190.376f, .z = 61.904f},
            .end_vertex = {.x = 161.854f, .y = -117.712f, .z = 261.899f},
            .connected_face_id = 2
        },
        {
            .face_id = 1,
            .edge_index = 3,
            .start_vertex = {.x = 161.854f, .y = -117.712f, .z = 261.899f},
            .end_vertex = {.x = -61.753f, .y = -190.366f, .z = 261.899f},
            .connected_face_id = 0
        },
        {
            .face_id = 1,
            .edge_index = 4,
            .start_vertex = {.x = -61.753f, .y = -190.366f, .z = 261.899f},
            .end_vertex = {.x = -99.947f, .y = -307.933f, .z = 61.904f},
            .connected_face_id = 255
        },
        {
            .face_id = 2,
            .edge_index = 0,
            .start_vertex = {.x = 323.786f, .y = -0.000f, .z = -61.700f},
            .end_vertex = {.x = 261.976f, .y = 190.211f, .z = 61.904f},
            .connected_face_id = 255
        },
        {
            .face_id = 2,
            .edge_index = 1,
            .start_vertex = {.x = 261.976f, .y = 190.211f, .z = 61.904f},
            .end_vertex = {.x = 161.966f, .y = 117.557f, .z = 261.899f},
            .connected_face_id = 255
        },
        {
            .face_id = 2,
            .edge_index = 2,
            .start_vertex = {.x = 161.966f, .y = 117.557f, .z = 261.899f},
            .end_vertex = {.x = 161.966f, .y = -117.557f, .z = 261.899f},
            .connected_face_id = 0
        },
        {
            .face_id = 2,
            .edge_index = 3,
            .start_vertex = {.x = 161.966f, .y = -117.557f, .z = 261.899f},
            .end_vertex = {.x = 261.976f, .y = -190.211f, .z = 61.904f},
            .connected_face_id = 1
        },
        {
            .face_id = 2,
            .edge_index = 4,
            .start_vertex = {.x = 261.976f, .y = -190.211f, .z = 61.904f},
            .end_vertex = {.x = 323.786f, .y = -0.000f, .z = -61.700f},
            .connected_face_id = 255
        }
    }};

    // Point geometry - define all points with correct face assignments
    static constexpr PointData POINTS[] = {
        {0, 0, -89.915f, 273.398f, 262.000f},
        {1, 0, -102.356f, 269.356f, 262.000f},
        {2, 0, -89.915f, 260.318f, 262.000f},
        {3, 0, -77.474f, 269.353f, 262.000f},
        {4, 0, -82.228f, 283.984f, 262.000f},
        {5, 1, -121.363f, -176.614f, 324.891f},
        {6, 1, -130.365f, -183.155f, 318.014f},
        {7, 1, -115.799f, -180.657f, 313.764f},
        {8, 1, -108.920f, -172.572f, 324.889f},
        {9, 1, -119.240f, -170.073f, 336.020f},
        {10, 2, 274.550f, -273.398f, 36.760f},
        {11, 2, 280.114f, -269.356f, 25.633f},
        {12, 2, 274.550f, -260.318f, 36.760f},
        {13, 2, 268.985f, -269.353f, 47.888f},
        {14, 2, 271.111f, -283.984f, 43.636f}
    };

    // Define neighbor relationships
    static constexpr NeighborData NEIGHBORS[] = {
        {0, {
            {.id = 2, .distance = 13.080f}, {.id = 1, .distance = 13.081f}, {.id = 3, .distance = 13.082f}, {.id = 4, .distance = 13.082f}
        }},
        {1, {
            {.id = 0, .distance = 13.081f}, {.id = 2, .distance = 15.377f}, {.id = 3, .distance = 24.882f}, {.id = 4, .distance = 24.882f}
        }},
        {2, {
            {.id = 0, .distance = 13.080f}, {.id = 3, .distance = 15.376f}, {.id = 1, .distance = 15.377f}, {.id = 4, .distance = 24.883f}
        }},
        {3, {
            {.id = 0, .distance = 13.082f}, {.id = 2, .distance = 15.376f}, {.id = 4, .distance = 15.383f}, {.id = 1, .distance = 24.882f}
        }},
        {4, {
            {.id = 0, .distance = 13.082f}, {.id = 3, .distance = 15.383f}, {.id = 1, .distance = 24.882f}, {.id = 2, .distance = 24.883f}
        }},
        {5, {
            {.id = 7, .distance = 13.080f}, {.id = 6, .distance = 13.081f}, {.id = 9, .distance = 13.082f}, {.id = 8, .distance = 13.082f}
        }},
        {6, {
            {.id = 5, .distance = 13.081f}, {.id = 7, .distance = 15.377f}, {.id = 9, .distance = 24.882f}, {.id = 8, .distance = 24.882f}
        }},
        {7, {
            {.id = 5, .distance = 13.080f}, {.id = 8, .distance = 15.376f}, {.id = 6, .distance = 15.377f}, {.id = 9, .distance = 24.883f}
        }},
        {8, {
            {.id = 5, .distance = 13.082f}, {.id = 7, .distance = 15.376f}, {.id = 9, .distance = 15.383f}, {.id = 6, .distance = 24.882f}
        }},
        {9, {
            {.id = 5, .distance = 13.082f}, {.id = 8, .distance = 15.383f}, {.id = 6, .distance = 24.882f}, {.id = 7, .distance = 24.883f}
        }},
        {10, {
            {.id = 12, .distance = 13.080f}, {.id = 11, .distance = 13.081f}, {.id = 13, .distance = 13.082f}, {.id = 14, .distance = 13.082f}
        }},
        {11, {
            {.id = 10, .distance = 13.081f}, {.id = 12, .distance = 15.377f}, {.id = 13, .distance = 24.882f}, {.id = 14, .distance = 24.882f}
        }},
        {12, {
            {.id = 10, .distance = 13.080f}, {.id = 13, .distance = 15.376f}, {.id = 11, .distance = 15.377f}, {.id = 14, .distance = 24.883f}
        }},
        {13, {
            {.id = 10, .distance = 13.082f}, {.id = 12, .distance = 15.376f}, {.id = 14, .distance = 15.383f}, {.id = 11, .distance = 24.882f}
        }},
        {14, {
            {.id = 10, .distance = 13.082f}, {.id = 13, .distance = 15.383f}, {.id = 11, .distance = 24.882f}, {.id = 12, .distance = 24.883f}
        }}
    };
};

}} // namespace PixelTheater::Models
