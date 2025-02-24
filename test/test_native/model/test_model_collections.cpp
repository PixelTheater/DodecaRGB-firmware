#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model - Collections") {
    TEST_CASE("LED indexing and access") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());

        SUBCASE("face-local to global indexing") {
            model.leds[7] = CRGB::Green;
            CHECK(model.faces[1].leds[2] == CRGB::Green);  // LED 7 is index 2 on face 1

            model.faces[2].leds[3] = CRGB::Blue;
            CHECK(model.leds[13] == CRGB::Blue);  // LED 13 is index 3 on face 2
        }

        SUBCASE("face boundaries") {
            // Set each face a different color
            for(size_t i = 0; i < model.face_count(); i++) {
                fill_solid(model.faces[i].leds, CRGB(i * 50, 0, 0));  // Different red shades
            }
            // Check face boundaries are respected
            CHECK(model.leds[4] != model.leds[5]);   // Face 0/1 boundary
            CHECK(model.leds[9] != model.leds[10]);  // Face 1/2 boundary
        }
    }

    TEST_CASE("LED collections") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());

        SUBCASE("array access") {
            model.leds[0] = CRGB::Red;
            CHECK(model.leds[0] == CRGB::Red);  // Direct access
            CHECK(model.faces[0].leds[0] == CRGB::Red);  // Same LED through face
        }
    }

    TEST_CASE("point collections") {
        // Point collection tests...
    }

    TEST_CASE("face collections") {
        // Face collection tests...
    }

    TEST_CASE("LED indexing") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());
        CHECK(model.led_count() == 15);  // ensure test conditions are met
        CHECK(model.face_count() == 3);

        // Test same LED through different index spaces
        model.leds[3] = CRGB::Red;
        CHECK(model.faces[0].leds[3] == CRGB::Red);  // Same LED, different index space

        // Test face-local indexing
        model.leds[8] = CRGB::Blue;
        CHECK(model.faces[1].leds[3] == CRGB::Blue); // global LED 8 is LED 3 on Face 1 (offset +5)
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

    TEST_CASE("Collection operations") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());

        SUBCASE("size operations") {
            CHECK(model.leds.size() == model.led_count());
            CHECK(model.faces.size() == model.face_count());
            CHECK(model.points.size() == model.led_count());
        }

        SUBCASE("range-based iteration") {
            // Fill using iteration
            for(auto& led : model.leds) {
                led = CRGB::Blue;
            }
            CHECK(model.leds[0] == CRGB::Blue);
            CHECK(model.leds[model.led_count()-1] == CRGB::Blue);

            // Face iteration
            for(auto& face : model.faces) {
                fill_solid(face.leds, CRGB::Red);
            }
            CHECK(model.leds[0] == CRGB::Red);
        }
    }
} 