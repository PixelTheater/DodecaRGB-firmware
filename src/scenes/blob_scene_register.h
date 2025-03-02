#pragma once

#include "PixelTheater/stage.h"
#include "blob_scene.h"

template<typename ModelDef>
void registerBlobScene(PixelTheater::Stage<ModelDef>& stage) {
    stage.template addScene<Scenes::BlobScene<ModelDef>>(stage);
}
