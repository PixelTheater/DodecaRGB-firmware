#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"
#include "../helpers/model_test_fixture.h"
#include "PixelTheater/color/definitions.h"
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model Faces") {

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face properties") {
        const Face& f0 = model->face(0);
        CHECK(f0.id() == 0);
        CHECK(f0.led_count() > 0);
        CHECK(f0.led_offset() == 0);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face iteration") {
        size_t total_leds = 0;
        for(size_t i = 0; i < model->faceCount(); ++i) {
            const Face& f = model->face(i);
            CHECK(f.id() == i);
            CHECK(f.led_count() > 0);
            total_leds += f.led_count();
        }
        CHECK(total_leds == platform.getNumLEDs());
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face LED access") {
        CRGB* leds_ptr = platform.getLEDs();
        const Face& f0 = model->face(0);
        leds_ptr[f0.led_offset()] = CRGB::Red;
        leds_ptr[f0.led_offset() + f0.led_count() - 1] = CRGB::Blue;
        
        CHECK(leds_ptr[f0.led_offset()] == CRGB::Red);
        CHECK(leds_ptr[f0.led_offset() + f0.led_count() - 1] == CRGB::Blue);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face LED iteration") {
        const Face& f1 = model->face(1);
        CRGB* leds_ptr = platform.getLEDs();
        for(size_t i = 0; i < f1.led_count(); ++i) {
            leds_ptr[f1.led_offset() + i] = CRGB::Green;
        }

        for(size_t i = 0; i < f1.led_count(); ++i) {
            CHECK(leds_ptr[f1.led_offset() + i] == CRGB::Green);
        }
        const Face& f0 = model->face(0);
        CHECK(leds_ptr[f0.led_offset()] != CRGB::Green);
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "face boundaries") {
        CRGB* leds_ptr = platform.getLEDs();
        size_t num_faces = model->faceCount();

        for (size_t i = 0; i < num_faces; ++i) {
            const Face& face = model->face(i);
            CRGB color(i * 50, 0, 0);
            for (size_t j = 0; j < face.led_count(); ++j) {
                leds_ptr[face.led_offset() + j] = color;
            }
        }

        for (size_t i = 0; i < num_faces - 1; ++i) {
            const Face& current_face = model->face(i);
            const Face& next_face = model->face(i + 1);
            CHECK(leds_ptr[current_face.led_offset() + current_face.led_count() - 1] !=
                  leds_ptr[next_face.led_offset()]);
        }
    }

    TEST_CASE_FIXTURE(PixelTheater::Testing::ModelTestFixture<BasicPentagonModel>, "edge connectivity through interface") {
        // Test edge connectivity methods through IModel interface
        // These methods exist on the interface and can be tested
        

        
        // Test edge count for face 0 (pentagon should have 5 edges)
        uint8_t edge_count = model->face_edge_count(0);
        CHECK(edge_count == 5);
        
        // Test edge connections (might return -1 for no connection or valid face ID)
        int8_t connected_face = model->face_at_edge(0, 0);
        CHECK(connected_face >= -1);  // Valid range: -1 for no connection, or valid face ID
        
        // Test invalid face ID (should handle gracefully)
        uint8_t invalid_edge_count = model->face_edge_count(static_cast<uint8_t>(255));
        CHECK(invalid_edge_count == 0);  // Should return 0 for invalid face
        
        int8_t invalid_connection = model->face_at_edge(static_cast<uint8_t>(255), 0);
        CHECK(invalid_connection == -1);  // Should return -1 for invalid face
    }
} 