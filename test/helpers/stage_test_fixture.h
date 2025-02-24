#pragma once
#include <memory>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/stage.h"

namespace PixelTheater::Testing {

template<typename ModelDef>
class StageTestFixture {
protected:
    std::unique_ptr<Stage<ModelDef>> stage;

    StageTestFixture() {
        auto platform = std::make_unique<NativePlatform>(ModelDef::LED_COUNT);
        platform->clear();  // Start with known state
        
        auto def = ModelDef{};
        auto model = std::make_unique<Model<ModelDef>>(def, platform->getLEDs());
        
        stage = std::make_unique<Stage<ModelDef>>(std::move(platform), std::move(model));
    }
};

} // namespace PixelTheater::Testing 