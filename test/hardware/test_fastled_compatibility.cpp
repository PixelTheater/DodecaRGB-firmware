#ifdef PLATFORM_TEENSY  // Only compile these tests for hardware
#include <doctest/doctest.h>
#include <FastLED.h>
#include "PixelTheater/core/color.h"

TEST_SUITE("FastLED Compatibility") {
    TEST_CASE("CRGB construction matches FastLED") {
        ::CRGB fastled_color(255, 0, 0);
        PixelTheater::CRGB our_color(255, 0, 0);
        
        CHECK(fastled_color.r == our_color.r);
        CHECK(fastled_color.g == our_color.g);
        CHECK(fastled_color.b == our_color.b);
    }

    TEST_CASE("Color operations match FastLED") {
        ::CRGB fastled_color(255, 0, 0);
        PixelTheater::CRGB our_color(255, 0, 0);
        
        fastled_color.fadeToBlackBy(128);
        our_color.fadeToBlackBy(128);
        
        CHECK(fastled_color.r == our_color.r);
        CHECK(fastled_color.g == our_color.g);
        CHECK(fastled_color.b == our_color.b);
    }
}
#endif 