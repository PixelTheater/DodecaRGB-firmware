#pragma once

#include <cmath>
#include <cstdint>
#include <emscripten.h>

namespace PixelTheater {
namespace WebGLUtil {

// Constants for Math
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float HALF_PI = 1.57079632679489661923f;

// Degree to radian conversion
inline float toRadians(float degrees) {
    return degrees * (PI / 180.0f);
}

// Radian to degree conversion
inline float toDegrees(float radians) {
    return radians * (180.0f / PI);
}

// Clamp a value between a minimum and maximum value
template<typename T>
inline T clamp(T value, T min, T max) {
    return (value < min) ? min : ((value > max) ? max : value);
}

// Linear interpolation between two values
inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Get the current time in seconds (for animation and timing)
inline double getCurrentTime() {
    return emscripten_get_now() / 1000.0;
}

// Get canvas dimensions
int getCanvasWidth();
int getCanvasHeight();

// Update FPS counter in UI
void updateFPSCounter(int fps);

} // namespace WebGLUtil
} // namespace PixelTheater 