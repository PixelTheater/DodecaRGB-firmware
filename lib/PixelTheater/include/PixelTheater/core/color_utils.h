#pragma once
#include <cstdint>

namespace PixelTheater {

// Forward declarations
class CRGB;
class CHSV;

// Saturating math functions
inline uint8_t qadd8(uint8_t i, uint8_t j) {
    unsigned int t = i + j;
    if (t > 255) t = 255;
    return t;
}

inline uint8_t qsub8(uint8_t i, uint8_t j) {
    int t = i - j;
    if (t < 0) t = 0;
    return t;
}

inline uint8_t scale8(uint8_t i, uint8_t scale) {
    return (((uint16_t)i * (1 + (uint16_t)scale)) >> 8);
}

// HSV conversion
void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb);

} // namespace PixelTheater 