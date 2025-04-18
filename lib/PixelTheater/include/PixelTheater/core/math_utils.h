#pragma once
#include <cstdint>

namespace PixelTheater {

// Forward declarations
class CRGB;
class CHSV;

// Saturating math functions
uint8_t qadd8(uint8_t i, uint8_t j);
uint8_t qsub8(uint8_t i, uint8_t j);
uint8_t scale8(uint8_t i, uint8_t scale);

// Linear Interpolation
uint8_t lerp8by8(uint8_t a, uint8_t b, uint8_t frac);

} // namespace PixelTheater 