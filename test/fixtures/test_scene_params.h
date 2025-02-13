// Auto-generated from test_scene.yaml
// YAML EXAMPLE: Generated from test_scene.yaml
// Generated on 2025-02-12 21:35:30
#pragma once
#include "PixelTheater/parameter.h"

namespace PixelTheater {


// Parameter definitions - one line per param for easy diffing
constexpr ParamDef TEST_SCENE_PARAMS[] = {
    PARAM_RATIO("speed", 0.5f, Flags::CLAMP, "Animation speed"),
    PARAM_COUNT("count", 10, 1000, 50, Flags::WRAP, "Number of particles"),
    PARAM_SWITCH("enabled", true, "Enable animation"),
};

} // namespace PixelTheater
