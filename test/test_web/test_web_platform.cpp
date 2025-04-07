#include <doctest/doctest.h>
#include "PixelTheater/platform/web_platform.h"

// Test suite for WebPlatform class
TEST_SUITE("WebPlatform") {
    
    // Test WebPlatform initialization
    TEST_CASE("WebPlatform can be constructed") {
        #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
        // In web environment, we can fully test the WebPlatform
        // TODO: Web environment tests need actual initialization
        PixelTheater::WebGL::WebPlatform platform;
        // Add checks relevant to web initialization later
        CHECK(platform.getNumLEDs() >= 0); // Basic check
        CHECK(platform.getBrightness() > 0);
        #else
        // In non-web environment, just verify the stub implementation
        MESSAGE("Running in non-web environment, using stub implementation");
        PixelTheater::WebGL::WebPlatform platform;
        // Basic checks for stub compilation
        CHECK(platform.getNumLEDs() >= 0); // Check default value or stub return
        CHECK(platform.getBrightness() > 0); // Check default
        #endif
    }
    
    // Test LED color setting
    TEST_CASE("WebPlatform can get LED buffer (Stub Check)") {
        #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
        // TODO: Web environment tests need actual initialization and LED buffer access
        PixelTheater::WebGL::WebPlatform platform;
        // Add checks for web LED buffer access later
        // Example (assuming leds are initialized):
        // platform.getLEDs()[0] = CRGB::Red;
        // CHECK(platform.getLEDs()[0] == CRGB::Red);
        CHECK(true); // Placeholder
        #else
        // Minimal test for non-web environment stub
        MESSAGE("Running in non-web environment, checking stub methods");
        PixelTheater::WebGL::WebPlatform platform;
        // Simple verification that we can call getLEDs in the stub
        CHECK_NOTHROW(platform.getLEDs()); 
        // The stub returns _leds which is nullptr initially, so don't check for non-null
        // Check other basic methods compile
        CHECK_NOTHROW(platform.clear());
        CHECK_NOTHROW(platform.show());
        CHECK_NOTHROW(platform.setBrightness(100));
        CHECK_NOTHROW(platform.setMaxRefreshRate(60));
        CHECK_NOTHROW(platform.setDither(1));
        #endif
    }

    // Test that the new Platform interface methods compile
    TEST_CASE("WebPlatform interface methods compile (Native Stub)") {
        MESSAGE("Checking compilation of Platform interface methods in WebPlatform stub");
        PixelTheater::WebGL::WebPlatform platform;

        // Timing
        CHECK(platform.deltaTime() >= 0.0f);
        CHECK(platform.millis() >= 0);

        // Random
        CHECK_NOTHROW(platform.random8());
        CHECK_NOTHROW(platform.random16());
        CHECK(platform.random(100) < 100);
        CHECK(platform.random(10, 20) >= 10);
        CHECK(platform.random(10, 20) < 20);
        CHECK(platform.randomFloat() >= 0.0f);
        CHECK(platform.randomFloat() <= 1.0f);
        CHECK(platform.randomFloat(10.0f) <= 10.0f);
        CHECK(platform.randomFloat(5.0f, 10.0f) >= 5.0f);
        CHECK(platform.randomFloat(5.0f, 10.0f) <= 10.0f);

        // Logging
        CHECK_NOTHROW(platform.logInfo("Info test"));
        CHECK_NOTHROW(platform.logWarning("Warning test"));
        CHECK_NOTHROW(platform.logError("Error test"));
    }
} 