#include <doctest/doctest.h>
#include "PixelTheater/palette.h"

using namespace PixelTheater;

TEST_SUITE("Palettes") {
    TEST_CASE("Palette can be created from gradient data") {
        // FastLED gradient format: index,r,g,b
        uint8_t gradient[] = {
            0,   0,   0,   128,  // Dark blue at 0%
            128, 255, 255, 255,  // White at 50%
            255, 0,   0,   128   // Dark blue at 100%
        };

        Palette pal(gradient, sizeof(gradient));    // Pass total bytes, let Palette handle division
        
        SUBCASE("Raw values are accessible") {
            // First entry
            CHECK(pal.value_at(0) == 0);    // index
            CHECK(pal.value_at(1) == 0);    // r
            CHECK(pal.value_at(2) == 0);    // g
            CHECK(pal.value_at(3) == 128);  // b

            // Second entry
            CHECK(pal.value_at(4) == 128);  // index
            CHECK(pal.value_at(5) == 255);  // r
            CHECK(pal.value_at(6) == 255);  // g
            CHECK(pal.value_at(7) == 255);  // b
        }

        SUBCASE("Size is correct") {
            CHECK(pal.size() == 3);  // 3 entries
        }
    }

    TEST_CASE("Palette validates input data") {
        SUBCASE("Empty data is invalid") {
            Palette pal(nullptr, 0);
            CHECK(pal.is_valid() == false);
        }

        SUBCASE("Size must be multiple of 4") {
            uint8_t bad_data[] = {0, 0, 0};  // Only 3 bytes
            Palette pal(bad_data, 3);
            CHECK(pal.is_valid() == false);
        }
    }
} 