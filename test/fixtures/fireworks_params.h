// Auto-generated from fireworks.yaml
// YAML EXAMPLE: Colorful particle-based firework simulation
// Generated on 2025-02-01 01:13:59
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
    // Switch parameter
    {
        "sparkle",
        ParamType::switch_type,
        {.bool_default = true},
        Flags::NONE,
        "Enable sparkle effect"
    },

    // Count parameter without clamp
    {
        "num_particles",
        ParamType::count,
        {.range_min_i = 10, .range_max_i = 1000, .default_val_i = 100},
        Flags::NONE,
        "Number of particles"
    },

    // Range parameter with wrap
    {
        "gravity",
        ParamType::range,
        {.range_min = -1.0f, .range_max = 2.0f, .default_val = -0.8f},
        Flags::WRAP,
        "Gravity control"
    },

    // Ratio parameter without clamp
    {
        "speed",
        ParamType::ratio,
        {.float_default = 0.5f},
        Flags::NONE,
        "Animation speed multiplier"
    },

    // Select parameters
    {
        "pattern",
        ParamType::select,
        {.default_idx = 0, .options = pattern_options},
        Flags::NONE,
        "Pattern type"
    },
    {
        "direction",
        ParamType::select,
        {
            .default_idx = 0,
            .options = direction_options
        },
        Flags::NONE,
        "Rotation direction"
    },

    // Resource parameter
    {
        "palette",
        ParamType::palette,
        {.str_default = "rainbow"},
        Flags::NONE,
        "Color palette"
    }
};

} // namespace PixelTheater

