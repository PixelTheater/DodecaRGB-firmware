#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model - Point Operations") {
    TEST_CASE("point coordinate access") {
        BasicPentagonModel def;
        NativePlatform platform(BasicPentagonModel::LED_COUNT);
        platform.clear();
        Model<BasicPentagonModel> model(def, platform.getLEDs());

        SUBCASE("point properties") {
            auto& point = model.points[0];
            CHECK(point.face_id() >= 0);
            CHECK(point.face_id() < model.face_count());
            CHECK(std::isfinite(point.x()));
            CHECK(std::isfinite(point.y()));
            CHECK(std::isfinite(point.z()));
        }

        SUBCASE("point iteration") {
            for(const auto& point : model.points) {
                CHECK(point.face_id() < model.face_count());
                // Set LED color based on height
                float height = point.y();
                model.leds[point.id()] = CRGB(
                    static_cast<uint8_t>(height * 255),
                    0,
                    0
                );
            }
        }
    }
} 