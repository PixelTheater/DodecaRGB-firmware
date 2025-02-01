// Auto-generated from fireworks.yaml
// YAML EXAMPLE: Colorful particle-based firework simulation
// Generated on 2025-02-01 00:52:27
#pragma once
#include "PixelTheater/parameter.h"
#include "PixelTheater/param_types.h"

namespace PixelTheater {

// Parameter definitions - one line per param for easy diffing
constexpr ParamDef FIREWORKS_PARAMS[] = {
    {"sparkle",           ParamType::switch, true, , "Enable sparkle effect"}
    {"num_particles",     ParamType::count, 10, 1000, 100, ParamFlag::Clamp, "Number of particles"}
    {"gravity",           ParamType::range, -1.0, 2.0, -0.8, ParamFlag::Wrap, "Gravity control"}
    {"speed",             ParamType::ratio, 0.5f, ParamFlag::Clamp, "Animation speed multiplier"}
    {"brightness",        ParamType::angle, 0.8f, ParamFlag::Wrap, "Overall LED brightness"}
    {"pattern",           ParamType::select, 0, , "", {"sphere", "fountain", "cascade"}},
    {"direction",         ParamType::select, 0, , "Rotation direction and speed", {"forward", "reverse", "oscillate"}},
    {"palette",           ParamType::palette, "rainbow", , ""}
};

} // namespace PixelTheater
