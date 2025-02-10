// Auto-generated from fireworks.yaml
// YAML EXAMPLE: Colorful particle-based firework simulation
// Generated on 2025-02-10 00:40:04
#pragma once
#include "PixelTheater/parameter.h"

namespace PixelTheater {

static constexpr const char* const pattern_options[] = {
    "sphere", "fountain", "cascade", nullptr
};
static constexpr const char* const direction_options[] = {
    "forward", "reverse", "oscillate", nullptr
};

// Parameter definitions - one line per param for easy diffing
constexpr ParamDef FIREWORKS_PARAMS[] = {
    PARAM_SWITCH("sparkle", true, "Enable sparkle effect"),
    PARAM_COUNT("num_particles", 10, 1000, 100, Flags::NONE, "Number of particles"),
    PARAM_RANGE("gravity", -1.0f, 2.0f, -0.8f, Flags::WRAP, "Gravity control"),
    PARAM_RATIO("speed", 0.5f, Flags::NONE, "Animation speed multiplier"),
    PARAM_ANGLE("brightness", 0.8f, Flags::WRAP, "Overall LED brightness"),
    PARAM_SELECT("pattern", 0, pattern_options, ""),
    PARAM_SELECT("direction", 0, direction_options, "Rotation direction and speed"),
    PARAM_PALETTE("palette", "rainbow", ""),
};

} // namespace PixelTheater

