#include <doctest/doctest.h>
#include <cmath> // For approximation checks
#include "PixelTheater/easing.h"

using namespace PixelTheater::Easing;

// Use a small epsilon for floating point comparisons
const float FLOAT_EPSILON = 0.0001f;

TEST_SUITE("Easing Functions") {

    TEST_CASE("Linear") {
        // Fractional
        CHECK(linearF(0.0f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON));
        CHECK(linearF(0.5f) == doctest::Approx(0.5f).epsilon(FLOAT_EPSILON));
        CHECK(linearF(1.0f) == doctest::Approx(1.0f).epsilon(FLOAT_EPSILON));
        // Interpolating
        CHECK(linear(0.0f, 100.0f, 0.0f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON));
        CHECK(linear(0.0f, 100.0f, 0.5f) == doctest::Approx(50.0f).epsilon(FLOAT_EPSILON));
        CHECK(linear(0.0f, 100.0f, 1.0f) == doctest::Approx(100.0f).epsilon(FLOAT_EPSILON));
        CHECK(linear(50.0f, 150.0f, 0.5f) == doctest::Approx(100.0f).epsilon(FLOAT_EPSILON));
        // Clamping
        CHECK(linear(0.0f, 100.0f, -0.5f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON)); 
        CHECK(linear(0.0f, 100.0f, 1.5f) == doctest::Approx(100.0f).epsilon(FLOAT_EPSILON)); 
    }

    TEST_CASE("Sine") {
        // InSine
        CHECK(inSineF(0.0f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON));
        CHECK(inSineF(0.5f) == doctest::Approx(1.0f - std::sqrt(2.0f)/2.0f).epsilon(FLOAT_EPSILON)); // Approx 0.2929
        CHECK(inSineF(1.0f) == doctest::Approx(1.0f).epsilon(FLOAT_EPSILON));
        CHECK(inSine(10.0f, 110.0f, 0.0f) == doctest::Approx(10.0f).epsilon(FLOAT_EPSILON));
        CHECK(inSine(10.0f, 110.0f, 1.0f) == doctest::Approx(110.0f).epsilon(FLOAT_EPSILON));

        // OutSine
        CHECK(outSineF(0.0f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON));
        CHECK(outSineF(0.5f) == doctest::Approx(std::sqrt(2.0f)/2.0f).epsilon(FLOAT_EPSILON)); // Approx 0.7071
        CHECK(outSineF(1.0f) == doctest::Approx(1.0f).epsilon(FLOAT_EPSILON));
        CHECK(outSine(10.0f, 110.0f, 0.0f) == doctest::Approx(10.0f).epsilon(FLOAT_EPSILON));
        CHECK(outSine(10.0f, 110.0f, 1.0f) == doctest::Approx(110.0f).epsilon(FLOAT_EPSILON));

        // InOutSine
        CHECK(inOutSineF(0.0f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON));
        CHECK(inOutSineF(0.5f) == doctest::Approx(0.5f).epsilon(FLOAT_EPSILON));
        CHECK(inOutSineF(1.0f) == doctest::Approx(1.0f).epsilon(FLOAT_EPSILON));
        CHECK(inOutSine(10.0f, 110.0f, 0.0f) == doctest::Approx(10.0f).epsilon(FLOAT_EPSILON));
        CHECK(inOutSine(10.0f, 110.0f, 0.5f) == doctest::Approx(60.0f).epsilon(FLOAT_EPSILON)); // 10 + (100 * 0.5)
        CHECK(inOutSine(10.0f, 110.0f, 1.0f) == doctest::Approx(110.0f).epsilon(FLOAT_EPSILON));
    }

    TEST_CASE("Quad") {
        // InQuad
        CHECK(inQuadF(0.0f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON));
        CHECK(inQuadF(0.5f) == doctest::Approx(0.25f).epsilon(FLOAT_EPSILON));
        CHECK(inQuadF(1.0f) == doctest::Approx(1.0f).epsilon(FLOAT_EPSILON));
        CHECK(inQuad(0.0f, 100.0f, 0.5f) == doctest::Approx(25.0f).epsilon(FLOAT_EPSILON));

        // OutQuad
        CHECK(outQuadF(0.0f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON));
        CHECK(outQuadF(0.5f) == doctest::Approx(0.75f).epsilon(FLOAT_EPSILON));
        CHECK(outQuadF(1.0f) == doctest::Approx(1.0f).epsilon(FLOAT_EPSILON));
        CHECK(outQuad(0.0f, 100.0f, 0.5f) == doctest::Approx(75.0f).epsilon(FLOAT_EPSILON));

        // InOutQuad
        CHECK(inOutQuadF(0.0f) == doctest::Approx(0.0f).epsilon(FLOAT_EPSILON));
        CHECK(inOutQuadF(0.25f) == doctest::Approx(0.125f).epsilon(FLOAT_EPSILON)); // 2*0.25*0.25 = 0.125
        CHECK(inOutQuadF(0.5f) == doctest::Approx(0.5f).epsilon(FLOAT_EPSILON));   // 1 - pow(-2*0.5+2, 2)/2 = 1 - pow(1, 2)/2 = 0.5 
        CHECK(inOutQuadF(0.75f) == doctest::Approx(0.875f).epsilon(FLOAT_EPSILON)); // 1 - pow(-2*0.75+2, 2)/2 = 1 - pow(0.5, 2)/2 = 1 - 0.25/2 = 0.875
        CHECK(inOutQuadF(1.0f) == doctest::Approx(1.0f).epsilon(FLOAT_EPSILON));
        CHECK(inOutQuad(0.0f, 100.0f, 0.5f) == doctest::Approx(50.0f).epsilon(FLOAT_EPSILON));
    }

    // TODO: Add test cases for Cubic, Quart, Quint, Expo, Circ, Back, Elastic, Bounce

} 