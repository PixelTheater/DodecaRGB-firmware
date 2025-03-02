#pragma once

// Define PLATFORM_WEB for Emscripten/web builds
#ifdef EMSCRIPTEN
#ifndef PLATFORM_WEB
#define PLATFORM_WEB
#endif
#endif

// Use standard library for native and web platforms
#if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
#include <cmath>
namespace PixelTheater {
    inline bool is_nan(float x) { return std::isnan(x); }
    inline bool is_inf(float x) { return std::isinf(x); }
    template<typename T>
    inline T constrain_value(T x, T min, T max) { 
        return std::min(std::max(x, min), max); 
    }
}

// Only provide Arduino compatibility functions in native environment
template<typename T>
T constrain(T x, T min, T max) {
    return PixelTheater::constrain_value(x, min, max);
}

// For Arduino platform
#else
#include <Arduino.h>
namespace PixelTheater {
    inline bool is_nan(float x) { return isnan(x); }
    inline bool is_inf(float x) { return isinf(x); }
    template<typename T>
    inline T constrain_value(T x, T min, T max) {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }
}
#endif 