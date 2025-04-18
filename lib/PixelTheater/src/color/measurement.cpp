#include "PixelTheater/color/measurement.h"
#include "PixelTheater/color/conversions.h" // For rgb2hsv_approximate
#include "PixelTheater/core/crgb.h" // For CRGB/CHSV types

#include <cmath> // For fabs, sqrt, atan2, acos etc.
#include <algorithm> // For std::min, std::max
#include <cstdint>

// Include FastLED if on Teensy for platform-specific brightness
#ifdef PLATFORM_TEENSY
#include <FastLED.h>
#endif

namespace PixelTheater {
namespace ColorUtils {

uint32_t colorDistance(const PixelTheater::CRGB& c1, const PixelTheater::CRGB& c2) {
    long long dr = (long long)c1.r - c2.r;
    long long dg = (long long)c1.g - c2.g;
    long long db = (long long)c1.b - c2.b;
    return dr * dr + dg * dg + db * db;
}

// --- Platform Specific Implementations or Stubs for Brightness ---

#ifdef PLATFORM_TEENSY
// Use real FastLED functions when on Teensy
float get_perceived_brightness(const PixelTheater::CHSV& color) {
    // Convert PixelTheater::CHSV to ::CRGB using FastLED's conversion
    ::CRGB rgb;
    ::hsv2rgb_rainbow(reinterpret_cast<const ::CHSV&>(color), rgb);
    // Now calculate brightness from ::CRGB
    return (0.2126f * rgb.r + 0.7152f * rgb.g + 0.0722f * rgb.b) / 255.0f;
}

#else
// Fallback / Stub implementations for non-Teensy environments

// Stub for get_perceived_brightness using the CHSV value component
float get_perceived_brightness(const PixelTheater::CHSV& color) {
    // For non-Teensy, CHSV might not accurately represent RGB brightness.
    // Using V directly is the most practical stub approximation.
    return color.v / 255.0f; // Simplest stub: just use Value
}

#endif // PLATFORM_TEENSY

// These functions use the result of get_perceived_brightness,
// so they work on any platform as long as get_perceived_brightness is defined.
float get_contrast_ratio(const PixelTheater::CHSV& color1, const PixelTheater::CHSV& color2) {
    float l1 = get_perceived_brightness(color1);
    float l2 = get_perceived_brightness(color2);
    float lighter = std::max(l1, l2);
    float darker = std::min(l1, l2);
    return (lighter + 0.05f) / (darker + 0.05f);
}

float get_hue_distance(const PixelTheater::CHSV& color1, const PixelTheater::CHSV& color2) {
    // Calculate the absolute difference, handling wrapping correctly for uint8_t
    uint8_t diff = abs((int16_t)color1.h - color2.h); // Use signed intermediate for correct difference
    // Find the shortest distance around the wheel (max is 128)
    uint8_t shortest_diff = std::min(diff, (uint8_t)(256 - diff));
    // Convert shortest hue difference (0-128) to degrees (0-180)
    return shortest_diff * (180.0f / 128.0f);
}

} // namespace ColorUtils
} // namespace PixelTheater 