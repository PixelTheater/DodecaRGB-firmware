#pragma once
#include <cmath>
#include <cstdint>  // For int32_t, uint8_t
#include "../constants.h"  // For M_PI
#include "Core"  // Eigen includes
#include "Dense"
#include "math_platform.h"  // For constrain_value

namespace PixelTheater {

class MathProvider {
public:
    // Core Arduino-like functions
    virtual int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) = 0;
    virtual float map(float x, float in_min, float in_max, float out_min, float out_max) = 0;
    
    virtual int32_t clamp_value(int32_t x, int32_t min, int32_t max) = 0;
    virtual float clamp_value(float x, float min, float max) = 0;
    
    // Min/max functions
    template<typename T>
    T min(T a, T b) { return (a < b) ? a : b; }
    
    template<typename T>
    T max(T a, T b) { return (a > b) ? a : b; }
    
    // Absolute value
    virtual int32_t abs(int32_t x) = 0;
    virtual float abs(float x) = 0;

    // FastLED-style fixed point math
    virtual uint8_t sin8(uint8_t theta) = 0;
    virtual uint8_t cos8(uint8_t theta) = 0;
    
    // FastLED-style saturating arithmetic
    virtual uint8_t qadd8(uint8_t a, uint8_t b) = 0;  // Add with saturation at 0xFF
    virtual uint8_t qsub8(uint8_t a, uint8_t b) = 0;  // Subtract with saturation at 0x00
    
    // FastLED-style random number generation
    // Uses a 16-bit LCG optimized for 8/16-bit values
    // Primarily used for color/animation effects
    virtual uint8_t random8() = 0;                          // Random 0-255
    virtual uint8_t random8(uint8_t lim) = 0;              // Random 0 to (lim-1)
    virtual uint8_t random8(uint8_t min, uint8_t lim) = 0; // Random min to (lim-1)
    virtual uint16_t random16() = 0;                        // Random 0-65535
    virtual void random16_set_seed(uint16_t seed) = 0;     // Set FastLED PRNG seed

    // Arduino-compatible random number generation
    // Uses a 32-bit LCG for general purpose randomness
    // Used for timing, positioning, and other non-visual effects
    virtual int32_t random(int32_t max) = 0;               // Random 0 to max-1
    virtual int32_t random(int32_t min, int32_t max) = 0;  // Random min to max-1
    virtual void setRandomSeed(uint32_t seed) = 0;         // Set Arduino PRNG seed

    virtual ~MathProvider() = default;
};

// Default implementation
class DefaultMathProvider : public MathProvider {
private:
    // FastLED-compatible lookup table for 8-bit trig functions
    static constexpr uint8_t SIN8_TABLE[256] = {
        128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173,
        177, 179, 182, 184, 187, 189, 192, 194, 197, 200, 202, 205, 207, 210, 212, 215,
        218, 219, 221, 223, 224, 226, 228, 229, 231, 233, 234, 236, 238, 239, 241, 243,
        245, 245, 246, 246, 247, 248, 248, 249, 250, 250, 251, 251, 252, 253, 253, 254,
        255, 254, 253, 253, 252, 251, 251, 250, 250, 249, 248, 248, 247, 246, 246, 245,
        245, 243, 241, 239, 238, 236, 234, 233, 231, 229, 228, 226, 224, 223, 221, 219,
        218, 215, 212, 210, 207, 205, 202, 200, 197, 194, 192, 189, 187, 184, 182, 179,
        177, 173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131,
        128, 125, 122, 119, 116, 113, 110, 107, 104, 101,  98,  95,  92,  89,  86,  83,
         79,  77,  74,  72,  69,  67,  64,  62,  59,  56,  54,  51,  49,  46,  44,  41,
         38,  37,  35,  33,  32,  30,  28,  27,  25,  23,  22,  20,  18,  17,  15,  13,
         11,  11,  10,  10,   9,   8,   8,   7,   6,   6,   5,   5,   4,   3,   3,   2,
          1,   2,   3,   3,   4,   5,   5,   6,   6,   7,   8,   8,   9,  10,  10,  11,
         11,  13,  15,  17,  18,  20,  22,  23,  25,  27,  28,  30,  32,  33,  35,  37,
         38,  41,  44,  46,  49,  51,  54,  56,  59,  62,  64,  67,  69,  72,  74,  77,
         79,  83,  86,  89,  92,  95,  98, 101, 104, 107, 110, 113, 116, 119, 122, 125
    };

    // FastLED uses a 16-bit PRNG state
    uint16_t _rand16seed = 1337; // Non-zero seed

    // LCG parameters from FastLED
    static constexpr uint16_t RAND16_MULTIPLIER = 2053;
    static constexpr uint16_t RAND16_ADD = 13849;

    uint32_t _random_seed = 1337;

public:
    int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) override {
        // Handle zero range case
        if (in_min == in_max) return out_min;
        
        // Match Arduino's implementation exactly
        return (long)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    float map(float x, float in_min, float in_max, float out_min, float out_max) override {
        // Handle zero range case
        if (in_min == in_max) return out_min;
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    int32_t clamp_value(int32_t x, int32_t min, int32_t max) override {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    float clamp_value(float x, float min, float max) override {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    int32_t abs(int32_t x) override { return std::abs(x); }
    float abs(float x) override { return std::abs(x); }

    uint8_t sin8(uint8_t theta) override {
        return SIN8_TABLE[theta];
    }

    uint8_t cos8(uint8_t theta) override {
      return SIN8_TABLE[(theta + 64) % 256];  // Quarter turn offset, ensure within 0-255
    }

    uint8_t qadd8(uint8_t a, uint8_t b) override {
        unsigned int sum = a + b;
        return sum > 255 ? 255 : sum;  // Saturate at 0xFF
    }

    uint8_t qsub8(uint8_t a, uint8_t b) override {
        return a > b ? a - b : 0;  // Saturate at 0x00
    }

    uint8_t random8() override {
        return random16() >> 8; // Use top 8 bits for better randomness
    }

    uint8_t random8(uint8_t lim) override {
        uint8_t r = random8();
        r = ((uint16_t)r * (uint16_t)lim) >> 8;
        return r;
    }

    uint8_t random8(uint8_t min, uint8_t lim) override {
        if (lim <= min) return min;
        return min + random8(lim - min);
    }

    uint16_t random16() override {
        _rand16seed = _rand16seed * RAND16_MULTIPLIER + RAND16_ADD;
        return _rand16seed;
    }

    void random16_set_seed(uint16_t seed) override {
        _rand16seed = seed ? seed : 1337;  // Ensure non-zero seed
    }

    int32_t random(int32_t max) override {
        // Use linear congruential generator with same parameters as Arduino
        _random_seed = _random_seed * 1103515245 + 12345;
        return _random_seed % max;
    }

    int32_t random(int32_t min, int32_t max) override {
        if (min >= max) return min;
        return min + random(max - min);
    }

    void setRandomSeed(uint32_t seed) override {
        _random_seed = seed ? seed : 1337;  // Ensure non-zero seed
    }
};

// Arduino-compatible map function
template<typename T>
static T map(T x, T in_min, T in_max, T out_min, T out_max) {
    // Use long for intermediate calculation to match Arduino behavior
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

} // namespace PixelTheater 