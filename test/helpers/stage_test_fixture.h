#pragma once

#include <doctest/doctest.h>
#include <memory>
// #include "PixelTheater/stage.h" // Temporarily remove direct Stage dependency
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
// #include "PixelTheater/scene.h" // Scene usage is tied to Stage

namespace PixelTheater {
namespace Testing {

// template<typename ModelDef>
// class StageTestFixture {
// public:
//     std::unique_ptr<NativePlatform> platform;
//     std::unique_ptr<Model<ModelDef>> model;
//     std::unique_ptr<Stage<ModelDef>> stage;
//     CRGB leds[ModelDef::LED_COUNT];

//     StageTestFixture() {
//         platform = std::make_unique<NativePlatform>(ModelDef::LED_COUNT, leds);
//         model = std::make_unique<Model<ModelDef>>(ModelDef(), platform->getLEDs());
//         // Stage now takes raw platform pointer, model is internal
//         stage = std::make_unique<Stage<ModelDef>>(platform.get()); 
//     }

//     // Helper methods (might need adjustment or removal)
//     // ... fillFace, verifyFaceColor, verifyAllLEDsColor etc ...
// };

// Add new fixtures here if needed later

} // namespace Testing
} // namespace PixelTheater 