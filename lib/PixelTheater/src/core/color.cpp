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

// HSV conversion implementation
void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb) {
    // Handle grayscale case
    if(hsv.sat == 0) {
        rgb.r = rgb.g = rgb.b = hsv.val;
        return;
    }

    uint8_t hue = hsv.hue;
    uint8_t sat = hsv.sat;
    uint8_t val = hsv.val;

    uint8_t offset = hue & 0x1F; // 0..31
    uint8_t offset8 = offset << 3; // 0..248
    
    uint8_t third = scale8(offset8, (256/3)); // max = 85
    uint8_t twothirds = scale8(offset8, ((256 * 2) / 3)); // max = 170

    uint8_t r, g, b;

    if(!(hue & 0x80)) { // 0XX
        if(!(hue & 0x40)) { // 00X
            if(!(hue & 0x20)) { // 000
                r = 255 - third;
                g = third;
                b = 0;
            } else { // 001
                r = 171;
                g = 85 + third;
                b = 0;
            }
        } else { // 01X
            if(!(hue & 0x20)) { // 010
                r = 171 - twothirds;
                g = 170 + third;
                b = 0;
            } else { // 011
                r = 0;
                g = 255 - third;
                b = third;
            }
        }
    } else { // 1XX
        if(!(hue & 0x40)) { // 10X
            if(!(hue & 0x20)) { // 100
                r = 0;
                g = 171 - twothirds;
                b = 85 + third;
            } else { // 101
                r = third;
                g = 0;
                b = 255 - third;
            }
        } else { // 11X
            if(!(hue & 0x20)) { // 110
                r = 85 + third;
                g = 0;
                b = 171 - twothirds;
            } else { // 111 - pink section
                r = 170;  // Was 171 + third
                g = 0;
                b = 85;   // Was 171 - twothirds
            }
        }
    }

    // Scale down colors if we're desaturated at all
    if(sat != 255) {
        if(sat == 0) {
            r = g = b = 255;  // Full white
        } else {
            uint8_t desat = 255 - sat;
            // Use exact scale8_video formula
            desat = ((int)desat * (int)desat >> 8) + ((desat && desat) ? 1 : 0);
            uint8_t satscale = 255 - desat;
            
            // Scale using exact scale8_video formula
            if(r) r = ((int)r * (int)satscale >> 8) + ((r && satscale) ? 1 : 0);
            if(g) g = ((int)g * (int)satscale >> 8) + ((g && satscale) ? 1 : 0);
            if(b) b = ((int)b * (int)satscale >> 8) + ((b && satscale) ? 1 : 0);
            
            // Add the brightness floor
            uint8_t brightness_floor = desat;
            r += brightness_floor;
            g += brightness_floor;
            b += brightness_floor;
        }
    }

    // Scale everything by value at the end using same approach
    if(val != 255) {
        if(val == 0) {
            r = g = b = 0;
        } else {
            if(r) r = ((int)r * (int)val >> 8) + 1;
            if(g) g = ((int)g * (int)val >> 8) + 1;
            if(b) b = ((int)b * (int)val >> 8) + 1;
        }
    }

    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
}

void nscale8(CRGB* leds, uint16_t count, uint8_t scale) {
    for(uint16_t i = 0; i < count; i++) {
        nscale8(leds[i], scale);
    }
}

} // namespace PixelTheater 