#include <doctest/doctest.h>
#include "PixelTheater/model/model.h"
#include "PixelTheater/core/crgb.h"  // Need full CRGB definition
#include "PixelTheater/model/face_type.h"  // For FaceType enum
#include "fixtures/models/led_test_model.h"

using namespace PixelTheater;

// Simple test model definition for runtime access testing
struct TestModel : public ModelDefinition<6, 2> {
    static constexpr size_t LED_COUNT = 6;
    static constexpr size_t FACE_COUNT = 2;
    
    // Hardware metadata
    struct HardwareData {
        char led_type[16];
        char color_order[8];
        float led_diameter_mm;
        float led_spacing_mm;
        uint16_t max_current_per_led_ma;
        uint16_t avg_current_per_led_ma;
    };
    static constexpr HardwareData HARDWARE{"WS2812B", "GRB", 1.6f, 4.5f, 20, 10};
    
    // LED Groups
    struct LedGroupData {
        char name[16];
        uint8_t face_type_id;
        uint8_t led_count;
        uint16_t led_indices[32];
    };
    
    static constexpr std::array<LedGroupData, 3> LED_GROUPS{{
        {
            .name = "center",
            .face_type_id = 0,
            .led_count = 1,
            .led_indices = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        },
        {
            .name = "ring0",
            .face_type_id = 0,
            .led_count = 2,
            .led_indices = {1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        },
        {
            .name = "edge0",
            .face_type_id = 0,
            .led_count = 1,
            .led_indices = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        }
    }};
    
    // Edge data
    struct EdgeData {
        uint8_t face_id;
        uint8_t edge_index;
        struct Point3D {
            float x, y, z;
        };
        Point3D start_vertex;
        Point3D end_vertex;
        int8_t connected_face_id;
    };
    
    static constexpr std::array<EdgeData, 4> EDGES{{
        {
            .face_id = 0,
            .edge_index = 0,
            .start_vertex = {0.0f, 0.0f, 100.0f},
            .end_vertex = {50.0f, 0.0f, 100.0f},
            .connected_face_id = 1
        },
        {
            .face_id = 0,
            .edge_index = 1,
            .start_vertex = {50.0f, 0.0f, 100.0f},
            .end_vertex = {50.0f, 50.0f, 100.0f},
            .connected_face_id = -1  // No connection
        },
        {
            .face_id = 1,
            .edge_index = 0,
            .start_vertex = {0.0f, 0.0f, 50.0f},
            .end_vertex = {50.0f, 0.0f, 50.0f},
            .connected_face_id = 0
        },
        {
            .face_id = 1,
            .edge_index = 1,
            .start_vertex = {50.0f, 0.0f, 50.0f},
            .end_vertex = {50.0f, 50.0f, 50.0f},
            .connected_face_id = -1  // No connection
        }
    }};
    
    // Face type data
    struct FaceTypeData {
        uint8_t id;
        FaceType type;  // Add missing type field
        uint16_t num_leds;
        float edge_length_mm;
    };
    
    static constexpr std::array<FaceTypeData, 1> FACE_TYPES{{
        {.id = 0, .type = FaceType::Pentagon, .num_leds = 3, .edge_length_mm = 60.0f}
    }};
    
    // Face data
    struct FaceData {
        uint8_t id;
        uint8_t type_id;
        uint8_t rotation;
        uint8_t geometric_id;
        struct Vertex {
            float x, y, z;
        };
        Vertex vertices[5];
    };
    
    static constexpr std::array<FaceData, FACE_COUNT> FACES{{
        {
            .id = 0,
            .type_id = 0,
            .rotation = 0,
            .geometric_id = 0,
            .vertices = {}
        },
        {
            .id = 1,
            .type_id = 0,
            .rotation = 0,
            .geometric_id = 1,
            .vertices = {}
        }
    }};
    
    // Point data
    struct PointData {
        uint16_t id;
        uint8_t face_id;
        float x, y, z;
    };
    
    static constexpr PointData POINTS[] = {
        {0, 0, 0.0f, 0.0f, 100.0f},
        {1, 0, 10.0f, 0.0f, 100.0f},
        {2, 0, 0.0f, 10.0f, 100.0f},
        {3, 1, 0.0f, 0.0f, 50.0f},
        {4, 1, 10.0f, 0.0f, 50.0f},
        {5, 1, 0.0f, 10.0f, 50.0f}
    };
    
    // Neighbor data
    struct NeighborData {
        uint16_t point_id;
        struct Neighbor {
            uint16_t id;
            float distance;
        };
        static constexpr size_t MAX_NEIGHBORS = 2;
        Neighbor neighbors[MAX_NEIGHBORS];
    };
    
    static constexpr NeighborData NEIGHBORS[] = {};
};

TEST_CASE("Model runtime access methods") {
    // Create test model with LED array
    CRGB led_array[TestModel::LED_COUNT];
    Model<TestModel> model(led_array);
    
    // Initialize LEDs with test colors
    for (size_t i = 0; i < TestModel::LED_COUNT; i++) {
        led_array[i] = CRGB(i * 40, 0, 0);  // Red gradient
    }
    
    SUBCASE("LED Group Access") {
        // Test face-centric group access (NEW API)
        auto face0 = model.face(0);
        auto center_group = face0.group("center");
        CHECK(center_group.size() == 1);
        CHECK(center_group.led_count == 1);
        
        // Test group LED access
        center_group[0] = CRGB::Blue;
        CHECK(led_array[0] == CRGB::Blue);
        
        // Test ring0 group for face 0
        auto ring0_group = face0.group("ring0");
        CHECK(ring0_group.size() == 2);
        CHECK(ring0_group.led_count == 2);
        
        // Test group iteration
        uint8_t count = 0;
        for (auto& led : ring0_group) {
            led = CRGB::Green;
            count++;
        }
        CHECK(count == 2);
        CHECK(led_array[1] == CRGB::Green);
        CHECK(led_array[2] == CRGB::Green);
        
        // Test edge0 group for face 0
        auto edge0_group = face0.group("edge0");
        CHECK(edge0_group.size() == 1);
        CHECK(edge0_group.led_count == 1);
        
        // Test nonexistent group
        auto invalid_group = face0.group("nonexistent");
        CHECK(invalid_group.size() == 0);
        CHECK(invalid_group.led_count == 0);
    }
    
    SUBCASE("LED Group Access by Face") {
        // Test face-centric group access for different faces (NEW API)
        auto face0 = model.face(0);
        auto face1 = model.face(1);
        
        auto center_face0 = face0.group("center");
        CHECK(center_face0.size() == 1);
        
        auto center_face1 = face1.group("center");
        CHECK(center_face1.size() == 1);
        
        // Modify center of each face with different colors
        center_face0[0] = CRGB::Red;
        center_face1[0] = CRGB::Blue;
        
        // Verify the LEDs are in different positions
        CHECK(led_array[0] == CRGB::Red);   // Face 0 center
        CHECK(led_array[3] == CRGB::Blue);  // Face 1 center
        
        // Test invalid face ID - should return clamped face (not crash)
        auto invalid_face = model.face(99);
        auto invalid_group = invalid_face.group("center");
        CHECK(invalid_group.size() >= 0);  // Should not crash
    }
    
    SUBCASE("Edge Access") {
        // Test edge access by index
        auto edge0 = model.edges(0);
        CHECK(edge0.face_id == 0);
        CHECK(edge0.edge_index == 0);
        CHECK(edge0.start_vertex.x == 0.0f);
        CHECK(edge0.start_vertex.y == 0.0f);
        CHECK(edge0.start_vertex.z == 100.0f);
        CHECK(edge0.end_vertex.x == 50.0f);
        CHECK(edge0.connected_face_id == 1);
        CHECK(edge0.has_connection() == true);
        
        // Test edge without connection
        auto edge1 = model.edges(1);
        CHECK(edge1.face_id == 0);
        CHECK(edge1.connected_face_id == -1);
        CHECK(edge1.has_connection() == false);
        
        // Test face-centric edge access (NEW API)
        auto face0 = model.face(0);
        auto face1 = model.face(1);
        
        CHECK(face0.face_at_edge(0) == 1);  // Face 0, edge 0 connects to face 1
        CHECK(face0.face_at_edge(1) == -1); // Face 0, edge 1 has no connection
        CHECK(face1.face_at_edge(0) == 0);  // Face 1, edge 0 connects to face 0
        CHECK(face1.face_at_edge(1) == -1); // Face 1, edge 1 has no connection
        
        // Test edge count for specific faces (NEW API)
        CHECK(face0.edge_count() == 5);  // Pentagon has 5 edges
        CHECK(face1.edge_count() == 5);  // Pentagon has 5 edges
        
        // Test invalid face (should clamp, not crash)
        auto invalid_face = model.face(99);
        CHECK(invalid_face.edge_count() >= 0);  // Should not crash
    }
    
    SUBCASE("Face Edges") {
        // Test getting all edges for face 0 (NEW API)
        auto face0 = model.face(0);
        auto face0_edges = face0.edges();
        
        uint8_t edge_count = 0;
        for (const auto& edge : face0_edges) {
            CHECK(edge.face_id == 0);
            edge_count++;
        }
        CHECK(edge_count == 2);  // Face 0 has 2 edges
        
        // Test getting all edges for face 1 (NEW API)
        auto face1 = model.face(1);
        auto face1_edges = face1.edges();
        
        edge_count = 0;
        for (const auto& edge : face1_edges) {
            CHECK(edge.face_id == 1);
            edge_count++;
        }
        CHECK(edge_count == 2);  // Face 1 has 2 edges
    }
    
    SUBCASE("Face Groups Enumeration") {
        // Test getting all group names for a face (NEW API)
        auto face0 = model.face(0);
        auto groups = face0.groups();
        
        // Should have the 3 groups we defined: center, ring0, edge0
        CHECK(groups.size() == 3);
        
        // Check group names (order not guaranteed, so check if they exist)
        bool found_center = false, found_ring0 = false, found_edge0 = false;
        for (size_t i = 0; i < groups.size(); i++) {
            const char* group_name = groups[i];
            if (group_name != nullptr) {
                if (std::string(group_name) == "center") found_center = true;
                if (std::string(group_name) == "ring0") found_ring0 = true;
                if (std::string(group_name) == "edge0") found_edge0 = true;
            }
        }
        CHECK(found_center);
        CHECK(found_ring0);
        CHECK(found_edge0);
        
        // Test iteration
        uint8_t count = 0;
        for (const char* group_name : groups) {
            CHECK(group_name != nullptr);
            count++;
        }
        CHECK(count == 3);
    }
    
    SUBCASE("Hardware Metadata Access") {
        // Test hardware metadata access
        auto hw = model.hardware();
        
        // Test LED type
        const char* led_type = hw.led_type();
        CHECK(std::string(led_type) == "WS2812B");
        
        // Test color order
        const char* color_order = hw.color_order();
        CHECK(std::string(color_order) == "GRB");
        
        // Test dimensions and power
        CHECK(hw.led_diameter_mm() == 1.6f);
        CHECK(hw.led_spacing_mm() == 4.5f);
        CHECK(hw.max_current_per_led_ma() == 20);
        CHECK(hw.avg_current_per_led_ma() == 10);
    }
    
    SUBCASE("Model Size Information") {
        // Test size accessors
        CHECK(model.led_count() == 6);
        CHECK(model.face_count() == 2);
        // Global edge_count() and group_count() removed - use face-specific methods instead
    }
}

TEST_CASE("Model group name matching") {
    CRGB led_array[TestModel::LED_COUNT];
    Model<TestModel> model(led_array);
    
    SUBCASE("Exact name matching") {
        // Test exact matches for face 0
        auto center = model.group("center", 0);
        CHECK(center.size() == 1);
        
        auto ring0 = model.group("ring0", 0);
        CHECK(ring0.size() == 2);
        
        auto edge0 = model.group("edge0", 0);
        CHECK(edge0.size() == 1);
    }
    
    SUBCASE("Case sensitivity") {
        // Should not match different case
        auto invalid1 = model.group("CENTER", 0);
        CHECK(invalid1.size() == 0);
        
        auto invalid2 = model.group("Ring0", 0);
        CHECK(invalid2.size() == 0);
    }
    
    SUBCASE("Partial and overflow names") {
        // Should not match partial names
        auto invalid1 = model.group("cent", 0);
        CHECK(invalid1.size() == 0);
        
        auto invalid2 = model.group("ring", 0);
        CHECK(invalid2.size() == 0);
        
        // Should not match names that are too long
        auto invalid3 = model.group("centerTooLong", 0);
        CHECK(invalid3.size() == 0);
    }
} 