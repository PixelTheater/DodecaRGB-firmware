#pragma once
#include <cstdint>

namespace PixelTheater {

// Forward declarations
class CRGB;
CRGB blend(const CRGB& color1, const CRGB& color2, uint8_t blend_amount);

// FastLED's scale8 function - used for color scaling and HSV conversion
inline uint8_t scale8(uint8_t i, uint8_t scale) {
    return (((uint16_t)i * (1 + (uint16_t)scale)) >> 8);
}

class CRGB {
public:
    union {
        struct {
            union {
                uint8_t r;
                uint8_t red;
            };
            union {
                uint8_t g; 
                uint8_t green;
            };
            union {
                uint8_t b;
                uint8_t blue;
            };
        };
        uint8_t raw[3];
    };

    // Constructors
    CRGB() : r(0), g(0), b(0) {}
    CRGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    CRGB(uint32_t colorcode) {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = colorcode & 0xFF;
    }

    // Common operations
    void fadeToBlackBy(uint8_t amount) {
        // Add 1 to handle rounding
        r = ((uint16_t)r * (256 - amount)) >> 8;
        g = ((uint16_t)g * (256 - amount)) >> 8;
        b = ((uint16_t)b * (256 - amount)) >> 8;
    }

    void fadeLightBy(uint8_t amount) {
        fadeToBlackBy(amount);
    }

    uint8_t getAverageLight() const {
        return (r + g + b) / 3;
    }

    void nscale8(uint8_t scale) {
        if (scale == 255) {
            // No change needed
            return;
        }
        if (scale == 0) {
            // Fast path to black
            r = g = b = 0;
            return;
        }
        // Use uint16_t to prevent overflow and match FastLED behavior
        r = ((uint16_t)r * (1 + scale)) >> 8;
        g = ((uint16_t)g * (1 + scale)) >> 8;
        b = ((uint16_t)b * (1 + scale)) >> 8;
    }

    // Common colors
    static const CRGB Black;   // = CRGB(0, 0, 0)
    static const CRGB White;   // = CRGB(255, 255, 255)
    static const CRGB Red;     // = CRGB(255, 0, 0)
    static const CRGB Green;   // = CRGB(0, 255, 0)
    static const CRGB Blue;    // = CRGB(0, 0, 255)
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