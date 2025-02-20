#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face_type.h"
#include "PixelTheater/model/region_type.h"

namespace PixelTheater {
namespace Fixtures {

// Simple two-face model with minimal LEDs to test relationships
struct RelationshipsTestModel : public ModelDefinition<12, 2> {  // 12 LEDs, 2 faces
    static constexpr const char* NAME = "Test Relationships Model";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Model for testing relationships";
    static constexpr const char* MODEL_TYPE = "Triangle";  // Simple face type

    // One face type: triangle with center + ring + edges
    static constexpr FaceTypeData FACE_TYPES[] = {
        {
            .id = 0,
            .type = FaceType::Triangle,
            .num_leds = 6,        // 1 center + 3 ring + 2 edge
            .edge_length_mm = 10.0f,
            .num_centers = 1,
            .num_rings = 1,
            .num_edges = 2
        }
    };

    // Two identical faces, opposite orientations
    static constexpr FaceData FACES[] = {
        {0, 0, 0,  0.0f,  0.0f,  1.0f},  // Face 0: top
        {1, 0, 2,  0.0f,  0.0f, -1.0f}   // Face 1: bottom, rotated 120°
    };

    // Points - 6 per face in a simple triangle pattern
    static constexpr PointData POINTS[] = {
        // Face 0
        {0, 0,   0.0f,  0.0f,  1.0f},  // Center
        {1, 0,   1.0f,  0.0f,  1.0f},  // Ring point 1
        {2, 0,  -0.5f,  0.87f, 1.0f},  // Ring point 2
        {3, 0,  -0.5f, -0.87f, 1.0f},  // Ring point 3
        {4, 0,   2.0f,  0.0f,  1.0f},  // Edge point 1
        {5, 0,  -1.0f,  1.73f, 1.0f},  // Edge point 2

        // Face 1 (mirrored)
        {6,  1,   0.0f,  0.0f, -1.0f},  // Center
        {7,  1,   1.0f,  0.0f, -1.0f},  // Ring point 1
        {8,  1,  -0.5f,  0.87f,-1.0f},  // Ring point 2
        {9,  1,  -0.5f, -0.87f,-1.0f},  // Ring point 3
        {10, 1,   2.0f,  0.0f, -1.0f},  // Edge point 1
        {11, 1,  -1.0f,  1.73f,-1.0f}   // Edge point 2
    };

    // Regions - define center, ring, edges for each face
    static constexpr RegionData REGIONS[] = {
        // Face 0
        {0, 0, RegionType::Center, 1, {0}},                // Center
        {1, 0, RegionType::Ring,   3, {1, 2, 3}},         // Ring
        {2, 0, RegionType::Edge,   2, {4, 5}},            // Edge

        // Face 1
        {3, 1, RegionType::Center, 1, {6}},               // Center
        {4, 1, RegionType::Ring,   3, {7, 8, 9}},         // Ring
        {5, 1, RegionType::Edge,   2, {10, 11}}           // Edge
    };

    // Define neighbor relationships
    static constexpr NeighborData NEIGHBORS[] = {
        // Face 0 center neighbors
        {0, {{1, 1.0f}, {2, 1.0f}, {3, 1.0f}}},  // Center to ring points
        // Face 1 center neighbors
        {6, {{7, 1.0f}, {8, 1.0f}, {9, 1.0f}}}   // Center to ring points
    };
};

}} // namespace PixelTheater::Test
