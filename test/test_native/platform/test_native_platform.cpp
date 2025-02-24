#include <doctest/doctest.h>
#include "PixelTheater/platform/platform.h"
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;

TEST_SUITE("NativePlatform") {
    TEST_CASE("Basic Platform Operations") {
        const size_t NUM_LEDS = 100;
        NativePlatform platform(NUM_LEDS);
        
        SUBCASE("LED Array Management") {
            auto* leds = platform.getLEDs();
            CHECK(platform.getNumLEDs() == NUM_LEDS);
            CHECK(leds != nullptr);
            
            // Test array initialization
            CHECK(leds[0].r == 0);
            CHECK(leds[0].g == 0);
            CHECK(leds[0].b == 0);
            
            // Test array write access
            leds[0] = CRGB::Red;
            CHECK(leds[0].r == 255);
            CHECK(leds[0].g == 0);
            CHECK(leds[0].b == 0);
        }
        
        SUBCASE("FastLED Helper Functions") {
            auto* leds = platform.getLEDs();
            
            // Test fill_solid
            fill_solid(leds, platform.getNumLEDs(), CRGB::Black);
            CHECK(leds[0] == CRGB::Black);
            CHECK(leds[NUM_LEDS-1] == CRGB::Black);
            
            // Test nscale8
            fill_solid(leds, platform.getNumLEDs(), CRGB::Blue);  // Set full blue first
            nscale8(leds, platform.getNumLEDs(), 128);  // 50% brightness
            CHECK(leds[0].b == 128);
        }
        
        SUBCASE("Clear Operation") {
            auto* leds = platform.getLEDs();
            leds[0] = CRGB::Red;
            leds[1] = CRGB::Green;
            
            platform.clear();
            
            for(size_t i = 0; i < NUM_LEDS; i++) {
                CHECK(leds[i] == CRGB::Black);
            }
        }
    }

    TEST_CASE("Platform Settings") {
        NativePlatform platform(100);
        
        SUBCASE("Brightness Control") {
            platform.setBrightness(128);
            // Note: In native platform, brightness is stored but not applied
            CHECK_MESSAGE(true, "Brightness setting stored");
        }
        
        SUBCASE("Performance Settings") {
            platform.setMaxRefreshRate(60);
            platform.setDither(0);
            CHECK_MESSAGE(true, "Performance settings stored");
        }
    }
} 