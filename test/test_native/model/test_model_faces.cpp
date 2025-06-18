#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"
#include "../helpers/model_test_fixture.h"
#include "PixelTheater/color/definitions.h"
#include "PixelTheater/core/crgb.h"
#include <chrono>

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model Faces") {

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face properties") {
        const Face& f0 = model->face(0);
        CHECK(f0.id() == 0);
        CHECK(f0.led_count() > 0);
        CHECK(f0.led_offset() == 0);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face iteration") {
        size_t total_leds = 0;
        for(size_t i = 0; i < model->faceCount(); ++i) {
            const Face& f = model->face(i);
            CHECK(f.id() == i);
            CHECK(f.led_count() > 0);
            total_leds += f.led_count();
        }
        CHECK(total_leds == platform.getNumLEDs());
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face LED access") {
        CRGB* leds_ptr = platform.getLEDs();
        const Face& f0 = model->face(0);
        leds_ptr[f0.led_offset()] = CRGB::Red;
        leds_ptr[f0.led_offset() + f0.led_count() - 1] = CRGB::Blue;
        
        CHECK(leds_ptr[f0.led_offset()] == CRGB::Red);
        CHECK(leds_ptr[f0.led_offset() + f0.led_count() - 1] == CRGB::Blue);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face LED iteration") {
        const Face& f1 = model->face(1);
        CRGB* leds_ptr = platform.getLEDs();
        for(size_t i = 0; i < f1.led_count(); ++i) {
            leds_ptr[f1.led_offset() + i] = CRGB::Green;
        }

        for(size_t i = 0; i < f1.led_count(); ++i) {
            CHECK(leds_ptr[f1.led_offset() + i] == CRGB::Green);
        }
        const Face& f0 = model->face(0);
        CHECK(leds_ptr[f0.led_offset()] != CRGB::Green);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face boundaries") {
        CRGB* leds_ptr = platform.getLEDs();
        size_t num_faces = model->faceCount();

        for (size_t i = 0; i < num_faces; ++i) {
            const Face& face = model->face(i);
            CRGB color(i * 50, 0, 0);
            for (size_t j = 0; j < face.led_count(); ++j) {
                leds_ptr[face.led_offset() + j] = color;
            }
        }

        for (size_t i = 0; i < num_faces - 1; ++i) {
            const Face& current_face = model->face(i);
            const Face& next_face = model->face(i + 1);
            CHECK(leds_ptr[current_face.led_offset() + current_face.led_count() - 1] !=
                  leds_ptr[next_face.led_offset()]);
        }
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "edge connectivity through interface") {
        // Test edge connectivity methods through IModel interface
        // These methods exist on the interface and can be tested
        

        
        // Test edge count for face 0 (pentagon should have 5 edges)
        uint8_t edge_count = model->face_edge_count(0);
        CHECK(edge_count == 5);
        
        // Test edge connections (might return -1 for no connection or valid face ID)
        int8_t connected_face = model->face_at_edge(0, 0);
        CHECK(connected_face >= -1);  // Valid range: -1 for no connection, or valid face ID
        
        // Test invalid face ID (should handle gracefully)
        uint8_t invalid_edge_count = model->face_edge_count(static_cast<uint8_t>(255));
        CHECK(invalid_edge_count == 0);  // Should return 0 for invalid face
        
        int8_t invalid_connection = model->face_at_edge(static_cast<uint8_t>(255), 0);
        CHECK(invalid_connection == -1);  // Should return -1 for invalid face
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "vertex access through face") {
        // Test vertex access through Face interface
        const Face& f0 = model->face(0);
        
        // Check that vertices are accessible
        CHECK(f0.vertices.size() >= 5);  // Pentagon should have at least 5 vertices
        
        // Check that we can access individual vertices
        const auto& vertex0 = f0.vertices[0];
        // Check that vertex has reasonable coordinates (not all zero)
        bool has_coords = (vertex0.x != 0.0f) || (vertex0.y != 0.0f) || (vertex0.z != 0.0f);
        CHECK(has_coords);
        
        // Test vertex iteration
        size_t vertex_count = 0;
        for (const auto& vertex : f0.vertices) {
            vertex_count++;
            // Basic sanity check - vertices should have reasonable coordinates
            CHECK(std::abs(vertex.x) < 10000.0f);
            CHECK(std::abs(vertex.y) < 10000.0f);
            CHECK(std::abs(vertex.z) < 10000.0f);
            if (vertex_count >= 5) break;  // Only check first 5 for pentagon
        }
        CHECK(vertex_count >= 5);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "vertex access through FaceProxy") {
        // Test vertex access through the FaceProxy from Model::face() method
        // Create a concrete model directly for FaceProxy testing
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        auto face_proxy = concrete_model.face(0);  // Get FaceProxy for geometric position 0
        
        // Test that vertices returns a reference to the face vertices (note: vertices is a member, not a function)
        const auto& vertices = face_proxy.vertices();
        CHECK(vertices.size() >= 5);  // Pentagon should have at least 5 vertices
        
        // Test vertex access
        const auto& vertex0 = vertices[0];
        // Check that vertex has reasonable coordinates (not all zero)
        bool has_coords = (vertex0.x != 0.0f) || (vertex0.y != 0.0f) || (vertex0.z != 0.0f);
        CHECK(has_coords);
        
        // Test vertex iteration through proxy
        size_t vertex_count = 0;
        for (const auto& vertex : vertices) {
            vertex_count++;
            // Vertices should have reasonable coordinates
            CHECK(std::abs(vertex.x) < 10000.0f);
            CHECK(std::abs(vertex.y) < 10000.0f);
            CHECK(std::abs(vertex.z) < 10000.0f);
            if (vertex_count >= 5) break;  // Only check first 5 for pentagon
        }
        CHECK(vertex_count >= 5);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "edge center calculation") {
        // Test calculating edge centers from vertices
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        auto face_proxy = concrete_model.face(0);  // Get FaceProxy for geometric position 0
        const auto& vertices = face_proxy.vertices();
        
        // Calculate center of first edge (vertex 0 to vertex 1)
        const auto& v0 = vertices[0];
        const auto& v1 = vertices[1];
        
        // Edge center is midpoint between two vertices
        float edge_center_x = (v0.x + v1.x) / 2.0f;
        float edge_center_y = (v0.y + v1.y) / 2.0f;
        float edge_center_z = (v0.z + v1.z) / 2.0f;
        
        // Sanity check - edge center should be reasonable
        CHECK(std::abs(edge_center_x) < 10000.0f);
        CHECK(std::abs(edge_center_y) < 10000.0f);
        CHECK(std::abs(edge_center_z) < 10000.0f);
        
        // Edge center should be between the two vertices
        CHECK(edge_center_x >= std::min(v0.x, v1.x));
        CHECK(edge_center_x <= std::max(v0.x, v1.x));
        CHECK(edge_center_y >= std::min(v0.y, v1.y));
        CHECK(edge_center_y <= std::max(v0.y, v1.y));
        CHECK(edge_center_z >= std::min(v0.z, v1.z));
        CHECK(edge_center_z <= std::max(v0.z, v1.z));
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "FaceProxy edge_center method") {
        // Test the new edge_center convenience method
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        auto face_proxy = concrete_model.face(0);  // Get FaceProxy for geometric position 0
        
        // Test edge center calculation for first edge
        auto edge_center = face_proxy.edge_center(0);
        
        // Verify the center is reasonable
        CHECK(std::abs(edge_center.x) < 10000.0f);
        CHECK(std::abs(edge_center.y) < 10000.0f);
        CHECK(std::abs(edge_center.z) < 10000.0f);
        
        // Verify it matches manual calculation
        const auto& vertices = face_proxy.vertices();
        const auto& v0 = vertices[0];
        const auto& v1 = vertices[1];
        
        float expected_x = (v0.x + v1.x) / 2.0f;
        float expected_y = (v0.y + v1.y) / 2.0f;
        float expected_z = (v0.z + v1.z) / 2.0f;
        
        CHECK(std::abs(edge_center.x - expected_x) < 0.001f);
        CHECK(std::abs(edge_center.y - expected_y) < 0.001f);
        CHECK(std::abs(edge_center.z - expected_z) < 0.001f);
        
        // Test edge wraparound (last edge should connect back to first vertex)
        uint8_t edge_count = face_proxy.edge_count();
        auto last_edge_center = face_proxy.edge_center(edge_count - 1);
        
        // Last edge should be from last vertex to first vertex
        const auto& v_last = vertices[edge_count - 1];
        const auto& v_first = vertices[0];
        
        float expected_last_x = (v_last.x + v_first.x) / 2.0f;
        float expected_last_y = (v_last.y + v_first.y) / 2.0f;
        float expected_last_z = (v_last.z + v_first.z) / 2.0f;
        
        CHECK(std::abs(last_edge_center.x - expected_last_x) < 0.001f);
        CHECK(std::abs(last_edge_center.y - expected_last_y) < 0.001f);
        CHECK(std::abs(last_edge_center.z - expected_last_z) < 0.001f);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "edge adjacency validation") {
        // Test that edge adjacency relationships are consistent
        size_t num_faces = model->faceCount();
        
        for (size_t face_idx = 0; face_idx < num_faces; face_idx++) {
            uint8_t edge_count = model->face_edge_count(face_idx);
            CHECK(edge_count == 5);  // Pentagon should have 5 edges
            
            for (uint8_t edge_idx = 0; edge_idx < edge_count; edge_idx++) {
                int8_t adjacent_face = model->face_at_edge(face_idx, edge_idx);
                
                if (adjacent_face >= 0) {
                    // Adjacent face should be valid
                    CHECK(adjacent_face < static_cast<int8_t>(num_faces));
                    CHECK(adjacent_face != static_cast<int8_t>(face_idx));  // Face shouldn't connect to itself
                }
                // adjacent_face == -1 is valid (no connection)
            }
        }
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "FaceProxy nearby_leds method") {
        // Test the new nearby_leds method
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        auto face_proxy = concrete_model.face(0);
        
        // Test with a vertex point - should find some nearby LEDs
        const auto& vertices = face_proxy.vertices();
        CHECK(vertices.size() >= 3);  // Should have vertices
        
        // Test finding LEDs near first vertex
        auto nearby = face_proxy.nearby_leds(vertices[0]);
        CHECK(nearby.size() > 0);  // Should find some LEDs
        CHECK(nearby.size() <= face_proxy.led_count());  // Shouldn't exceed face LED count
        
        // Results should be sorted by distance
        for (size_t i = 1; i < nearby.size(); i++) {
            CHECK(nearby[i].distance >= nearby[i-1].distance);
        }
        
        // Test with distance limit
        auto nearby_close = face_proxy.nearby_leds(vertices[0], 50.0f);
        CHECK(nearby_close.size() <= nearby.size());  // Should be subset
        
        // All distances should be within limit
        for (const auto& led : nearby_close) {
            CHECK(led.distance <= 50.0f);
        }
        
        // Test LED indices are valid
        for (const auto& led : nearby) {
            CHECK(led.led_index >= face_proxy.led_offset());
            CHECK(led.led_index < face_proxy.led_offset() + face_proxy.led_count());
        }
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "FaceProxy vertex_midpoint method") {
        // Test vertex midpoint calculation
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        auto face_proxy = concrete_model.face(0);
        
        const auto& vertices = face_proxy.vertices();
        CHECK(vertices.size() >= 2);  // Need at least 2 vertices
        
        // Calculate midpoint between first two vertices
        auto midpoint = face_proxy.vertex_midpoint(0, 1);
        
        const auto& v0 = vertices[0];
        const auto& v1 = vertices[1];
        
        // Verify midpoint calculation
        float expected_x = (v0.x + v1.x) / 2.0f;
        float expected_y = (v0.y + v1.y) / 2.0f;
        float expected_z = (v0.z + v1.z) / 2.0f;
        
        CHECK(std::abs(midpoint.x - expected_x) < 0.001f);
        CHECK(std::abs(midpoint.y - expected_y) < 0.001f);
        CHECK(std::abs(midpoint.z - expected_z) < 0.001f);
        
        // Test invalid indices
        auto invalid_midpoint = face_proxy.vertex_midpoint(0, 255);
        CHECK(invalid_midpoint.x == 0.0f);
        CHECK(invalid_midpoint.y == 0.0f);
        CHECK(invalid_midpoint.z == 0.0f);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "FaceProxy validate_geometry method") {
        // Test geometry validation
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        auto face_proxy = concrete_model.face(0);
        
        auto validation = face_proxy.validate_geometry();
        
        // Basic checks
        CHECK(validation.has_vertices);
        CHECK(validation.has_leds);
        CHECK(validation.vertices_reasonable);
        CHECK(validation.leds_reasonable);
        CHECK(validation.vertex_count == 5);  // Pentagon
        CHECK(validation.led_count > 0);
        CHECK(validation.face_radius > 0.0f);
        
        // Test all faces
        size_t num_faces = model->faceCount();
        for (size_t i = 0; i < num_faces; i++) {
            auto test_face = concrete_model.face(static_cast<uint8_t>(i));
            auto test_validation = test_face.validate_geometry();
            
            CHECK(test_validation.has_vertices);
            CHECK(test_validation.has_leds);
            CHECK(test_validation.vertices_reasonable);
            CHECK(test_validation.leds_reasonable);
            CHECK(test_validation.vertex_count == 5);  // All pentagons
            CHECK(test_validation.led_count > 0);
            CHECK(test_validation.face_radius > 0.0f);
        }
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "geometric edge detection with nearby_leds") {
        // Test using the new nearby_leds method for edge detection
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        auto face_proxy = concrete_model.face(0);
        
        // First, validate geometry to ensure we have reasonable data
        auto validation = face_proxy.validate_geometry();
        REQUIRE(validation.has_vertices);
        REQUIRE(validation.has_leds);
        REQUIRE(validation.vertices_reasonable);
        REQUIRE(validation.leds_reasonable);
        
        // Get edge center and find nearby LEDs - use a generous distance
        auto edge_center = face_proxy.edge_center(0);
        auto nearby = face_proxy.nearby_leds(edge_center, 1000.0f);  // Use larger distance
        
        // Debug: Print some values to understand the geometry
        INFO("Edge center: (" << edge_center.x << ", " << edge_center.y << ", " << edge_center.z << ")");
        INFO("Face has " << face_proxy.led_count() << " LEDs");
        INFO("Found " << nearby.size() << " LEDs within 1000.0 units");
        
        if (nearby.size() > 0) {
            INFO("Closest LED distance: " << nearby[0].distance);
        }
        
        // Should find at least some LEDs on this face
        CHECK(nearby.size() > 0);  
        
        // Test finding all LEDs on face regardless of distance
        auto all_nearby = face_proxy.nearby_leds(edge_center);  // No distance limit
        CHECK(all_nearby.size() == face_proxy.led_count());  // Should find all LEDs on face
        
        // Test finding different numbers of LEDs for different face IDs
        // Simulate the identify_sides algorithm
        for (uint8_t num_leds = 1; num_leds <= 3; num_leds++) {
            auto limited_nearby = face_proxy.nearby_leds(edge_center);
            // Should find enough LEDs or return all available LEDs
            bool has_enough_leds = (limited_nearby.size() >= num_leds) || (limited_nearby.size() == face_proxy.led_count());
            CHECK(has_enough_leds);
            
            // Take first N LEDs (closest to edge)
            size_t leds_to_use = std::min(static_cast<size_t>(num_leds), limited_nearby.size());
            for (size_t i = 0; i < leds_to_use; i++) {
                CHECK(limited_nearby[i].led_index >= face_proxy.led_offset());
                CHECK(limited_nearby[i].led_index < face_proxy.led_offset() + face_proxy.led_count());
            }
        }
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "identify_sides edge detection demo") {
        // Demonstrate how to use the new geometric methods for identify_sides scene
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        
        // For each face, find edge clusters using geometry
        for (uint8_t face_idx = 0; face_idx < model->faceCount(); face_idx++) {
            auto face_proxy = concrete_model.face(face_idx);
            
            // For each edge on this face
            for (uint8_t edge_idx = 0; edge_idx < face_proxy.edge_count(); edge_idx++) {
                // Calculate edge center point  
                auto edge_center = face_proxy.edge_center(edge_idx);
                
                // Find LEDs close to this edge
                auto edge_leds = face_proxy.nearby_leds(edge_center);
                
                // Verify we found LEDs
                CHECK(edge_leds.size() > 0);
                CHECK(edge_leds.size() <= face_proxy.led_count());
                
                // Take the closest N LEDs to represent this edge
                // (Simulating what identify_sides scene would do)
                uint8_t connected_face_id = face_proxy.face_at_edge(edge_idx);
                uint8_t num_edge_leds = (connected_face_id >= 0) ? (connected_face_id + 1) : 1;
                
                size_t leds_to_use = std::min(static_cast<size_t>(num_edge_leds), edge_leds.size());
                
                // Verify LED indices are valid
                for (size_t i = 0; i < leds_to_use; i++) {
                    CHECK(edge_leds[i].led_index >= face_proxy.led_offset());
                    CHECK(edge_leds[i].led_index < face_proxy.led_offset() + face_proxy.led_count());
                }
                
                // This demonstrates the basic algorithm:
                // 1. Get edge center with face_proxy.edge_center(edge_idx)
                // 2. Find nearby LEDs with face_proxy.nearby_leds(edge_center)  
                // 3. Take first N LEDs based on connected face ID
                // 4. Light those LEDs in the scene
            }
        }
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "Model validation - basic functionality") {
        // Test the new model validation system
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        
        // Test data integrity validation only
        auto data_validation = concrete_model.validate_model(false, true);
        CHECK(data_validation.total_checks > 0);
        CHECK(data_validation.data_integrity.face_ids_unique);
        CHECK(data_validation.data_integrity.led_indices_sequential);
        CHECK(data_validation.data_integrity.edge_data_complete);
        CHECK(data_validation.data_integrity.vertex_data_complete);
        CHECK(data_validation.data_integrity.indices_in_bounds);
        
        // Test geometric validation only  
        auto geom_validation = concrete_model.validate_model(true, false);
        CHECK(geom_validation.total_checks > 0);
        CHECK(geom_validation.geometric.vertex_coordinates_sane);
        CHECK(geom_validation.geometric.led_coordinates_sane);
        
        // Test full validation
        auto full_validation = concrete_model.validate_model(true, true);
        CHECK(full_validation.total_checks > data_validation.total_checks);
        CHECK(full_validation.total_checks > geom_validation.total_checks);
        
        // Check overall validation status
        INFO("Validation errors: " << full_validation.failed_checks << "/" << full_validation.total_checks);
        if (full_validation.errors.error_count > 0) {
            for (uint8_t i = 0; i < full_validation.errors.error_count; i++) {
                INFO("Error " << i << ": " << full_validation.errors.error_messages[i]);
            }
        }
        
        // Model should be valid for our test fixture
        CHECK(full_validation.is_valid);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "Model validation - interface consistency") {
        // Test validation through IModel interface via ModelWrapper
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        auto wrapper = std::make_unique<ModelWrapper<BasicPentagonModel>>(
            std::make_unique<Model<BasicPentagonModel>>(platform.getLEDs())
        );
        
        // Test through interface
        auto interface_validation = wrapper->validate_model();
        CHECK(interface_validation.total_checks > 0);
        
        // Compare with direct validation
        auto direct_validation = concrete_model.validate_model();
        CHECK(interface_validation.is_valid == direct_validation.is_valid);
        CHECK(interface_validation.total_checks == direct_validation.total_checks);
        CHECK(interface_validation.failed_checks == direct_validation.failed_checks);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "Model validation - error detection") {
        // Test that validation can detect issues in a broken model
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        
        // Test geometric validation detects reasonable issues
        auto validation = concrete_model.validate_model();
        
        // Check validation structure integrity
        CHECK(validation.total_checks >= 5);  // Should have at least several checks
        
        // Check error reporting system works
        CHECK(validation.errors.error_count < IModel::ModelValidation::ErrorDetails::MAX_ERRORS);
        
        // Verify validation categories are properly tested
        bool tested_geometric = (validation.geometric.vertex_coordinates_sane || 
                               validation.geometric.led_coordinates_sane ||
                               validation.geometric.all_faces_planar);
        bool tested_data_integrity = (validation.data_integrity.face_ids_unique ||
                                    validation.data_integrity.led_indices_sequential ||
                                    validation.data_integrity.edge_data_complete);
        
        CHECK(tested_geometric);
        CHECK(tested_data_integrity);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "Model validation - performance check") {
        // Test that validation doesn't take unreasonably long
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Run full validation multiple times
        for (int i = 0; i < 5; i++) {
            auto validation = concrete_model.validate_model(true, true);
            CHECK(validation.total_checks > 0);
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // Should complete quickly (less than 100ms for 5 runs on a small model)
        INFO("5 validation runs took: " << duration.count() << "ms");
        CHECK(duration.count() < 100);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "Model validation - selective validation") {
        // Test selective validation flags
        Model<BasicPentagonModel> concrete_model(platform.getLEDs());
        
        // Test geometry-only validation
        auto geom_only = concrete_model.validate_model(true, false);
        CHECK(geom_only.total_checks > 0);
        CHECK(geom_only.data_integrity.face_ids_unique == false);  // Should be default/unset
        
        // Test data-only validation  
        auto data_only = concrete_model.validate_model(false, true);
        CHECK(data_only.total_checks > 0);
        CHECK(data_only.geometric.vertex_coordinates_sane == false);  // Should be default/unset
        
        // Test no validation
        auto no_validation = concrete_model.validate_model(false, false);
        CHECK(no_validation.total_checks == 0);
        CHECK(no_validation.failed_checks == 0);
        CHECK(no_validation.is_valid == true);  // No failures = valid
    }
} 