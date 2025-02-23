#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/stage.h"
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;

TEST_SUITE("Stage Platform Integration") {
    TEST_CASE("Basic Stage Operations") {
        Stage stage;
        stage.setPlatform(std::make_unique<NativePlatform>(100));
        
        SUBCASE("LED Array Access") {
            // Test basic array access through stage
            auto* leds = stage.leds();
            CHECK(stage.numLeds() == 100);
            CHECK(leds != nullptr);
            
            // Test write operations
            leds[0] = CRGB::Red;
            CHECK(leds[0].r == 255);
            CHECK(leds[0].g == 0);
            CHECK(leds[0].b == 0);
        }
        
        SUBCASE("Platform Operations") {
            // Test that stage forwards platform operations correctly
            stage.setBrightness(128);
            stage.show(); // Should forward to platform.show()
            stage.clear();
            
            // Verify clear operation worked
            auto* leds = stage.leds();
            for(size_t i = 0; i < stage.numLeds(); i++) {
                CHECK(leds[i].r == 0);
                CHECK(leds[i].g == 0);
                CHECK(leds[i].b == 0);
            }
        }
    }
    
    TEST_CASE("Model Integration") {
        Stage stage;
        stage.setPlatform(std::make_unique<NativePlatform>(100));
        
        SUBCASE("LED Array Sharing") {
            // Get LED array from stage
            auto* stage_leds = stage.leds();
            
            // Write through stage
            stage_leds[0] = CRGB::Red;
            stage_leds[1] = CRGB::Green;
            
            // Verify changes are visible
            CHECK(stage_leds[0].r == 255);
            CHECK(stage_leds[1].g == 255);
        }
    }
    
    TEST_CASE("Platform Lifecycle") {
        Stage stage;
        
        SUBCASE("Platform Replacement") {
            // Set initial platform
            stage.setPlatform(std::make_unique<NativePlatform>(100));
            auto* leds1 = stage.leds();
            
            // Replace with new platform
            stage.setPlatform(std::make_unique<NativePlatform>(200));
            auto* leds2 = stage.leds();
            
            // Verify new platform
            CHECK(stage.numLeds() == 200);
            CHECK(leds1 != leds2);
        }
    }
} 