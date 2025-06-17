#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"

// Generated on: 2025-06-18 00:23:48
// Generated using: generate_model.py -d test/fixtures/models/test_no_remap -y
// Model source: Test fixture
// Author: Test

namespace PixelTheater {
namespace Models {

// Simple pyramid model without face remapping for testing
struct PyramidNoRemap : public ModelDefinition<12, 4> {

    // Required metadata
    static constexpr const char* NAME = "PyramidNoRemap";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Simple pyramid model without face remapping for testing";
    static constexpr const char* MODEL_TYPE = "Pyramid";
    static constexpr const char* GENERATED_DATE = "2025-06-18 00:23:48";

    static constexpr size_t LED_COUNT = 12;
    static constexpr size_t FACE_COUNT = 4;
    static constexpr float SPHERE_RADIUS = 352.823f;

    // Hardware metadata from YAML configuration
    static constexpr HardwareData HARDWARE = {
        .led_type = "WS2812B",
        .color_order = "GRB",
        .led_diameter_mm = 2.0f,
        .led_spacing_mm = 5.0f,
        .max_current_per_led_ma = 20,
        .avg_current_per_led_ma = 10
    };

    // LED Groups defined in YAML
    static constexpr std::array<LedGroupData, 2> LED_GROUPS{{
        {
            .name = "center",
            .face_type_id = 0,
            .led_count = 1,
            .led_indices = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        },
        {
            .name = "edge0",
            .face_type_id = 0,
            .led_count = 2,
            .led_indices = {1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        }
    }};

    // Face type definitions with vertex geometry
    static constexpr std::array<FaceTypeData, 1> FACE_TYPES{{
        {
            .id = 0,
            .type = FaceType::Triangle,
            .num_leds = 3,
            .edge_length_mm = 50.0f
        }
    }};

    // Face instances with transformed vertices
    static constexpr std::array<FaceData, FACE_COUNT> FACES{{
        {.id = 0, .type_id = 0, .rotation = 0, .geometric_id = 0,
            .vertices = {
                {.x = -200.000f, .y = 0.000f, .z = 262.000f},
                {.x = 100.000f, .y = -173.205f, .z = 262.000f},
                {.x = 100.000f, .y = 173.205f, .z = 262.000f},
                {.x = 0.0f, .y = 0.0f, .z = 0.0f},
                {.x = 0.0f, .y = 0.0f, .z = 0.0f}
            }
        },
        {.id = 1, .type_id = 0, .rotation = 0, .geometric_id = 1,
            .vertices = {
                {.x = -99.947f, .y = -307.933f, .z = 61.904f},
                {.x = 232.264f, .y = -233.861f, .z = -2.513f},
                {.x = 84.922f, .y = -126.801f, .z = 292.153f},
                {.x = 0.0f, .y = 0.0f, .z = 0.0f},
                {.x = 0.0f, .y = 0.0f, .z = 0.0f}
            }
        },
        {.id = 2, .type_id = 0, .rotation = 0, .geometric_id = 2,
            .vertices = {
                {.x = 323.786f, .y = -0.000f, .z = -61.700f},
                {.x = 189.608f, .y = 173.205f, .z = 206.622f},
                {.x = 189.608f, .y = -173.205f, .z = 206.622f},
                {.x = 0.0f, .y = 0.0f, .z = 0.0f},
                {.x = 0.0f, .y = 0.0f, .z = 0.0f}
            }
        },
        {.id = 3, .type_id = 0, .rotation = 0, .geometric_id = 3,
            .vertices = {
                {.x = 100.055f, .y = 307.938f, .z = -61.700f},
                {.x = -106.136f, .y = 233.852f, .z = 206.622f},
                {.x = 223.320f, .y = 126.805f, .z = 206.622f},
                {.x = 0.0f, .y = 0.0f, .z = 0.0f},
                {.x = 0.0f, .y = 0.0f, .z = 0.0f}
            }
        }
    }};

    // Edge geometry and face relationships
    static constexpr std::array<EdgeData, 12> EDGES{{
        {
            .face_id = 0,
            .edge_index = 0,
            .start_vertex = {.x = -200.000f, .y = 0.000f, .z = 262.000f},
            .end_vertex = {.x = 100.000f, .y = -173.205f, .z = 262.000f},
            .connected_face_id = 255
        },
        {
            .face_id = 0,
            .edge_index = 1,
            .start_vertex = {.x = 100.000f, .y = -173.205f, .z = 262.000f},
            .end_vertex = {.x = 100.000f, .y = 173.205f, .z = 262.000f},
            .connected_face_id = 255
        },
        {
            .face_id = 0,
            .edge_index = 2,
            .start_vertex = {.x = 100.000f, .y = 173.205f, .z = 262.000f},
            .end_vertex = {.x = -200.000f, .y = 0.000f, .z = 262.000f},
            .connected_face_id = 255
        },
        {
            .face_id = 1,
            .edge_index = 0,
            .start_vertex = {.x = -99.947f, .y = -307.933f, .z = 61.904f},
            .end_vertex = {.x = 232.264f, .y = -233.861f, .z = -2.513f},
            .connected_face_id = 255
        },
        {
            .face_id = 1,
            .edge_index = 1,
            .start_vertex = {.x = 232.264f, .y = -233.861f, .z = -2.513f},
            .end_vertex = {.x = 84.922f, .y = -126.801f, .z = 292.153f},
            .connected_face_id = 255
        },
        {
            .face_id = 1,
            .edge_index = 2,
            .start_vertex = {.x = 84.922f, .y = -126.801f, .z = 292.153f},
            .end_vertex = {.x = -99.947f, .y = -307.933f, .z = 61.904f},
            .connected_face_id = 255
        },
        {
            .face_id = 2,
            .edge_index = 0,
            .start_vertex = {.x = 323.786f, .y = -0.000f, .z = -61.700f},
            .end_vertex = {.x = 189.608f, .y = 173.205f, .z = 206.622f},
            .connected_face_id = 255
        },
        {
            .face_id = 2,
            .edge_index = 1,
            .start_vertex = {.x = 189.608f, .y = 173.205f, .z = 206.622f},
            .end_vertex = {.x = 189.608f, .y = -173.205f, .z = 206.622f},
            .connected_face_id = 255
        },
        {
            .face_id = 2,
            .edge_index = 2,
            .start_vertex = {.x = 189.608f, .y = -173.205f, .z = 206.622f},
            .end_vertex = {.x = 323.786f, .y = -0.000f, .z = -61.700f},
            .connected_face_id = 255
        },
        {
            .face_id = 3,
            .edge_index = 0,
            .start_vertex = {.x = 100.055f, .y = 307.938f, .z = -61.700f},
            .end_vertex = {.x = -106.136f, .y = 233.852f, .z = 206.622f},
            .connected_face_id = 255
        },
        {
            .face_id = 3,
            .edge_index = 1,
            .start_vertex = {.x = -106.136f, .y = 233.852f, .z = 206.622f},
            .end_vertex = {.x = 223.320f, .y = 126.805f, .z = 206.622f},
            .connected_face_id = 255
        },
        {
            .face_id = 3,
            .edge_index = 2,
            .start_vertex = {.x = 223.320f, .y = 126.805f, .z = 206.622f},
            .end_vertex = {.x = 100.055f, .y = 307.938f, .z = -61.700f},
            .connected_face_id = 255
        }
    }};

    // Point geometry - define all points with correct face assignments
    static constexpr PointData POINTS[] = {
        {0, 0, -74.001f, 224.419f, 262.000f},
        {1, 0, -90.534f, 191.972f, 262.000f},
        {2, 0, -107.066f, 159.525f, 262.000f},
        {3, 1, -86.815f, -184.983f, 287.626f},
        {4, 1, -87.262f, -202.042f, 255.456f},
        {5, 1, -87.708f, -219.102f, 223.287f},
        {6, 2, 267.432f, -224.419f, 50.994f},
        {7, 2, 274.826f, -191.972f, 36.208f},
        {8, 2, 282.220f, -159.525f, 21.421f},
        {9, 3, 296.076f, 184.993f, 50.994f},
        {10, 3, 267.502f, 202.052f, 36.208f},
        {11, 3, 238.928f, 219.112f, 21.421f}
    };

    // Define neighbor relationships
    static constexpr NeighborData NEIGHBORS[] = {
        {0, {
            {.id = 1, .distance = 36.416f}, {.id = 2, .distance = 72.832f}
        }},
        {1, {
            {.id = 0, .distance = 36.416f}, {.id = 2, .distance = 36.416f}
        }},
        {2, {
            {.id = 1, .distance = 36.416f}, {.id = 0, .distance = 72.832f}
        }},
        {3, {
            {.id = 4, .distance = 36.416f}, {.id = 5, .distance = 72.832f}
        }},
        {4, {
            {.id = 5, .distance = 36.416f}, {.id = 3, .distance = 36.416f}
        }},
        {5, {
            {.id = 4, .distance = 36.416f}, {.id = 3, .distance = 72.832f}
        }},
        {6, {
            {.id = 7, .distance = 36.416f}, {.id = 8, .distance = 72.832f}
        }},
        {7, {
            {.id = 6, .distance = 36.416f}, {.id = 8, .distance = 36.416f}
        }},
        {8, {
            {.id = 7, .distance = 36.416f}, {.id = 6, .distance = 72.832f}
        }},
        {9, {
            {.id = 10, .distance = 36.416f}, {.id = 11, .distance = 72.832f}
        }},
        {10, {
            {.id = 11, .distance = 36.416f}, {.id = 9, .distance = 36.416f}
        }},
        {11, {
            {.id = 10, .distance = 36.416f}, {.id = 9, .distance = 72.832f}
        }}
    };
};

}} // namespace PixelTheater::Models
