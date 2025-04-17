#pragma once
#include <cstdint>
#include <cstddef>  // for size_t
#include "core/crgb.h" // Use the core CRGB type
#include "palettes.h" // Include for CRGBPalette16 type alias

namespace PixelTheater {

// Optional wrapper class for palette data (either CRGBPalette16 or gradient)
class PaletteWrapper {
public:
    // Constants remain relevant for validation
    static constexpr size_t MIN_ENTRIES = 2;      // At least black and white (for gradient)
    static constexpr size_t MAX_ENTRIES = 16;     // Max gradient entries (matches CRGBPalette16)
    
    // Constructors
    PaletteWrapper(const CRGBPalette16& paletteRef); // Constructor for CRGBPalette16
    PaletteWrapper(const uint8_t* data, size_t bytes); // Constructor for gradient data

    // Simplified Validation
    bool isValid() const { return _isValid; }
    // Potentially add type query? isGradient(), isCRGBPalette16()?
    
    // Removed getColor() - use PixelTheater::colorFromPalette API function instead
    // Removed size() - use palette directly or underlying data size if needed.
    // Removed value_at() - internal detail.

private:
    // Internal representation can vary based on constructor
    // For simplicity, maybe just store validity and potentially type?
    // Or store pointers/references, but that adds complexity.
    // Let's keep it minimal: just validity for now.
    bool _isValid = false; 

    // Internal validation helpers (simplified)
    bool validateCRGBPalette16(const CRGBPalette16& paletteRef);
    bool validateGradientData(const uint8_t* data, size_t bytes);
};

} // namespace PixelTheater 