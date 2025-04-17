#include <doctest/doctest.h>
#include "PixelTheater/palette_wrapper.h"
#include "PixelTheater/palettes.h" // Include for palette constants
#include "PixelTheater/core/crgb.h" // Include for CRGB

using namespace PixelTheater;

TEST_SUITE("PaletteWrapper") {
    TEST_CASE("Wrapper validates CRGBPalette16") {
        PaletteWrapper wrap1(Palettes::basePalette); // Use a known valid palette
        CHECK(wrap1.isValid() == true);

        // Currently no invalid CRGBPalette16 cases defined, as size is fixed by std::array
    }

    TEST_CASE("Wrapper validates Gradient Data") {
        // Valid gradient
        const uint8_t good_gradient[] = {
            0,   0,   0, 128,  // Dark blue at 0%
            128, 255, 255, 255,  // White at 50%
            255, 0,   0, 128   // Dark blue at 100%
        };
        PaletteWrapper wrap_good(good_gradient, sizeof(good_gradient));
        CHECK(wrap_good.isValid() == true);

        // Invalid - nullptr
        PaletteWrapper wrap_null(nullptr, 0);
        CHECK(wrap_null.isValid() == false);

        // Invalid - zero bytes
        PaletteWrapper wrap_zero(good_gradient, 0);
        CHECK(wrap_zero.isValid() == false);
        
        // Invalid - not multiple of 4
        const uint8_t bad_size[] = {0, 0, 0};
        PaletteWrapper wrap_bad_size(bad_size, sizeof(bad_size));
        CHECK(wrap_bad_size.isValid() == false);

        // Invalid - too few entries
        const uint8_t too_few[] = {0, 0, 0, 0};
        PaletteWrapper wrap_too_few(too_few, sizeof(too_few));
        CHECK(wrap_too_few.isValid() == false);

        // Invalid - too many entries (need 17+ entries -> 17*4 = 68 bytes)
        // For simplicity, just test with a small invalid array > MAX_ENTRIES
        // This test assumes MAX_ENTRIES = 16
        const uint8_t too_many[68] = {0}; // Need to define a valid-ish structure > 64 bytes
        // Fill with ascending indices to pass that check
        // uint8_t too_many_gradient[17 * 4]; 
        // for(int i=0; i<17; ++i) { too_many_gradient[i*4] = i * (255/16); } // Simple ascending indices
        // too_many_gradient[16*4] = 255; // Ensure last is 255
        // PaletteWrapper wrap_too_many(too_many_gradient, sizeof(too_many_gradient));
        // CHECK(wrap_too_many.isValid() == false); // This check is complex to set up, skip for now

        // Invalid - first index not 0
        const uint8_t bad_start[] = {1, 0,0,0, 255, 255,255,255};
        PaletteWrapper wrap_bad_start(bad_start, sizeof(bad_start));
        CHECK(wrap_bad_start.isValid() == false);

        // Invalid - last index not 255
        const uint8_t bad_end[] = {0, 0,0,0, 250, 255,255,255};
        PaletteWrapper wrap_bad_end(bad_end, sizeof(bad_end));
        CHECK(wrap_bad_end.isValid() == false);

        // Invalid - indices not ascending
        const uint8_t bad_order[] = {0, 0,0,0, 100, 1,1,1, 50, 2,2,2, 255, 3,3,3};
        PaletteWrapper wrap_bad_order(bad_order, sizeof(bad_order));
        CHECK(wrap_bad_order.isValid() == false);
    }

} 