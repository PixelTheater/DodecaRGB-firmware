#include <doctest/doctest.h>
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model - Collections") {
    TEST_CASE("LED collections") {
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);

        SUBCASE("fill operations") {
            // Fill entire array
            model.leds.fill(CRGB::Blue);
            CHECK(model.leds[0] == CRGB::Blue);

            // Fill a face
            auto& face = model.faces[0];
            face.leds.fill(CRGB::Red);
            CHECK(face.leds[0] == CRGB::Red);
        }

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
        Model<BasicPentagonModel> model(def);
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
        Model<BasicPentagonModel> model(def);

        // Fill all LEDs in each face
        for(auto& face : model.faces) {
            face.leds.fill(CRGB::Red);
        }
        CHECK(model.leds[0] == CRGB::Red);
    }

    TEST_CASE("Collection operations") {
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);

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
                face.leds.fill(CRGB::Red);
            }
            CHECK(model.leds[0] == CRGB::Red);
        }
    }
} 