#pragma once

#include "PixelTheater/stage.h"
#include "test_scene.h"

// Function to register the test scene with the stage
template<typename ModelDef>
inline void registerTestScene(PixelTheater::Stage<ModelDef>& stage) {
    stage.template registerScene<Scenes::TestScene<ModelDef>>("Test Scene", "A simple test scene with rainbow patterns");
}
