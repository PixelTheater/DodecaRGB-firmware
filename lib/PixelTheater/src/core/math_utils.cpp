#include "PixelTheater/core/math_utils.h"

namespace PixelTheater {

// Saturating math functions
uint8_t qadd8(uint8_t i, uint8_t j) {
    unsigned int t = i + j;
    if (t > 255) t = 255;
    return t;
}

uint8_t qsub8(uint8_t i, uint8_t j) {
    int t = i - j;
    if (t < 0) t = 0;
    return t;
}

uint8_t scale8(uint8_t i, uint8_t scale) {
    return (((uint16_t)i * (1 + (uint16_t)scale)) >> 8);
}

// --- Linear Interpolation ---
uint8_t lerp8by8(uint8_t a, uint8_t b, uint8_t frac) {
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

} // namespace PixelTheater 