#pragma once
#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../fixtures/models/basic_pentagon_model.h"
#include "../fixtures/models/led_test_model.h"

namespace PixelTheater::Testing {
using namespace PixelTheater::Fixtures;

template<typename ModelDef>
class ModelTestFixture {
protected:
    ModelDef def;
    NativePlatform platform;
    std::unique_ptr<Model<ModelDef>> model;

    ModelTestFixture() 
        : platform(ModelDef::LED_COUNT)
    {
        platform.clear();  // Ensure clean state
        model = std::make_unique<Model<ModelDef>>(def, platform.getLEDs());
    }

    // Helper methods for common test operations
    void fillFace(size_t face_id, const CRGB& color) {
        auto& face = model->faces[face_id];
        for(auto& led : face.leds) {
            led = color;
        }
    }

    void verifyFaceColor(size_t face_id, const CRGB& expected_color) {
        auto& face = model->faces[face_id];
        for(auto& led : face.leds) {
            CHECK(led == expected_color);
        }
    }

    // Common verification helpers
    void verifyAllLEDsColor(const CRGB& expected_color) {
        for(auto& led : model->leds) {
            CHECK(led == expected_color);
        }
    }

    void verifyFaceBoundaries() {
        // Fill each face with a different shade first
        for(size_t i = 0; i < model->face_count(); i++) {
            fillFace(i, CRGB(i * 50, 0, 0));  // Different red shades
        }

        // Then verify boundaries
        for(size_t i = 0; i < model->face_count() - 1; i++) {
            auto& face = model->faces[i];
            auto next_led_idx = face.led_offset() + face.led_count();
            CHECK(model->leds[next_led_idx - 1] != model->leds[next_led_idx]);
        }
    }
};

// Common test fixture types

using BasicPentagonFixture = ModelTestFixture<BasicPentagonModel>;
using LedTestFixture = ModelTestFixture<LedTestModel>;

} // namespace PixelTheater::Testing 