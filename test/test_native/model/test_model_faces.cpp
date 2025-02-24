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
} 