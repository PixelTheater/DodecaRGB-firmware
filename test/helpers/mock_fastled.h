#pragma once

namespace Animation {

// Basic RGB structure
struct CRGB {
    uint8_t r, g, b;
    CRGB(uint8_t r_ = 0, uint8_t g_ = 0, uint8_t b_ = 0) 
        : r(r_), g(g_), b(b_) {}
};

// Minimal CRGBPalette16 implementation
class CRGBPalette16 {
public:
    CRGBPalette16() = default;
    CRGB& operator[](uint8_t index) { return entries[index & 15]; }
    const CRGB& operator[](uint8_t index) const { return entries[index & 15]; }

private:
    CRGB entries[16];
};

// Common palettes needed by tests
extern const CRGBPalette16 RainbowColors_p;
extern const CRGBPalette16 OceanColors_p;
extern const CRGBPalette16 LavaColors_p;

// Minimal palette helper function
inline CRGB ColorFromPalette(const CRGBPalette16& pal, uint8_t index) {
    return pal[index >> 4];
}

} // namespace Animation 