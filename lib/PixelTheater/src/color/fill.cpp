#include "PixelTheater/core/color.h"

namespace PixelTheater {

void fill_solid(CRGB* leds, uint16_t numToFill, const CRGB& color) {
    for(uint16_t i = 0; i < numToFill; i++) {
        leds[i] = color;
    }
}

void fill_rainbow(CRGB* leds, int numToFill, uint8_t initialHue, uint8_t deltaHue) {
    CHSV hsv(0, 255, 255);  // Full saturation and value
    
    for(int i = 0; i < numToFill; i++) {
        hsv.hue = initialHue + (i * deltaHue);
        hsv2rgb_rainbow(hsv, leds[i]);
    }
}

void fill_gradient_RGB(CRGB* leds, uint16_t startpos, CRGB startcolor, 
                      uint16_t endpos, CRGB endcolor) {
    // Check if start and end are the same
    if (startpos == endpos) {
        leds[startpos] = startcolor;
        return;
    }

    // Add proper rounding to deltas
    int32_t rdelta = ((int32_t)endcolor.r - (int32_t)startcolor.r) * 255 / (endpos - startpos + 1);
    int32_t gdelta = ((int32_t)endcolor.g - (int32_t)startcolor.g) * 255 / (endpos - startpos + 1);
    int32_t bdelta = ((int32_t)endcolor.b - (int32_t)startcolor.b) * 255 / (endpos - startpos + 1);

    // Fill the range with interpolated colors
    for(uint16_t i = startpos; i <= endpos; i++) {
        int16_t offset = i - startpos;
        // Add 128 for proper rounding
        leds[i].r = startcolor.r + ((rdelta * offset + 128) >> 8);
        leds[i].g = startcolor.g + ((gdelta * offset + 128) >> 8);
        leds[i].b = startcolor.b + ((bdelta * offset + 128) >> 8);
    }
}

} // namespace PixelTheater 