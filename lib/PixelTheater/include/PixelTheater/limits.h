#pragma once
#include <cstddef>     // for size_t
#include <cstdint>     // for uint8_t, uint16_t

namespace PixelTheater {

// Region limits
// These enable compile-time initialization of data structures
namespace Limits {
    // Maximum calculated point neighbors
    static constexpr size_t MAX_NEIGHBORS = 7;         // Per LED
    static constexpr float NEIGHBOR_THRESHOLD = 30.0f; // Based on existing points.cpp data

    // Maximum possible values for any model
    static constexpr size_t MAX_LEDS_PER_REGION = 32;
    static constexpr size_t MAX_LEDS_PER_FACE = 128;
    static constexpr size_t MAX_RINGS_PER_FACE = 8;
    
    // Absolute hardware limits for validation
    static constexpr size_t ABSOLUTE_MAX_LEDS = 10000;    // Sanity check
    static constexpr size_t ABSOLUTE_MAX_FACES = 32;      // Sanity check

    // New limits for region counts
    static constexpr size_t MAX_RINGS = 8;  // Max rings per face
    static constexpr size_t MAX_EDGES = 6;  // Max edges per face (hexagon)
    
} // namespace Limits
} // namespace PixelTheater 