#pragma once
#include <cstdint>
#include "crgb.h"
#include "color_utils.h"

namespace PixelTheater {

// Forward declarations
class CRGB;
class CHSV;
void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb);
CRGB blend(const CRGB& color1, const CRGB& color2, uint8_t blend_amount);

class CHSV {
public:
    union {
        struct {
            union {
                uint8_t h;
                uint8_t hue;
            };
            union {
                uint8_t s;
                uint8_t sat;
                uint8_t saturation;
            };
            union {
                uint8_t v;
                uint8_t val;
                uint8_t value;
            };
        };
        uint8_t raw[3];
    };

    // Constructors
    CHSV() : h(0), s(0), v(0) {}
    CHSV(uint8_t hue, uint8_t sat, uint8_t val) : h(hue), s(sat), v(val) {}

    // Common HSV color points
    static const uint8_t HUE_RED = 0;
    static const uint8_t HUE_ORANGE = 32;
    static const uint8_t HUE_YELLOW = 64;
    static const uint8_t HUE_GREEN = 96;
    static const uint8_t HUE_AQUA = 128;
    static const uint8_t HUE_BLUE = 160;
    static const uint8_t HUE_PURPLE = 192;
    static const uint8_t HUE_PINK = 224;
};

// Helper function matching FastLED's blend8
inline uint8_t blend8(uint8_t a, uint8_t b, uint8_t amountOfB) {
    uint16_t result = (a * (255 - amountOfB) + b * amountOfB) + 128;
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

// Add HSV conversion functions
void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb);
void hsv2rgb_spectrum(const CHSV& hsv, CRGB& rgb);

// Array versions
void hsv2rgb_rainbow(const CHSV* hsv, CRGB* rgb, int count);
void hsv2rgb_spectrum(const CHSV* hsv, CRGB* rgb, int count);

// Automatic conversion operator
inline CRGB operator|(const CHSV& hsv, const CRGB&) {
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);
    return rgb;
}

// Assignment operator for automatic conversion
inline CRGB& operator%=(CRGB& rgb, const CHSV& hsv) {
    hsv2rgb_rainbow(hsv, rgb);
    return rgb;
}

} // namespace PixelTheater 