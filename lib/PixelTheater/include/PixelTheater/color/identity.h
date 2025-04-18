#pragma once

#include "../core/crgb.h" // Include core type definitions
#include <string> // Use std::string instead of Arduino String
#include <vector> // Needed for ColorName lookup if we use vector
#include <cstdint>

namespace PixelTheater {
namespace ColorUtils { // Keep the namespace for clarity within color/ subfolder

// Structure to hold color name and value for lookup
struct ColorName {
    const char* name;
    PixelTheater::CRGB color; // Use PixelTheater::CRGB
};

// Find the closest matching named color from the lookup table
std::string getClosestColorName(const PixelTheater::CRGB& color); // Use std::string

// Convert RGB color to an ANSI 24-bit escape code string for terminal output (background)
std::string getAnsiColorString(const PixelTheater::CRGB& color, const char c = ' '); // Use std::string

} // namespace ColorUtils
} // namespace PixelTheater 