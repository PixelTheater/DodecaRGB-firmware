#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"
#include "../helpers/model_test_fixture.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model Points") {

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "point coordinates") {
        // Access points via model interface
        for(size_t i = 0; i < def.LED_COUNT; ++i) {
            const Point& p = model->point(i); // USE model interface method
            CHECK(p.x() == doctest::Approx(def.POINTS[i].x));
            CHECK(p.y() == doctest::Approx(def.POINTS[i].y));
            CHECK(p.z() == doctest::Approx(def.POINTS[i].z));
            CHECK(p.face_id() == def.POINTS[i].face_id);
        }
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "point distances") {
        // Access points via model interface
        const Point& p0 = model->point(0);
        const Point& p1 = model->point(1);

        float dist = p0.distanceTo(p1);
        CHECK(dist == doctest::Approx(1.0f)); // Assuming unit distance between first two points
    }
} 