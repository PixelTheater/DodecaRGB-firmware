// Auto-generated from fireworks.yaml
// YAML EXAMPLE: Colorful particle-based firework simulation
// Generated on 2025-02-01 22:25:12
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
    {"sparkle",           ParamType::switch_type, {.bool_default = true}, Flags::NONE, "Enable sparkle effect"},
    {"num_particles",     ParamType::count, {.range_min_i = 10, .range_max_i = 1000, .default_val_i = 100}, Flags::CLAMP, "Number of particles"},
    {"gravity",           ParamType::range, {.range_min_i = -1.0, .range_max_i = 2.0, .default_val_i = -0.8}, Flags::WRAP, "Gravity control"},
    {"speed",             ParamType::ratio, {.range_min = 0.0f, .range_max = 1.0f, .default_val = 0.5f}, Flags::CLAMP, "Animation speed multiplier"},
    {"brightness",        ParamType::angle, {.range_min = 0.0f, .range_max = 3.141592653589793f, .default_val = 0.8f}, Flags::WRAP, "Overall LED brightness"},
    {"pattern",           ParamType::select, {.default_idx = 0, .options = pattern_options}, Flags::NONE, ""},
    {"direction",         ParamType::select, {.default_idx = 0, .options = direction_options}, Flags::NONE, "Rotation direction and speed"},
    {"palette",           ParamType::palette, {.str_default = "rainbow"}, Flags::NONE, ""}
};

} // namespace PixelTheater
