#include <doctest/doctest.h>
#include "PixelTheater/platform/platform.h"
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"
#include "../helpers/model_test_fixture.h"
#include "PixelTheater/color/definitions.h"
#include <vector>
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;
using namespace PixelTheater::Testing;

TEST_SUITE("Model - Basic Operations") {
    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "construction") {
        // Check that the model was constructed and basic properties are accessible
        CHECK(model != nullptr);
        CHECK(model->pointCount() == def.LED_COUNT);
        CHECK(model->faceCount() == def.FACE_COUNT);
    }

    TEST_CASE_FIXTURE(ModelTestFixture<BasicPentagonModel>, "point access") {
        CHECK(model->point(0).face_id() == def.POINTS[0].face_id);
        CHECK(model->point(0).x() == doctest::Approx(def.POINTS[0].x));
        CHECK(model->point(0).y() == doctest::Approx(def.POINTS[0].y));
        CHECK(model->point(0).z() == doctest::Approx(def.POINTS[0].z));
    }

    TEST_CASE_FIXTURE(ModelTestFixture<BasicPentagonModel>, "face operations") {
        fillFace(0, CRGB::Red);
        verifyFaceColor(0, CRGB::Red);
        verifyFaceBoundaries();
    }

    // Removed LED access tests that were better suited for platform or face tests
    // TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "led access") {
    //     // Use platform LEDs for direct modification/verification
    //     platform.getLEDs()[0] = CRGB::Red;
    //     CHECK(platform.getLEDs()[0] == CRGB::Red);
    // 
    //     // Check const access via platform
    //     const NativePlatform& const_platform = platform;
    //     CHECK(const_platform.getLEDs()[0] == CRGB::Red);
    // 
    //     platform.getLEDs()[platform.getNumLEDs() - 1] = CRGB::Green;
    //     CHECK(platform.getLEDs()[platform.getNumLEDs() - 1] == CRGB::Green);
    // }
    // 
    // TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "led bounds") {
    //     // Bounds checking is handled by ILedBuffer wrapper or platform, not tested here
    //     // platform.getLEDs()[platform.getNumLEDs() - 1] = CRGB::Red; // Set last valid LED
    //     // CHECK(platform.getLEDs()[platform.getNumLEDs()] == CRGB::Red); // This might depend on platform/wrapper behavior (clamp/error/dummy)
    // }
} 