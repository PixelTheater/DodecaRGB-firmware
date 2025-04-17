#pragma once
#include <cstdint>
#include "crgb.h"
#include "color_utils.h"

namespace PixelTheater {

// Forward declarations
class CRGB;
void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb);
CRGB blend(const CRGB& color1, const CRGB& color2, uint8_t blend_amount);

// Helper function matching FastLED's blend8
inline uint8_t blend8(uint8_t a, uint8_t b, uint8_t amountOfB) {
    // Add 128 for proper rounding
    uint16_t result = (a * (255 - amountOfB) + b * amountOfB + 128);
    return (result + (result >> 8)) >> 8;
}

// Global color operations
inline void nblend(CRGB& existing, const CRGB& overlay, uint8_t amount) {
    // Edge cases for speed
    if(amount == 0) return;
    if(amount == 255) {
        existing = overlay;
        return;
    }
    
    // Use FastLED's blend8 function
    existing.r = blend8(existing.r, overlay.r, amount);
    existing.g = blend8(existing.g, overlay.g, amount);
    existing.b = blend8(existing.b, overlay.b, amount);
}

inline CRGB blend(const CRGB& color1, const CRGB& color2, uint8_t blend_amount) {
    CRGB result = color1;
    nblend(result, color2, blend_amount);
    return result;
}

// Overload for direct scaling
inline uint8_t scale8(uint8_t i, uint8_t scale, uint8_t /*unused*/) {
    return ((((uint16_t)i) * ((uint16_t)(scale)+1)) >> 8);
}

// Video scaling version that adds 1 to scale first
inline uint8_t scale8_video(uint8_t i, uint8_t scale) {
    return (((uint16_t)i * (uint16_t)scale) >> 8) + ((i && scale) ? 1 : 0);
}

// Add single-color version of nscale8 before it's used
inline void nscale8(CRGB& color, uint8_t scale) {
    // Delegate to the CRGB method which handles platform specifics
    color.nscale8(scale);
}

// Add HSV conversion functions
void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb);
void hsv2rgb_spectrum(const CHSV& hsv, CRGB& rgb);

// Array versions
void hsv2rgb_rainbow(const CHSV* hsv, CRGB* rgb, int count);
void hsv2rgb_spectrum(const CHSV* hsv, CRGB* rgb, int count);

// Analysis functions
inline uint8_t getAverageLight(const CRGB& color) {
    return (color.r + color.g + color.b) / 3;
}

// Fading operations
inline void fadeToBlackBy(CRGB& color, uint8_t amount) {
    // Delegate to the CRGB method which handles platform specifics
    color.fadeToBlackBy(amount);
}

inline void fadeLightBy(CRGB& color, uint8_t amount) {
    fadeToBlackBy(color, amount);
}

// Array fill operations
void fill_solid(CRGB* leds, uint16_t numToFill, const CRGB& color);
void fill_rainbow(CRGB* leds, int numToFill, uint8_t initialHue, uint8_t deltaHue);
void fill_gradient_RGB(CRGB* leds, uint16_t startpos, CRGB startcolor,
                      uint16_t endpos, CRGB endcolor);

// Template array operations
template<typename Range>
void fill_solid(Range& leds, const CRGB& color) {
    // Use a standard for loop as Range might not support std::begin/std::end (e.g., LedsProxy)
    for (size_t i = 0; i < leds.size(); ++i) {
        leds[i] = color;
    }
}

// Deprecated template function
template<typename Range>
[[deprecated("Use fill_solid instead to match FastLED API")]]
void fill(Range& leds, const CRGB& color) {
    fill_solid(leds, color);
}

} // namespace PixelTheater 