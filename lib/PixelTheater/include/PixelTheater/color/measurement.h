#pragma once

#include "../core/crgb.h" // Include core type definitions
#include <cstdint>

namespace PixelTheater {
namespace ColorUtils { // Keep the namespace for clarity within color/ subfolder

// Calculate Euclidean distance squared between two colors in RGB space
uint32_t colorDistance(const PixelTheater::CRGB& c1, const PixelTheater::CRGB& c2);

// Calculate perceived brightness (luminance) of a color (0.0 - 1.0)
float get_perceived_brightness(const PixelTheater::CHSV& color);

// Calculate contrast ratio between two colors (WCAG formula)
float get_contrast_ratio(const PixelTheater::CHSV& color1, const PixelTheater::CHSV& color2);

// Calculate the shortest distance between two hues on the color wheel (0-180 degrees)
float get_hue_distance(const PixelTheater::CHSV& color1, const PixelTheater::CHSV& color2);

} // namespace ColorUtils
} // namespace PixelTheater 