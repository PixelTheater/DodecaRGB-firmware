#pragma once
#include <cstdint>

#ifdef PLATFORM_NATIVE
// Mock FastLED types for native tests
class CRGB {
public:
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint8_t raw[3];
    };
    
    CRGB(uint8_t r_ = 0, uint8_t g_ = 0, uint8_t b_ = 0) 
        : r(r_), g(g_), b(b_) {}

    void fadeToBlackBy(uint8_t amount) {
        r = ((uint16_t)r * (256 - amount)) >> 8;
        g = ((uint16_t)g * (256 - amount)) >> 8;
        b = ((uint16_t)b * (256 - amount)) >> 8;
    }
};

class CRGBPalette16 {
    CRGB entries[16];
public:
    CRGBPalette16() = default;
    CRGB& operator[](uint8_t index) { return entries[index & 15]; }
    const CRGB& operator[](uint8_t index) const { return entries[index & 15]; }
};

// Common palettes needed by tests
extern const CRGBPalette16 RainbowColors_p;
extern const CRGBPalette16 OceanColors_p;
extern const CRGBPalette16 LavaColors_p;

inline CRGB ColorFromPalette(const CRGBPalette16& pal, uint8_t index) {
    return pal[index >> 4];
}

#else
#include <FastLED.h>
#endif 