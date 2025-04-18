#include "PixelTheater/color/conversions.h"
#include "PixelTheater/core/crgb.h" // For CRGB/CHSV definitions
#include "PixelTheater/core/math_utils.h" // For scale8
#include <algorithm> // For std::min, std::max

// Include FastLED on Teensy for potential use in rgb2hsv_approximate if needed
#ifdef PLATFORM_TEENSY
#include <FastLED.h>
#endif

namespace PixelTheater {

// --- Helper for non-Teensy RGB->HSV --- (Used by C++ fallback)
static uint8_t approx_hue(const PixelTheater::CRGB& rgb, uint8_t min_val, uint8_t max_val, uint8_t delta) {
    if (delta == 0) return 0;

    uint8_t hue;
    if (max_val == rgb.r) {
        hue = ((rgb.g - rgb.b) * 43) / delta;
    } else if (max_val == rgb.g) {
        hue = 85 + ((rgb.b - rgb.r) * 43) / delta;
    } else { // max_val == rgb.b
        hue = 171 + ((rgb.r - rgb.g) * 43) / delta;
    }
    return hue;
}

// Provides the standard C++ fallback implementation for RGB to HSV conversion
// This is also used by the get_perceived_brightness fallback
PixelTheater::CHSV rgb2hsv_approximate(const PixelTheater::CRGB& rgb) {
    // On Teensy, we could potentially call FastLED's rgb2hsv_approximate
    // But the project plan seems to imply using a C++ fallback regardless?
    // Let's stick to the C++ implementation for consistency across platforms
    // unless FastLED version is explicitly required later.

    uint8_t min_val = std::min({rgb.r, rgb.g, rgb.b});
    uint8_t max_val = std::max({rgb.r, rgb.g, rgb.b});
    uint8_t delta = max_val - min_val;

    PixelTheater::CHSV hsv;
    hsv.v = max_val; // Value is the max component
    if (hsv.v == 0) { // Black
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = static_cast<uint16_t>(delta) * 255 / hsv.v; // Saturation
    if (hsv.s == 0) { // Grey
         hsv.h = 0;
         return hsv;
    }

    hsv.h = approx_hue(rgb, min_val, max_val, delta); // Approximate Hue

    return hsv;
}

// HSV conversion implementation (moved from core/color.cpp)
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

// Operator implementations
CRGB operator|(const CHSV& hsv, const CRGB&) {
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);
    return rgb;
}

CRGB& operator%=(CRGB& rgb, const CHSV& hsv) {
    hsv2rgb_rainbow(hsv, rgb);
    return rgb;
}

} // namespace PixelTheater 