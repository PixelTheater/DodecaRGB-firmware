#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model - Face Operations") {
    TEST_CASE("local face LED indexing") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());

        // Check face 0
        const auto& face = model.faces[0];
        
        // Check face properties
        CHECK(face.id() == 0);
        CHECK(face.led_count() == 5);
        CHECK(face.led_offset() == 0);

        // Test local-to-global LED access
        model.leds[0] = CRGB::Red;             // Global LED 0
        CHECK(face.leds[0] == CRGB::Red);      // Should be same LED

        // Test writing through face LED access
        face.leds[3] = CRGB::Blue;             // Local LED 5
        CHECK(model.leds[3] == CRGB::Blue);    // Should be global LED 5
    }

    TEST_CASE("face LED bounds") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());

        const auto& face = model.faces[0];

        // Set last LED in face
        uint16_t last_idx = face.led_count() - 1;
        face.leds[last_idx] = CRGB::Red;

        // Out of bounds should return last LED
        CHECK(face.leds[last_idx + 1] == CRGB::Red);
        CHECK(face.leds[0xFFFF] == CRGB::Red);
    }

    TEST_CASE("face indexing relationships") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());


        SUBCASE("local to global index mapping") {
            // Check face 1 
            model.leds[7] = CRGB::Green;
            CHECK(model.faces[1].leds[2] == CRGB::Green);  // LED 7 is index 2 on face 1

            // Check face 2
            model.leds[12] = CRGB::Blue;
            CHECK(model.faces[2].leds[2] == CRGB::Blue);  // LED 12 is index 2 on face 2

            // Check writing through face access
            model.faces[1].leds[3] = CRGB::White;  // Should be LED 8
            CHECK(model.leds[8] == CRGB::White);
        }

        SUBCASE("face led iteration") {
            auto& face = model.faces[0];
            for(auto& led : face.leds) {
                led = CRGB::Green;
            }
            CHECK(model.leds[face.led_offset()] == CRGB::Green);
        }
    }

    TEST_CASE("face operations") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());


        // Fill all LEDs in each face
        for(auto& face : model.faces) {
            fill_solid(face.leds, CRGB::Red);
        }
        CHECK(model.leds[0] == CRGB::Red);
    }

    TEST_CASE("face vertex access") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());

        // Check face 0 vertices
        const auto& face0 = model.faces[0];
        CHECK(face0.vertices[0].x == doctest::Approx(0.0f));   // Center x
        CHECK(face0.vertices[0].y == doctest::Approx(0.0f));   // Center y
        CHECK(face0.vertices[0].z == doctest::Approx(1.0f));   // Center z

        CHECK(face0.vertices[1].x == doctest::Approx(1.0f));   // Right vertex x
        CHECK(face0.vertices[1].y == doctest::Approx(0.0f));   // Right vertex y
        CHECK(face0.vertices[1].z == doctest::Approx(1.0f));   // Right vertex z

        // Check face 1 vertices (middle face)
        const auto& face1 = model.faces[1];
        for(const auto& vertex : face1.vertices) {
            CHECK(vertex.z == doctest::Approx(0.0f));  // All vertices should be at z=0
        }

        // Check face 2 vertices (bottom face)
        const auto& face2 = model.faces[2];
        for(const auto& vertex : face2.vertices) {
            CHECK(vertex.z == doctest::Approx(-1.0f));  // All vertices should be at z=-1
        }

        // Verify vertex count for each face
        for(const auto& face : model.faces) {
            CHECK(face.vertices.size() == Limits::MAX_EDGES_PER_FACE);
        }

        // Test vertex array bounds checking
        const auto& face = model.faces[0];
        const auto& last_valid_vertex = face.vertices[Limits::MAX_EDGES_PER_FACE - 1];
        
        // Should return last valid vertex when out of bounds
        const auto& out_of_bounds = face.vertices[Limits::MAX_EDGES_PER_FACE];
        CHECK(out_of_bounds.x == last_valid_vertex.x);
        CHECK(out_of_bounds.y == last_valid_vertex.y);
        CHECK(out_of_bounds.z == last_valid_vertex.z);

        const auto& large_index = face.vertices[0xFFFF];
        CHECK(large_index.x == last_valid_vertex.x);
        CHECK(large_index.y == last_valid_vertex.y);
        CHECK(large_index.z == last_valid_vertex.z);
    }
} 