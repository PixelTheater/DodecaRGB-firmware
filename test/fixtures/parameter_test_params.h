#pragma once
#include "PixelTheater/params/param_def.h"

namespace PixelTheater {

// Test parameter definitions
static constexpr ParamDef TEST_PARAMS[] = {
    // [0] test_bool
    PARAM_SWITCH("test_bool", true, "Test boolean parameter"),

    // [1] test_int
    PARAM_COUNT("test_int", 0, 100, 50, Flags::NONE, "Test integer parameter"),

    // [2] test_ratio
    PARAM_RATIO("test_ratio", 0.5f, Flags::NONE, "Test ratio parameter"),

    // [3] test_angle
    PARAM_ANGLE("test_angle", Constants::PT_HALF_PI, Flags::NONE, "Test angle parameter"),

    // [4] test_range_float - This was wrong
    PARAM_RANGE("test_range", -1.0f, 1.0f, 0.0f, Flags::NONE, "Test range parameter"),

    // [5] test_clamp - This was missing CLAMP flag
    PARAM_RATIO("test_clamp", 0.5f, Flags::CLAMP, "Test clamped parameter")
};

} // namespace PixelTheater 