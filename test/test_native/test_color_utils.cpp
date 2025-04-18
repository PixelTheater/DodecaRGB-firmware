// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN // Remove: Implementation should be in only one file (e.g., test_main.cpp)
#include <doctest/doctest.h>
#include <string>
// #include "PixelTheater/color_utils.h" // OLD
#include "PixelTheater/color/measurement.h" // NEW - for distance, brightness, contrast, hue_distance
#include "PixelTheater/color/identity.h"    // NEW - for name, ansi string
#include "PixelTheater/core/crgb.h"         // For CRGB/CHSV types
#include "PixelTheater/color/definitions.h" // Added for CRGB constants
// #include "helpers/fastled_test_helper.h" // Maybe not needed if using fallbacks

using namespace PixelTheater;
using namespace PixelTheater::ColorUtils;
using PixelTheater::ColorUtils::colorDistance;
using PixelTheater::ColorUtils::getClosestColorName;
using PixelTheater::ColorUtils::getAnsiColorString;
using PixelTheater::ColorUtils::get_perceived_brightness;
using PixelTheater::ColorUtils::get_contrast_ratio;
using PixelTheater::ColorUtils::get_hue_distance;
using PixelTheater::CRGB;
using PixelTheater::CHSV;

TEST_SUITE("ColorUtils (Native Fallbacks)") {
    TEST_CASE("colorDistance") {
        CHECK(colorDistance(CRGB::Red, CRGB::Red) == 0);
        CHECK(colorDistance(CRGB::Black, CRGB::White) == (255L*255L + 255L*255L + 255L*255L));
        CHECK(colorDistance(CRGB(10, 20, 30), CRGB(10, 20, 30)) == 0);
        CHECK(colorDistance(CRGB(10, 20, 30), CRGB(11, 22, 33)) == (1L*1L + 2L*2L + 3L*3L));
    }

    TEST_CASE("getClosestColorName") {
        CHECK(getClosestColorName(CRGB::Red) == "Red");
        CHECK(getClosestColorName(CRGB(250, 5, 5)) == "Red"); // Close to Red
        CHECK(getClosestColorName(CRGB::Green) == "Green");
        CHECK(getClosestColorName(CRGB(5, 250, 5)) == "Green");
        CHECK(getClosestColorName(CRGB::Blue) == "Blue");
        CHECK(getClosestColorName(CRGB(5, 5, 250)) == "Blue");
        CHECK(getClosestColorName(CRGB::White) == "White");
        CHECK(getClosestColorName(CRGB::Black) == "Black");
        CHECK(getClosestColorName(CRGB(128, 128, 128)) != "Black");
        CHECK(getClosestColorName(CRGB(128, 128, 128)) != "White");
    }

    TEST_CASE("getAnsiColorString") {
        std::string ansi = getAnsiColorString(CRGB::Red, 'X');
        CHECK(ansi.find("\033[48;2;") == 0);
        CHECK(ansi.rfind("mX\033[0m") == (ansi.length() - 6));
        CHECK(ansi.find("255;0;0") != std::string::npos);
    }

    TEST_CASE("get_perceived_brightness (Stub)") {
        CHECK(get_perceived_brightness(CHSV(0, 0, 255)) == doctest::Approx(1.0));
        CHECK(get_perceived_brightness(CHSV(0, 0, 0)) == doctest::Approx(0.0));
        CHECK(get_perceived_brightness(CHSV(0, 255, 128)) == doctest::Approx(0.5).epsilon(0.01));
    }

    TEST_CASE("get_contrast_ratio (Stub)") {
        CHSV white = CHSV(0, 0, 255);
        CHSV black = CHSV(0, 0, 0);
        CHSV grey = CHSV(0, 0, 128);
        CHECK(get_contrast_ratio(white, black) == doctest::Approx(21.0));
        CHECK(get_contrast_ratio(white, white) == doctest::Approx(1.0));
        CHECK(get_contrast_ratio(black, black) == doctest::Approx(1.0));
        CHECK(get_contrast_ratio(white, grey) == doctest::Approx(1.902).epsilon(0.001));
        CHECK(get_contrast_ratio(black, grey) == doctest::Approx(11.039).epsilon(0.001));
    }

    TEST_CASE("get_hue_distance") {
        CHECK(get_hue_distance(CHSV(0, 255, 255), CHSV(0, 255, 255)) == doctest::Approx(0.0));
        CHECK(get_hue_distance(CHSV(0, 255, 255), CHSV(128, 255, 255)) == doctest::Approx(180.0));
        CHECK(get_hue_distance(CHSV(10, 255, 255), CHSV(240, 255, 255)) == doctest::Approx(36.5625));
        CHECK(get_hue_distance(CHSV(0, 255, 255), CHSV(64, 255, 255)) == doctest::Approx(90.0));
    }
} 