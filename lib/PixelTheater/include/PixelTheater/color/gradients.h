// Auto-generated palette data
#pragma once
#include <cstdint>
#include <cstddef> // For size_t

namespace PixelTheater {

// Definition for the Gradient Palette Data structure
struct GradientPaletteData {
    const uint8_t* data;
    size_t size;
};


    // WLED Rainbow
    constexpr uint8_t PALETTE_RAINBOW_DATA[] = { 0, 255, 0, 0, 42, 255, 255, 0, 85, 0, 255, 0, 128, 0, 255, 255, 170, 0, 0, 255, 212, 255, 0, 255, 255, 255, 0, 0 };

    constexpr GradientPaletteData PALETTE_RAINBOW = {
        PALETTE_RAINBOW_DATA,
        sizeof(PALETTE_RAINBOW_DATA)
    };

    // WLED Party
    constexpr uint8_t PALETTE_PARTY_DATA[] = { 0, 255, 0, 0, 85, 255, 255, 0, 170, 0, 255, 255, 255, 255, 0, 255 };

    constexpr GradientPaletteData PALETTE_PARTY = {
        PALETTE_PARTY_DATA,
        sizeof(PALETTE_PARTY_DATA)
    };

    // WLED Ocean Breeze
    constexpr uint8_t PALETTE_OCEAN_BREEZE_DATA[] = { 0, 0, 32, 96, 64, 0, 128, 192, 128, 0, 192, 255, 192, 64, 224, 255, 255, 192, 255, 255 };

    constexpr GradientPaletteData PALETTE_OCEAN_BREEZE = {
        PALETTE_OCEAN_BREEZE_DATA,
        sizeof(PALETTE_OCEAN_BREEZE_DATA)
    };

    // WLED Sunset Real
    constexpr uint8_t PALETTE_SUNSET_REAL_DATA[] = { 0, 176, 10, 0, 32, 192, 32, 0, 64, 208, 58, 0, 96, 224, 88, 0, 128, 240, 128, 0, 160, 255, 172, 0, 192, 255, 255, 0, 224, 255, 255, 128, 255, 255, 255, 255 };

    constexpr GradientPaletteData PALETTE_SUNSET_REAL = {
        PALETTE_SUNSET_REAL_DATA,
        sizeof(PALETTE_SUNSET_REAL_DATA)
    };

    // WLED RGI 15
    constexpr uint8_t PALETTE_RGI_15_DATA[] = { 0, 255, 192, 0, 64, 0, 128, 255, 128, 0, 255, 0, 192, 0, 128, 255, 255, 255, 192, 0 };

    constexpr GradientPaletteData PALETTE_RGI_15 = {
        PALETTE_RGI_15_DATA,
        sizeof(PALETTE_RGI_15_DATA)
    };

    // WLED Forest
    constexpr uint8_t PALETTE_FOREST_DATA[] = { 0, 0, 255, 0, 85, 255, 255, 0, 170, 0, 128, 0, 255, 0, 255, 0 };

    constexpr GradientPaletteData PALETTE_FOREST = {
        PALETTE_FOREST_DATA,
        sizeof(PALETTE_FOREST_DATA)
    };

} // namespace PixelTheater
