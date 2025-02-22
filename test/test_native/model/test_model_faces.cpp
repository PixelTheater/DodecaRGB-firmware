#include <doctest/doctest.h>
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model - Face Operations") {
    TEST_CASE("face LED indexing") {
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);

        // Our test model has 1 face with 20 LEDs
        const auto& face = model.faces()[0];
        
        // Check face properties
        CHECK(face.id() == 0);
        CHECK(face.led_count() == 20);
        CHECK(face.led_offset() == 0);

        // Test local-to-global LED access
        model.leds[0] = CRGB::Red;             // Global LED 0
        CHECK(face.leds[0] == CRGB::Red);      // Should be same LED

        // Test writing through face LED access
        face.leds[5] = CRGB::Blue;             // Local LED 5
        CHECK(model.leds[5] == CRGB::Blue);    // Should be global LED 5
    }

    TEST_CASE("face LED bounds") {
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);
        const auto& face = model.faces()[0];

        // Set last LED in face
        uint16_t last_idx = face.led_count() - 1;
        face.leds[last_idx] = CRGB::Red;

        // Out of bounds should return last LED
        CHECK(face.leds[last_idx + 1] == CRGB::Red);
        CHECK(face.leds[0xFFFF] == CRGB::Red);
    }
} 