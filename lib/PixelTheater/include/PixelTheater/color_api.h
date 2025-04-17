#pragma once

#include "core/crgb.h"
#include "palettes.h" // For CRGBPalette16
#include <cstdint>

namespace PixelTheater {

// Forward declare to avoid circular includes if needed later
// class CRGB;
// class CHSV;
// class CRGBPalette16;

// Blend type enum (mirroring FastLED)
enum TBlendType {
    LINEARBLEND = 0, ///< Linear interpolation between palette entries. High quality, slower.
    NOBLEND = 1      ///< No interpolation between palette entries. Low quality, faster.
};

// --- Platform-Independent Color API Functions ---

/**
 * @brief Get a color from a 16-entry palette.
 * 
 * Handles interpolation between entries based on blend type.
 * Note: Native/Web implementation currently only supports CRGBPalette16,
 * gradient palette support is Teensy-only via FastLED for now.
 * 
 * @param pal The 16-entry palette (CRGBPalette16).
 * @param index The 8-bit index (0-255) into the virtual 256-entry palette.
 * @param brightness Optional brightness scale (0-255).
 * @param blendType How to blend between the 16 entries (LINEARBLEND or NOBLEND).
 * @return PixelTheater::CRGB The calculated color.
 */
CRGB colorFromPalette(const CRGBPalette16& pal, 
                       uint8_t index, 
                       uint8_t brightness = 255, 
                       TBlendType blendType = LINEARBLEND);

/**                     
 * @brief Blend one color toward another by a specified amount.
 * 
 * @param p1 The starting color.
 * @param p2 The target color.
 * @param amount The amount (0-255) to blend toward the target.
 * @return PixelTheater::CRGB The blended color.
 */
CRGB blend(const CRGB& p1, const CRGB& p2, uint8_t amount);

/**
 * @brief Blend a variable amount of a color into an existing color.
 * 
 * Combines the two colors, scaling the overlay color by the given amount
 * before adding it to the existing color.
 * 
 * @param existing The background color.
 * @param overlay The color to add.
 * @param amount The amount (0-255) of the overlay color to add.
 * @return PixelTheater::CRGB The resulting color (updates existing by reference).
 */
// CRGB& nblend(CRGB& existing, const CRGB& overlay, uint8_t amount); // <<< REMOVED (defined inline in core/color.h)

/**
 * @brief Convert HSV color to RGB using FastLED's rainbow mapping.
 * 
 * @param hsv The input HSV color.
 * @param rgb The output RGB color (updated by reference).
 */
void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb);

// Add other necessary API functions here (e.g., fill_solid, fill_rainbow?)

} // namespace PixelTheater 