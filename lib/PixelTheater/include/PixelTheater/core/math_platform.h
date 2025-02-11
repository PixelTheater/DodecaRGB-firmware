#pragma once

#ifdef PLATFORM_NATIVE
#include <cmath>
namespace PixelTheater {
    inline bool is_nan(float x) { return std::isnan(x); }
    inline bool is_inf(float x) { return std::isinf(x); }
}
#else
#include <Arduino.h>
namespace PixelTheater {
    inline bool is_nan(float x) { return isnan(x); }
    inline bool is_inf(float x) { return isinf(x); }
}
#endif 