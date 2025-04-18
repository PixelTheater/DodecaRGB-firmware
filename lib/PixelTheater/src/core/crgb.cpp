#include "PixelTheater/core/crgb.h"
#include "PixelTheater/core/math_utils.h" // For scale8
#include "PixelTheater/color/conversions.h" // For hsv2rgb_rainbow needed in constructor/operator

// Include FastLED if on Teensy for CRGB methods
#ifdef PLATFORM_TEENSY
#include <FastLED.h>
#endif

namespace PixelTheater {

// Definitions for static colors MOVED to color/definitions.cpp
/*
const CRGB CRGB::Black = CRGB(0x000000);
// ... (rest of definitions commented out or removed)
const CRGB CRGB::FairyLightNCC = CRGB(0xFF9D2A);  // No color correction
*/

// LED RGB color that roughly approximates
// the color of incandescent fairy lights,
// assuming that you're using FastLED
// color correction on your LEDs (recommended).
// REMOVED - Definition moved to definitions.cpp
// const CRGB CRGB::FairyLight = CRGB(0xFFE42D);

// Implementation for fadeToBlackBy
CRGB& CRGB::fadeToBlackBy(uint8_t fadeBy) {
#ifdef PLATFORM_TEENSY
    // FastLED handles this directly on the CRGB object
    reinterpret_cast<::CRGB*>(this)->fadeToBlackBy(fadeBy);
#else
    // Use the scale8 function from math_utils
    uint8_t scale = 255 - fadeBy;
    r = scale8(r, scale);
    g = scale8(g, scale);
    b = scale8(b, scale);
#endif
    return *this;
}

// Implementation for nscale8
CRGB& CRGB::nscale8(uint8_t scaledown) {
#ifdef PLATFORM_TEENSY
    // FastLED handles this directly on the CRGB object
    reinterpret_cast<::CRGB*>(this)->nscale8(scaledown);
#else
    // Use the scale8 function from math_utils
    r = scale8(r, scaledown);
    g = scale8(g, scaledown);
    b = scale8(b, scaledown);
#endif
    return *this;
}

// Implementation for HSV constructor
CRGB::CRGB(const CHSV& rhs) {
    hsv2rgb_rainbow(rhs, *this);
}

// Implementation for HSV assignment operator
CRGB& CRGB::operator=(const CHSV& rhs) {
    hsv2rgb_rainbow(rhs, *this);
    return *this;
}

#ifdef PLATFORM_TEENSY
// Implementation for FastLED conversion operators/constructors
CRGB::operator ::CRGB() const {
    return ::CRGB(r, g, b);
}

CRGB::CRGB(const ::CRGB& rhs) : r(rhs.r), g(rhs.g), b(rhs.b) {}

CRGB& CRGB::operator=(const ::CRGB& rhs) {
    r = rhs.r;
    g = rhs.g;
    b = rhs.b;
    return *this;
}
#endif

} // namespace PixelTheater 