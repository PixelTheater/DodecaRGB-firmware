#pragma once
#include "PixelTheater/core/array_view.h"
#include "led.h"

namespace PixelTheater {

// Represents a non-contiguous collection of LEDs that form a region
class LedSpan {
public:
    LedSpan() = default;
    
    // Initialize from LED array and indices
    LedSpan(Led* leds, const uint16_t* indices, size_t count)
        : _leds(leds)
        , _indices(indices)
        , _count(count)
    {}
    
    // Access methods
    Led& operator[](size_t index) {
        return index < _count ? _leds[_indices[index]] : _leds[0];
    }
    
    const Led& operator[](size_t index) const {
        return index < _count ? _leds[_indices[index]] : _leds[0];
    }
    
    size_t size() const { return _count; }

private:
    Led* _leds{nullptr};              // Base LED array
    const uint16_t* _indices{nullptr}; // Indices into LED array
    size_t _count{0};                // Number of LEDs in span
};

} // namespace PixelTheater 