#pragma once

#include "PixelTheater/core/iled_buffer.h" // Correct path
#include "PixelTheater/core/crgb.h"      // Correct path
#include "PixelTheater/color/definitions.h"
#include <cstddef> // size_t
#include <stdexcept> // std::out_of_range
#include <cassert> // For assert()

namespace PixelTheater {

/**
 * @brief Concrete implementation of ILedBuffer wrapping a raw CRGB array.
 */
class LedBufferWrapper : public ILedBuffer {
private:
    CRGB* leds_ptr_;
    size_t num_leds_;

public:
    /**
     * @brief Construct a wrapper around an existing LED buffer.
     * 
     * @param leds Pointer to the first CRGB element.
     * @param num_leds Total number of LEDs in the buffer.
     */
    LedBufferWrapper(CRGB* leds, size_t num_leds)
        : leds_ptr_(leds), num_leds_(num_leds) 
    {
        assert((leds_ptr_ != nullptr || num_leds_ == 0) && "LedBufferWrapper: leds pointer cannot be null if num_leds > 0");
    }

    // Disable copy/move operations for simplicity if ownership is not intended
    LedBufferWrapper(const LedBufferWrapper&) = delete;
    LedBufferWrapper& operator=(const LedBufferWrapper&) = delete;
    LedBufferWrapper(LedBufferWrapper&&) = delete;
    LedBufferWrapper& operator=(LedBufferWrapper&&) = delete;

    ~LedBufferWrapper() override = default;

    CRGB& led(size_t index) override {
        // Clamp index instead of assert/throw
        if (index >= num_leds_) { 
            if (num_leds_ == 0) return dummyLed(); // Handle empty buffer case
            index = num_leds_ - 1; // Clamp to last valid index
        }
        // assert(index < num_leds_ && "LedBufferWrapper::led index out of range");
        return leds_ptr_[index];
    }

    const CRGB& led(size_t index) const override {
        // Clamp index instead of assert/throw
        if (index >= num_leds_) { 
             if (num_leds_ == 0) return dummyLed(); // Handle empty buffer case
            index = num_leds_ - 1; // Clamp to last valid index
        }
        // assert(index < num_leds_ && "LedBufferWrapper::led const index out of range");
        return leds_ptr_[index];
    }

    size_t ledCount() const override {
        return num_leds_;
    }

private:
    // Helper for returning a dummy LED when clamping an empty buffer
    static CRGB& dummyLed() {
        static CRGB dummy = CRGB::Black;
        return dummy;
    }
};

} // namespace PixelTheater 