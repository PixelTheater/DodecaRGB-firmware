#pragma once

namespace PixelTheater {
namespace Constants {
    // Math constants
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float TWO_PI = 2.0f * PI;
    static constexpr float HALF_PI = PI / 2.0f;

    // Parameter ranges
    static constexpr float RATIO_MIN = 0.0f;
    static constexpr float RATIO_MAX = 1.0f;
    static constexpr float SIGNED_RATIO_MIN = -1.0f;
    static constexpr float SIGNED_RATIO_MAX = 1.0f;
    static constexpr float ANGLE_MIN = 0.0f;
    static constexpr float ANGLE_MAX = PI;
    static constexpr float SIGNED_ANGLE_MIN = -PI;
    static constexpr float SIGNED_ANGLE_MAX = PI;
} // namespace Constants
} // namespace PixelTheater 