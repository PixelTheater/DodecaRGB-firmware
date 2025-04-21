#pragma once

#include <cmath> // For pow, sin, cos, etc.
#include <algorithm> // For std::min, std::max

namespace PixelTheater {
namespace Easing {

    // Define PI if not available (cmath might provide M_PI)
    #ifndef PI
        #ifdef M_PI
            #define PI M_PI
        #else
            #define PI 3.14159265358979323846
        #endif
    #endif
    
    // Helper to clamp input time t to [0.0, 1.0]
    inline float clamp01(float t) {
        return std::max(0.0f, std::min(1.0f, t));
    }

    // --- Fractional Easing Functions (Input t [0,1], Output eased fraction [0,1]) ---

    inline float linearF(float t) {
        return t;
    }

    inline float inSineF(float t) {
        return 1.0f - std::cos((t * PI) / 2.0f);
    }

    inline float outSineF(float t) {
        return std::sin((t * PI) / 2.0f);
    }

    inline float inOutSineF(float t) {
        return -(std::cos(PI * t) - 1.0f) / 2.0f;
    }

    inline float inQuadF(float t) {
        return t * t;
    }

    inline float outQuadF(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    inline float inOutQuadF(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }

    // --- Interpolating Easing Functions (Input start, end, t [0,1], Output interpolated value) ---

    // Generic template to apply any fractional ease
    template<typename Func>
    inline float ease(float start, float end, float t, Func easingFuncF) {
        float fraction = easingFuncF(clamp01(t));
        return start + (end - start) * fraction;
    }

    // Specific ease functions calling the generic template
    inline float linear(float start, float end, float t) {
        return ease(start, end, t, linearF);
    }

    inline float inSine(float start, float end, float t) {
        return ease(start, end, t, inSineF);
    }

    inline float outSine(float start, float end, float t) {
        return ease(start, end, t, outSineF);
    }
    
    inline float inOutSine(float start, float end, float t) {
        return ease(start, end, t, inOutSineF);
    }

    inline float inQuad(float start, float end, float t) {
        return ease(start, end, t, inQuadF);
    }

    inline float outQuad(float start, float end, float t) {
        return ease(start, end, t, outQuadF);
    }

    inline float inOutQuad(float start, float end, float t) {
        return ease(start, end, t, inOutQuadF);
    }

} // namespace Easing
} // namespace PixelTheater 