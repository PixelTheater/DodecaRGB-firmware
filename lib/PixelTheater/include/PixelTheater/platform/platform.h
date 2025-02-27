#pragma once

#include "PixelTheater/core/crgb.h"

namespace PixelTheater {

// Forward declare FastLED-compatible helper functions
// Remove this declaration to avoid ambiguity with the inline implementation below
// void fill_solid(CRGB* leds, size_t num_leds, const CRGB& color);
void nscale8(CRGB* leds, size_t num_leds, uint8_t scale);

class Platform {
public:
    virtual ~Platform() = default;

    // Core LED array management
    virtual CRGB* getLEDs() = 0;
    virtual uint16_t getNumLEDs() const = 0;

    // Hardware control operations
    virtual void show() = 0;
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual void clear() = 0;

    // Performance settings
    virtual void setMaxRefreshRate(uint8_t fps) = 0;
    virtual void setDither(uint8_t dither) = 0;
};

// FastLED-compatible helper function implementations for native environment
inline void fill_solid(CRGB* leds, uint16_t num_leds, const CRGB& color) {
    for (uint16_t i = 0; i < num_leds; i++) {
        leds[i] = color;
    }
}

inline void nscale8(CRGB* leds, uint16_t num_leds, uint8_t scale) {
    for (uint16_t i = 0; i < num_leds; i++) {
        leds[i] *= scale;
    }
}

} // namespace PixelTheater 