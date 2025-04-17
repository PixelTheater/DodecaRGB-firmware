#include "PixelTheater/color_utils.h"
#include <cmath> // For fabs, sqrt, atan2, acos etc.
#include <algorithm> // For std::min, std::max 
#include <vector> // For ColorName lookup
#include <cstdio> // For snprintf

// Note: Palettes (basePalette, highlightPalette, uniquePalette) were moved to palettes.h

namespace PixelTheater {
namespace ColorUtils {

// Define the lookup table using the PixelTheater::CRGB type
// This table is used regardless of the platform
const std::vector<ColorName> colorLookup = {
    {"Black", CRGB::Black},
    {"Red", CRGB::Red},
    {"Green", CRGB::Green},
    {"Blue", CRGB::Blue},
    {"Yellow", CRGB::Yellow},
    {"Cyan", CRGB::Cyan},
    {"Magenta", CRGB::Magenta},
    {"White", CRGB::White},
    {"Orange", CRGB::Orange},
    {"Purple", CRGB::Purple},
    {"Pink", CRGB::Pink},
    {"Aqua", CRGB::Aqua},
    {"Chartreuse", CRGB::Chartreuse},
    {"Coral", CRGB::Coral},
    {"Gold", CRGB::Gold},
    {"Lavender", CRGB::Lavender},
    {"Lime", CRGB::Lime},
    {"Maroon", CRGB::Maroon},
    {"Navy", CRGB::Navy},
    {"Olive", CRGB::Olive},
    {"Plum", CRGB::Plum},
    {"Salmon", CRGB::Salmon},
    {"SeaGreen", CRGB::SeaGreen},
    {"Sienna", CRGB::Sienna},
    {"Silver", CRGB::Silver},
    {"Teal", CRGB::Teal},
    {"Turquoise", CRGB::Turquoise},
    {"Violet", CRGB::Violet},
    {"Wheat", CRGB::Wheat},
    {"Crimson", CRGB::Crimson},
    {"DarkBlue", CRGB::DarkBlue},
    {"DarkGreen", CRGB::DarkGreen}
};

std::string getClosestColorName(const PixelTheater::CRGB& color) {
    if (colorLookup.empty()) return "";

    const ColorName* closest = &colorLookup[0];
    uint32_t closestDistance = colorDistance(color, colorLookup[0].color);

    for (size_t i = 1; i < colorLookup.size(); ++i) {
        uint32_t dist = colorDistance(color, colorLookup[i].color);
        if (dist < closestDistance) {
            closest = &colorLookup[i];
            closestDistance = dist;
        }
        if (dist == 0) break; // Exact match found
    }
    return std::string(closest->name);
}

uint32_t colorDistance(const PixelTheater::CRGB& c1, const PixelTheater::CRGB& c2) {
    long long dr = (long long)c1.r - c2.r;
    long long dg = (long long)c1.g - c2.g;
    long long db = (long long)c1.b - c2.b;
    return dr * dr + dg * dg + db * db; 
}

std::string getAnsiColorString(const PixelTheater::CRGB& color, const char c) {
    char buf[64];
    snprintf(buf, sizeof(buf),
             "\033[48;2;%d;%d;%dm%c\033[0m", 
             color.r, color.g, color.b, c);
    return std::string(buf);
}

// --- Helper for non-Teensy RGB->HSV ---
// (Moved outside #ifdef as it provides the standard C++ fallback)
static uint8_t approx_hue(const PixelTheater::CRGB& rgb, uint8_t min_val, uint8_t max_val, uint8_t delta) {
    if (delta == 0) return 0;

    uint8_t hue;
    if (max_val == rgb.r) {
        hue = ((rgb.g - rgb.b) * 43) / delta; 
    } else if (max_val == rgb.g) {
        hue = 85 + ((rgb.b - rgb.r) * 43) / delta;
    } else { // max_val == rgb.b
        hue = 171 + ((rgb.r - rgb.g) * 43) / delta;
    }
    return hue;
}

// Provides the standard C++ fallback implementation for RGB to HSV conversion
PixelTheater::CHSV rgb2hsv_approximate(const PixelTheater::CRGB& rgb) {
    uint8_t min_val = std::min({rgb.r, rgb.g, rgb.b});
    uint8_t max_val = std::max({rgb.r, rgb.g, rgb.b});
    uint8_t delta = max_val - min_val;

    PixelTheater::CHSV hsv;
    hsv.v = max_val; // Value is the max component
    if (hsv.v == 0) { // Black
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = static_cast<uint16_t>(delta) * 255 / hsv.v; // Saturation
    if (hsv.s == 0) { // Grey
         hsv.h = 0;
         return hsv;
    }
    
    hsv.h = approx_hue(rgb, min_val, max_val, delta); // Approximate Hue

    return hsv;
}

// --- Platform Specific Implementations or Stubs for Brightness ---

#ifdef PLATFORM_TEENSY
// Use real FastLED functions when on Teensy
float get_perceived_brightness(const PixelTheater::CHSV& color) {
    // Convert PixelTheater::CHSV to ::CRGB using FastLED's conversion
    // Note: PixelTheater::CHSV needs to be convertible or compatible with ::CHSV
    // Assuming direct cast or appropriate constructor/operator exists.
    // If PixelTheater::CHSV doesn't map directly, we need an explicit conversion.
    // Let's assume hsv2rgb_rainbow can be used:
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