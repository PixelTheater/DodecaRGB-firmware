#include "PixelTheater/color/palette_api.h"
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/core/math_utils.h" // For lerp8by8

#include <cstdint>

// Include FastLED if on Teensy
#ifdef PLATFORM_TEENSY
#include <FastLED.h>
#endif

namespace PixelTheater {

// Function implementations will go here...

CRGB colorFromPalette(const CRGBPalette16& pal, 
                       uint8_t index, 
                       uint8_t brightness,
                       TBlendType blendType) {
#ifdef PLATFORM_TEENSY
    // On Teensy, use FastLED's ColorFromPalette
    // We need to cast our PixelTheater::CRGBPalette16 to ::CRGBPalette16
    // Assuming CRGBPalette16 is essentially std::array<CRGB, 16>
    // and PixelTheater::CRGB is layout-compatible with ::CRGB
    const ::CRGBPalette16& fastled_pal = reinterpret_cast<const ::CRGBPalette16&>(pal);
    ::TBlendType fastled_blend_type = (blendType == LINEARBLEND) ? ::LINEARBLEND : ::NOBLEND;
    
    // Call FastLED function (Need to include FastLED.h)
    ::CRGB result = ColorFromPalette(fastled_pal, index, brightness, fastled_blend_type);
    
    // Convert back to PixelTheater::CRGB (implicit conversion operator should handle this)
    return result;
#else
    // --- Native C++ Fallback Implementation --- 
    // Based on FastLED's ColorFromPalette logic

    // Calculate the fractional part and integer part of the index
    uint8_t index_adj = index;
    if (blendType == LINEARBLEND) {
        index_adj += 1; // Adjust index for linear blending
    }
    uint8_t hi4 = index_adj >> 4; // Integer part: palette entry index (0-15)
    uint8_t lo4 = index_adj & 0x0F; // Fractional part: blend amount (0-15)

    // Get the two palette entries
    CRGB entry1 = pal[hi4];
    CRGB entry2;
    if (blendType == LINEARBLEND) {
        // For linear blend, wrap around if necessary
        entry2 = pal[(hi4 + 1) & 0x0F]; 
    } else {
        // For no blend, use the same entry
        entry2 = pal[hi4];
    }

    // Calculate blend amount (scale 0-15 to 0-255)
    uint8_t blend_amount = (lo4 << 4) | lo4;

    // Blend the colors
    CRGB blended_color;
    if (blendType == LINEARBLEND) {
        // Inline blend logic (similar to nblend/blend8 but maybe simplified for palette)
        // FastLED uses a slightly different blend for palettes:
        // blended = entry1 + (entry2 - entry1) * (blend_amount / 256)
        // which simplifies to lerp8
        blended_color.r = lerp8by8(entry1.r, entry2.r, blend_amount);
        blended_color.g = lerp8by8(entry1.g, entry2.g, blend_amount);
        blended_color.b = lerp8by8(entry1.b, entry2.b, blend_amount);
    } else { // NOBLEND
        blended_color = entry1; // No blending needed
    }
    
    // Apply brightness scaling if necessary
    if (brightness != 255) {
        // Use the CRGB nscale8 method which handles platform differences
        // (though here we are in the #else block, it's good practice)
        blended_color.nscale8(brightness);
    }

    return blended_color;
#endif                     
}

} // namespace PixelTheater 