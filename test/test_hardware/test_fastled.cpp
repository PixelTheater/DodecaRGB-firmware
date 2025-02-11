#include <doctest/doctest.h>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;

TEST_CASE("FastLED Integration") {
    Serial.println("\n=== Testing FastLED Integration ===");

    SUBCASE("Core CRGB") {
        // Use FastLED's specific color constants
        ::CRGB color = ::CHSV(HUE_RED, 255, 255);  // Explicitly use FastLED's CHSV
        Serial.printf("FastLED HSV color: R=%d, G=%d, B=%d\n", 
            color.r, color.g, color.b);
    }

    SUBCASE("Color Helpers") {
        // Use FastLED's specific palette features
        CRGBPalette16 pal;  // Only exists in FastLED
        fill_rainbow(pal, 16, 0, 16);
        
        ::CRGB color = ColorFromPalette(pal, 0);  // FastLED-specific
        Serial.printf("Palette color: R=%d, G=%d, B=%d\n", 
            color.r, color.g, color.b);
    }

    SUBCASE("FastLED Purity Test") {
        // Use FastLED's specific math functions
        ::CRGB color(255, 0, 0);
        color.nscale8_video(128);  // FastLED-specific scaling
        
        // Use FastLED's specific operators
        color |= ::CRGB(0, 255, 0);  // FastLED's operator overloading
        
        Serial.printf("After FastLED ops: R=%d, G=%d, B=%d\n", 
            color.r, color.g, color.b);
    }

    Serial.println("FastLED tests complete!");
} 