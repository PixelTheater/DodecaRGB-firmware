// Auto-generated from space_scene.yaml
// Generated on 2024-03-14 10:30:45
#pragma once
#include "PixelTheater/parameter.h"

namespace PixelTheater {

struct SpaceSceneParameters {
    Parameter<float> speed {
        "speed",
        -1.0, 1.0,
        0.5
    }
    Parameter<float> brightness {
        "brightness",
        0.0, 1.0,
        0.8
    }
    Parameter<int> num_particles {
        "num_particles",
        0, 1000,
        100
    }
};

} // namespace PixelTheater 