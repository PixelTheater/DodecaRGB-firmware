#pragma once

#include <cstddef> // For size_t
#include "PixelTheater/core/crgb.h" // Path relative to include root

namespace PixelTheater {

// Forward declaration if needed, or include directly
// class CRGB; 

/**
 * @brief Interface for accessing an LED buffer.
 * 
 * This abstracts the underlying LED data storage (e.g., a raw array, 
 * FastLED CRGB array, etc.) allowing Scenes to interact with LEDs 
 * in a platform-agnostic way.
 */
class ILedBuffer {
public:
    virtual ~ILedBuffer() = default; // Virtual destructor is important for interfaces

    /**
     * @brief Get a reference to the LED at the specified index.
     * 
     * @param index The index of the LED.
     * @return CRGB& Reference to the LED color object.
     * Implementations should handle invalid index gracefully (e.g., clamp index,
     * return dummy reference) rather than throwing exceptions.
     */
    virtual CRGB& led(size_t index) = 0;

    /**
     * @brief Get a const reference to the LED at the specified index.
     * 
     * @param index The index of the LED.
     * @return const CRGB& Const reference to the LED color object.
     * Implementations should handle invalid index gracefully (e.g., clamp index,
     * return dummy reference) rather than throwing exceptions.
     */
    virtual const CRGB& led(size_t index) const = 0;

    /**
     * @brief Get the total number of LEDs in the buffer.
     * 
     * @return size_t The number of LEDs.
     */
    virtual size_t ledCount() const = 0;

    // Future consideration: Add a method to get a span or iterator?
    // virtual std::span<CRGB> leds() = 0; 
    // virtual std::span<const CRGB> leds() const = 0;
};

} // namespace PixelTheater 