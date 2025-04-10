#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model Platform Integration") {
    TEST_CASE("Model with Platform LEDs") {
        const uint16_t NUM_LEDS = BasicPentagonModel::LED_COUNT;
        NativePlatform platform(NUM_LEDS);
        BasicPentagonModel def;
        
        platform.clear();  // Start with known state
        Model<BasicPentagonModel> model(platform.getLEDs());
        
        SUBCASE("Platform Requirements") {
            // Verify platform provides required LED array
            CHECK(platform.getLEDs() != nullptr);
            CHECK(platform.getNumLEDs() == NUM_LEDS);
        }

        SUBCASE("Model-Platform Integration") {
            // Verify model and platform share LED array
            auto* platform_leds = platform.getLEDs();
            auto* model_leds = &model.leds[0];
            CHECK(platform_leds == model_leds);
        }
    }

    TEST_CASE("platform LEDs are correctly initialized") {
        BasicPentagonModel def;
        NativePlatform platform(def.LED_COUNT);
        Model<BasicPentagonModel> model(platform.getLEDs());

        CHECK(platform.getNumLEDs() == def.LED_COUNT);
    }
} 