#pragma once

#include "../core/crgb.h" // Include core type definitions
#include <cstdint>

namespace PixelTheater {

// Fill functions (previously in core/color.h or similar)
void fill_solid(CRGB* leds, uint16_t numToFill, const CRGB& color);
void fill_rainbow(CRGB* leds, int numToFill, uint8_t initialHue, uint8_t deltaHue = 5);
void fill_gradient_RGB(CRGB* leds, uint16_t startpos, CRGB startcolor, uint16_t endpos, CRGB endcolor);

// Potentially add template versions or overloads for LedBufferWrapper etc. if needed

} // namespace PixelTheater 