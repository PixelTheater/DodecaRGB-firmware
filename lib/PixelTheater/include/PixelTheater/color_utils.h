#pragma once

#include "PixelTheater/core/crgb.h" // Include core type definitions
#include <string> // Use std::string instead of Arduino String
#include <vector> // Needed for ColorName lookup if we use vector
#include <cstdint>
#include <cmath> // For std::abs

namespace PixelTheater {
namespace ColorUtils {

// Structure to hold color name and value for lookup
struct ColorName {
    const char* name;
    PixelTheater::CRGB color; // Use PixelTheater::CRGB
};

// Find the closest matching named color from the lookup table
std::string getClosestColorName(const PixelTheater::CRGB& color); // Use std::string

// Calculate Euclidean distance squared between two colors in RGB space
uint32_t colorDistance(const PixelTheater::CRGB& c1, const PixelTheater::CRGB& c2);

// Convert RGB color to an ANSI 24-bit escape code string for terminal output (background)
std::string getAnsiColorString(const PixelTheater::CRGB& color, const char c = ' '); // Use std::string

// Calculate perceived brightness (luminance) of a color (0.0 - 1.0)
float get_perceived_brightness(const PixelTheater::CHSV& color); // Use PixelTheater::CHSV

// Calculate contrast ratio between two colors (WCAG formula)
float get_contrast_ratio(const PixelTheater::CHSV& color1, const PixelTheater::CHSV& color2);

// Calculate the shortest distance between two hues on the color wheel (0-180 degrees)
float get_hue_distance(const PixelTheater::CHSV& color1, const PixelTheater::CHSV& color2);

// Fallback / Stub function declarations needed for non-Teensy builds
PixelTheater::CHSV rgb2hsv_approximate(const PixelTheater::CRGB& rgb); 

// --- Linear Interpolation ---

/**
 * @brief Linear interpolation between two 8-bit values.
 *
 * @param a The first value.
 * @param b The second value.
 * @param frac The 8-bit fraction (0-255) specifying the blend amount towards b.
 * @return uint8_t The interpolated value.
 */
inline uint8_t lerp8by8(uint8_t a, uint8_t b, uint8_t frac) {
    uint16_t result;
    if (b > a) {
        uint16_t delta = b - a;
        result = a + (((delta * frac) + 128) >> 8);
    } else {
        uint16_t delta = a - b;
        result = a - (((delta * frac) + 128) >> 8);
    }
    return (uint8_t)result;
}

} // namespace ColorUtils
} // namespace PixelTheater 