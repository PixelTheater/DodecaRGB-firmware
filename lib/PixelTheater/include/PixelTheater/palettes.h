#pragma once

#include "PixelTheater/core/crgb.h" // Provides PixelTheater::CRGB
#include <array>                   // For std::array

namespace PixelTheater {

// Define the standard palette type using std::array for cross-platform use.
using CRGBPalette16 = std::array<PixelTheater::CRGB, 16>;

// Make standard FastLED palettes available only when compiling for Teensy
// Note: When PLATFORM_TEENSY is defined, FastLED's TProgmemRGBPalette16
// might also be available globally. We declare our versions here for a 
// consistent API via PixelTheater::Palettes namespace, using PixelTheater::CRGB.
// <FastLED.h> should only be included in .cpp files where needed.

// Namespace for predefined palettes
namespace Palettes {
    // --- Standard FastLED Palettes (Declarations) ---
    extern const CRGBPalette16 CloudColors;       // Equivalent to CloudColors_p
    extern const CRGBPalette16 LavaColors;        // Equivalent to LavaColors_p
    extern const CRGBPalette16 OceanColors;       // Equivalent to OceanColors_p
    extern const CRGBPalette16 ForestColors;      // Equivalent to ForestColors_p
    extern const CRGBPalette16 RainbowColors;     // Equivalent to RainbowColors_p
    extern const CRGBPalette16 RainbowStripeColors; // Equivalent to RainbowStripeColors_p
    extern const CRGBPalette16 PartyColors;       // Equivalent to PartyColors_p
    extern const CRGBPalette16 HeatColors;        // Equivalent to HeatColors_p

    // --- Custom PixelTheater Palettes (Declarations) ---
    extern const CRGBPalette16 basePalette;       // Ported from old color_utils
    extern const CRGBPalette16 highlightPalette;  // Ported from old color_utils
    extern const CRGBPalette16 uniquePalette;     // Ported from old color_utils

    // --- Add declarations for more custom palettes here --- 

} // namespace Palettes
} // namespace PixelTheater 