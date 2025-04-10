#pragma once
#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/core/imodel.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/core/model_wrapper.h"
#include "../fixtures/models/basic_pentagon_model.h"
#include "../fixtures/models/led_test_model.h"

namespace PixelTheater::Testing {
using namespace PixelTheater::Fixtures;

template<typename ModelDef>
class ModelTestFixture {
protected:
    ModelDef def;
    NativePlatform platform;
    std::unique_ptr<ModelWrapper<ModelDef>> model_wrapper_;
    IModel* model;

    ModelTestFixture() : platform(ModelDef::LED_COUNT) {
        auto concrete_model = std::make_unique<Model<ModelDef>>(
            platform.getLEDs()
        );

        model_wrapper_ = std::make_unique<ModelWrapper<ModelDef>>(std::move(concrete_model));
        model = model_wrapper_.get();
    }

    // Helper methods for common test operations
    void fillFace(size_t face_id, const CRGB& color) {
        const Face& face = model->face(face_id);
        CRGB* leds_ptr = platform.getLEDs();
        for(size_t i = 0; i < face.led_count(); ++i) {
            leds_ptr[face.led_offset() + i] = color;
        }
    }

    void verifyFaceColor(size_t face_id, const CRGB& expected_color) {
        const Face& face = model->face(face_id);
        CRGB* leds_ptr = platform.getLEDs();
        for(size_t i = 0; i < face.led_count(); ++i) {
            CHECK(leds_ptr[face.led_offset() + i] == expected_color);
        }
    }

    // Common verification helpers
    void verifyAllLEDsColor(const CRGB& expected_color) {
        CRGB* leds_ptr = platform.getLEDs();
        for(size_t i = 0; i < platform.getNumLEDs(); ++i) {
            CHECK(leds_ptr[i] == expected_color);
        }
    }

    void verifyFaceBoundaries() {
        // Fill each face with a different shade first
        size_t num_faces = model->faceCount();
        for(size_t i = 0; i < num_faces; i++) {
            fillFace(i, CRGB(i * 50, 0, 0));  // Different red shades
        }

        // Then verify boundaries using platform LEDs
        CRGB* leds_ptr = platform.getLEDs();
        for(size_t i = 0; i < num_faces - 1; i++) {
            const Face& current_face = model->face(i);
            const Face& next_face = model->face(i + 1);
            size_t last_led_idx_current = current_face.led_offset() + current_face.led_count() - 1;
            size_t first_led_idx_next = next_face.led_offset();
            CHECK(leds_ptr[last_led_idx_current] != leds_ptr[first_led_idx_next]);
        }
    }
};

// Common test fixture types

using BasicPentagonFixture = ModelTestFixture<BasicPentagonModel>;
using LedTestFixture = ModelTestFixture<LedTestModel>;

} // namespace PixelTheater::Testing 