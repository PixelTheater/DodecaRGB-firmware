#pragma once

#include "PixelTheater/stage.h"

// Include scene registration functions
#include "test_scene_register.h"

// Include the blob scene
#include "blob_scene_register.h"

// Forward declarations of your scene registration functions
// Add all your scene registration functions here

template<typename ModelDef>
void registerAllScenes(PixelTheater::Stage<ModelDef>& stage) {
    // Register all scenes
    registerTestScene<ModelDef>(stage);
    
    // Register the blob scene
    registerBlobScene<ModelDef>(stage);
} 