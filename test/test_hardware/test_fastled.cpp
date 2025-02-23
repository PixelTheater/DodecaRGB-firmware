#include <doctest/doctest.h>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;

TEST_CASE("FastLED Integration") {
    Serial.println("\n=== Testing FastLED Integration ===");

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

    Serial.println("FastLED tests complete!");
} 