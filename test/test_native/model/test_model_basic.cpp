#include <doctest/doctest.h>
#include "PixelTheater/platform/platform.h"
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"
#include "../../helpers/model_test_fixture.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;
using namespace PixelTheater::Testing;

TEST_SUITE("Model - Basic Operations") {
    TEST_CASE_FIXTURE(ModelTestFixture<BasicPentagonModel>, "construction") {
        // Check initial LED state
        verifyAllLEDsColor(CRGB::Black);

        // Check points were initialized correctly
        CHECK(model->points[0].face_id() == def.POINTS[0].face_id);
        CHECK(model->points[0].x() == def.POINTS[0].x);
        CHECK(model->points[0].y() == def.POINTS[0].y);
        CHECK(model->points[0].z() == def.POINTS[0].z);
    }

    TEST_CASE_FIXTURE(ModelTestFixture<BasicPentagonModel>, "face operations") {
        fillFace(0, CRGB::Red);
        verifyFaceColor(0, CRGB::Red);
        verifyFaceBoundaries();
    }

    TEST_CASE_FIXTURE(ModelTestFixture<BasicPentagonModel>, "LED direct access") {
        model->leds[0] = CRGB::Red;
        CHECK(model->leds[0] == CRGB::Red);

        const auto& const_model = *model;
        CHECK(const_model.leds[0] == CRGB::Red);
    }

    TEST_CASE_FIXTURE(ModelTestFixture<BasicPentagonModel>, "bounds checking") {
        model->leds[model->led_count() - 1] = CRGB::Red;
        CHECK(model->leds[model->led_count()] == CRGB::Red);
    }
} 