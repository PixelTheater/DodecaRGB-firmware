#include <doctest/doctest.h>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/core/color.h"

using namespace PixelTheater;

TEST_CASE("FastLED Integration") {
    // Remove the Serial.println here as our custom reporter will handle this
    // Serial.println("\n=== Testing FastLED Integration ===");

    SUBCASE("Basic Color Operations") {
        Serial.println("Testing basic color operations...");
        ::CRGB color(255, 0, 0);  // Red
        CHECK(color.r == 255);
        CHECK(color.g == 0);
        CHECK(color.b == 0);
        Serial.printf("Color components check: R=%d, G=%d, B=%d\n", color.r, color.g, color.b);

        // Test color scaling
        color.nscale8(128);
        CHECK(color.r == 128);
        Serial.printf("Color scaling check: R=%d (expected 128)\n", color.r);
    }

    SUBCASE("Color Fill Operations") {
        Serial.println("Testing color fill operations...");
        ::CRGB leds[5];
        fill_solid(leds, 5, ::CRGB::Blue);
        
        bool all_correct = true;
        for(int i = 0; i < 5; i++) {
            CHECK(leds[i].b == 255);
            CHECK(leds[i].r == 0);
            CHECK(leds[i].g == 0);
            if(leds[i].b != 255 || leds[i].r != 0 || leds[i].g != 0) {
                all_correct = false;
                Serial.printf("LED %d incorrect: R=%d, G=%d, B=%d\n", i, leds[i].r, leds[i].g, leds[i].b);
            }
        }
        if(all_correct) {
            Serial.println("All LEDs verified blue (0, 0, 255)");
        }
    }

    SUBCASE("PixelTheater and FastLED Integration") {
        Serial.println("Testing PixelTheater and FastLED integration...");
        
        // Create arrays for both implementations
        ::CRGB fastled_array[5] = {};
        PixelTheater::CRGB pixeltheater_array[5] = {};
        
        // Test FastLED's fill_solid
        fill_solid(fastled_array, 5, ::CRGB::Red);
        
        // Test PixelTheater's fill_solid
        PixelTheater::fill_solid(pixeltheater_array, 5, PixelTheater::CRGB(255, 0, 0));
        
        // Verify both implementations work correctly
        CHECK(fastled_array[0].r == 255);
        CHECK(pixeltheater_array[0].r == 255);
        
        Serial.println("Both implementations produce correct results");
        
        // Test conversion between types
        PixelTheater::CRGB pt_color = PixelTheater::CRGB(fastled_array[0].r, fastled_array[0].g, fastled_array[0].b);
        CHECK(pt_color.r == 255);
        CHECK(pt_color.g == 0);
        CHECK(pt_color.b == 0);
        
        Serial.println("Conversion between types works correctly");
    }
    
    SUBCASE("FastLED Hardware Functions") {
        Serial.println("Testing FastLED hardware functions...");
        
        // Create a small LED array
        ::CRGB leds[10];
        
        // Configure FastLED with hardware pins
        FastLED.addLeds<WS2812B, 19, GRB>(leds, 10);
        
        // Set brightness
        FastLED.setBrightness(50);
        CHECK(FastLED.getBrightness() == 50);
        
        // Test show() function (should not crash)
        fill_solid(leds, 10, ::CRGB::Red);
        
        unsigned long start_time = micros();
        FastLED.show();
        unsigned long show_time = micros() - start_time;
        Serial.printf("FastLED.show() took %lu microseconds\n", show_time);
        
        delay(100);
        
        // Test clear function
        FastLED.clear();
        FastLED.show();
        delay(100);
        
        // Test FPS control
        FastLED.setMaxRefreshRate(60);
        
        Serial.println("FastLED hardware functions verified");
    }

    // Remove the Serial.println here as our custom reporter will handle this
    // Serial.println("FastLED tests complete!");
} 