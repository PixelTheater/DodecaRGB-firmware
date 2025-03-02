#include <doctest/doctest.h>
#include "PixelTheater/platform/web_platform.h"

// Test suite for WebPlatform class
TEST_SUITE("WebPlatform") {
    
    // Test WebPlatform initialization
    TEST_CASE("WebPlatform can be constructed") {
        // Create a WebPlatform with a small number of LEDs for testing
        const uint16_t num_leds = 10;
        
        #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
        // In web environment, we can fully test the WebPlatform
        PixelTheater::WebPlatform platform(num_leds);
        
        // Verify LED count
        CHECK(platform.getNumLEDs() == num_leds);
        
        // Verify default brightness
        CHECK(platform.getBrightness() > 0);
        #else
        // In non-web environment, just verify the stub implementation
        // This allows tests to compile but not actually use WebGL
        MESSAGE("Running in non-web environment, using stub implementation");
        PixelTheater::WebPlatform platform(num_leds);
        
        // These basic functions should work even in the stub
        CHECK(platform.getNumLEDs() == num_leds);
        #endif
    }
    
    // Test LED color setting
    TEST_CASE("WebPlatform can set LED colors") {
        const uint16_t num_leds = 5;
        
        #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
        PixelTheater::WebPlatform platform(num_leds);
        
        // Set all LEDs to red
        for (uint16_t i = 0; i < num_leds; i++) {
            platform.getLEDs()[i] = CRGB(255, 0, 0);
        }
        
        // Verify LEDs were set correctly
        for (uint16_t i = 0; i < num_leds; i++) {
            CHECK(platform.getLEDs()[i].r == 255);
            CHECK(platform.getLEDs()[i].g == 0);
            CHECK(platform.getLEDs()[i].b == 0);
        }
        #else
        // Minimal test for non-web environment
        MESSAGE("Running in non-web environment, some tests skipped");
        PixelTheater::WebPlatform platform(num_leds);
        // Simple verification that we can get LEDs
        CHECK(platform.getLEDs() != nullptr);
        #endif
    }
} 