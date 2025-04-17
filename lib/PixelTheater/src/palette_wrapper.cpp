#include "PixelTheater/palette_wrapper.h"

namespace PixelTheater {

// Constructor for CRGBPalette16
PaletteWrapper::PaletteWrapper(const CRGBPalette16& paletteRef) {
    _isValid = validateCRGBPalette16(paletteRef);
}

// Constructor for gradient data
PaletteWrapper::PaletteWrapper(const uint8_t* data, size_t bytes) {
    _isValid = validateGradientData(data, bytes);
}

// --- Private Validation Helpers ---

bool PaletteWrapper::validateCRGBPalette16(const CRGBPalette16& paletteRef) {
    // CRGBPalette16 is std::array<CRGB, 16>, so size is always 16.
    // No specific content validation needed for this type wrapper unless 
    // stricter rules were desired (e.g., no duplicate entries?)
    // For now, just accept it as valid.
    (void)paletteRef; // Mark as used
    return true; 
}

bool PaletteWrapper::validateGradientData(const uint8_t* data, size_t bytes) {
    if (!data || bytes == 0) return false;

    // Check size constraints (must be multiple of 4)
    if (bytes % 4 != 0) return false;
    size_t numEntries = bytes / 4;
    if (numEntries < MIN_ENTRIES || numEntries > MAX_ENTRIES) return false;

    // Check index rules
    // First index must be 0
    if (data[0] != 0) return false;
    // Last index must be 255
    if (data[(numEntries - 1) * 4] != 255) return false;
    // Check ascending order
    for (size_t i = 1; i < numEntries; i++) {
        uint8_t prev = data[(i - 1) * 4];
        uint8_t curr = data[i * 4];
        if (curr <= prev) return false;
    }

    // Optional: Check R,G,B values are 0-255 (should be inherent in uint8_t)

    return true;
}

} // namespace PixelTheater 