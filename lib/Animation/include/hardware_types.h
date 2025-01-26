#pragma once
#include <vector>
#include <cstdint>
#include "point.h"  // For core Point type

namespace Animation {

// Basic RGB color type (matches FastLED's CRGB)
struct RGB {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    
    constexpr RGB() = default;
    constexpr RGB(uint8_t r_, uint8_t g_, uint8_t b_) 
        : r(r_), g(g_), b(b_) {}
};

// Physical LED point with neighbors
struct LED_Point : public Point {
    std::vector<uint16_t> neighbors;  // Indices of neighboring LEDs
    uint8_t side{0};                  // Which face this LED belongs to
    
    LED_Point() = default;
    LED_Point(float x_, float y_, float z_, uint8_t side_ = 0) 
        : Point(x_, y_, z_), side(side_) {}
};

// Complete hardware configuration
struct HardwareConfig {
    RGB* leds{nullptr};           // LED array
    const LED_Point* points{nullptr}; // Physical layout
    size_t num_leds{0};          // Total number of LEDs
    uint8_t num_sides{0};        // Number of faces
    uint16_t leds_per_side{0};   // LEDs per face
};

} // namespace Animation 