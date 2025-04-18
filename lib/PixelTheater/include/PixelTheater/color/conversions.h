#pragma once

#include "../core/crgb.h" // Include core type definitions

namespace PixelTheater {

// HSV <-> RGB Conversions
void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb);
PixelTheater::CHSV rgb2hsv_approximate(const PixelTheater::CRGB& rgb);

// Operator overloads - DECLARATIONS ONLY
CRGB operator|(const CHSV& hsv, const CRGB&);
CRGB& operator%=(CRGB& rgb, const CHSV& hsv);

} // namespace PixelTheater 