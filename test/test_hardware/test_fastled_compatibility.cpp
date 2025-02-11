#include <doctest/doctest.h>
#include <FastLED.h>
#include "PixelTheater/core/color.h"

TEST_SUITE("FastLED Compatibility") {
    TEST_CASE("Basic color operations") {
        ::CRGB fastled_color(255, 0, 0);
        PixelTheater::CRGB our_color(255, 0, 0);
        
        CHECK(fastled_color.r == our_color.r);
        CHECK(fastled_color.g == our_color.g);
        CHECK(fastled_color.b == our_color.b);
    }
} 