#include <doctest/doctest.h>
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model - Basic Operations") {
    TEST_CASE("construction") {
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);
        
        // Check initial LED state
        for(size_t i = 0; i < model.led_count(); i++) {
            CHECK(model.leds[i] == CRGB::Black);
        }

        // Check points were initialized correctly
        CHECK(model.points[0].face_id() == def.POINTS[0].face_id);
        CHECK(model.points[0].x() == def.POINTS[0].x);
        CHECK(model.points[0].y() == def.POINTS[0].y);
        CHECK(model.points[0].z() == def.POINTS[0].z);
    }

    TEST_CASE("LED direct access") {
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);
        
        model.leds[0] = CRGB::Red;
        CHECK(model.leds[0] == CRGB::Red);

        const auto& const_model = model;
        CHECK(const_model.leds[0] == CRGB::Red);
    }

    TEST_CASE("bounds checking") {
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);

        model.leds[model.led_count() - 1] = CRGB::Red;
        CHECK(model.leds[model.led_count()] == CRGB::Red);
    }
} 