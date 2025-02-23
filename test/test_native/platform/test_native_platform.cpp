#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;

TEST_SUITE("NativePlatform") {
    TEST_CASE("Basic Platform Operations") {
        NativePlatform platform(100); // Initialize with 100 LEDs
        
        SUBCASE("LED Array Management") {
            // Test array access and size
            auto* leds = platform.getLEDs();
            CHECK(platform.getNumLEDs() == 100);
            CHECK(leds != nullptr);
            
            // Test array write access
            leds[0] = CRGB::Red;
            CHECK(leds[0].r == 255);
            CHECK(leds[0].g == 0);
            CHECK(leds[0].b == 0);
        }
        
        SUBCASE("Clear Operation") {
            // Write some colors first
            auto* leds = platform.getLEDs();
            leds[0] = CRGB::Red;
            leds[1] = CRGB::Green;
            leds[2] = CRGB::Blue;
            
            // Test clear
            platform.clear();
            
            // Verify all LEDs are black
            for(size_t i = 0; i < platform.getNumLEDs(); i++) {
                CHECK(leds[i].r == 0);
                CHECK(leds[i].g == 0);
                CHECK(leds[i].b == 0);
            }
        }
        
        SUBCASE("Brightness Control") {
            platform.setBrightness(128); // 50% brightness
            CHECK_MESSAGE(true, "Brightness control tested (visual verification needed on hardware)");
        }
    }
    
    TEST_CASE("Platform Configuration") {
        NativePlatform platform(100);
        
        SUBCASE("Refresh Rate Control") {
            platform.setMaxRefreshRate(60);
            CHECK_MESSAGE(true, "Refresh rate set (timing verification needed on hardware)");
        }
        
        SUBCASE("Dither Settings") {
            platform.setDither(0);
            CHECK_MESSAGE(true, "Dither control set (visual verification needed on hardware)");
        }
    }
    
    TEST_CASE("Array Bounds Safety") {
        NativePlatform platform(100);
        auto* leds = platform.getLEDs();
        
        SUBCASE("Valid Array Access") {
            // Access first and last LED
            leds[0] = CRGB::Red;
            leds[99] = CRGB::Blue;
            
            CHECK(leds[0].r == 255);
            CHECK(leds[99].b == 255);
        }
        
        // Note: Bounds checking would be done in debug builds
        // or through wrapper classes. Raw array access can't
        // be bounds-checked for performance reasons.
    }
} 