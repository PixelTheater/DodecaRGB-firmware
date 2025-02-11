#pragma once

namespace PixelTheater {
namespace Constants {
#ifdef PLATFORM_NATIVE
    // Native platform uses our definitions
    static constexpr float PT_PI = 3.14159265358979323846f;
    static constexpr float PT_TWO_PI = 2.0f * PT_PI;
    static constexpr float PT_HALF_PI = PT_PI / 2.0f;
#else
    // Hardware platform uses Arduino's literal values
    static constexpr float PT_PI = 3.1415926535897932384626433832795f;
    static constexpr float PT_TWO_PI = 6.283185307179586476925286766559f;
    static constexpr float PT_HALF_PI = 1.5707963267948966192313216916398f;
#endif

    // Parameter ranges
    static constexpr float RATIO_MIN = 0.0f;
    static constexpr float RATIO_MAX = 1.0f;
    static constexpr float SIGNED_RATIO_MIN = -1.0f;
    static constexpr float SIGNED_RATIO_MAX = 1.0f;
    static constexpr float ANGLE_MIN = 0.0f;
    static constexpr float ANGLE_MAX = PT_PI;
    static constexpr float SIGNED_ANGLE_MIN = -PT_PI;
    static constexpr float SIGNED_ANGLE_MAX = PT_PI;
} // namespace Constants
} // namespace PixelTheater 