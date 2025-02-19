#pragma once
#include <cstdint>
#include "../limits.h"

namespace PixelTheater {

// Base template for model definitions
template<uint16_t NumLeds, uint8_t NumFaces>
struct ModelDefinition {
    // Required constants
    static constexpr uint16_t LED_COUNT = NumLeds;
    static constexpr uint8_t FACE_COUNT = NumFaces;
    
    // Required metadata
    static constexpr const char* NAME = nullptr;
    static constexpr const char* VERSION = nullptr;
    static constexpr const char* DESCRIPTION = nullptr;

    // Validation at compile time
    static_assert(LED_COUNT <= Limits::ABSOLUTE_MAX_LEDS, 
                 "LED count exceeds maximum");
    static_assert(FACE_COUNT <= Limits::ABSOLUTE_MAX_FACES,
                 "Face count exceeds maximum");

    // Face type constants (for reference)
    struct FaceTypes {
        static constexpr uint8_t NONE = 0;
        static constexpr uint8_t STRIP = 1;
        static constexpr uint8_t CIRCLE = 2;
        static constexpr uint8_t TRIANGLE = 3;
        static constexpr uint8_t SQUARE = 4;
        static constexpr uint8_t PENTAGON = 5;
        static constexpr uint8_t HEXAGON = 6;
        // static constexpr uint8_t HEPTAGON = 7;  // TODO
        // static constexpr uint8_t OCTAGON = 8;
    };
    
    // Region type constants (for reference)
    struct RegionTypes {
        static constexpr uint8_t CENTER = 0;
        static constexpr uint8_t RING = 1;
        static constexpr uint8_t EDGE = 2;
        // static constexpr uint8_t WEDGE = 3;  // TODO
        // static constexpr uint8_t PATCH = 4;  // TODO
    };
};

} // namespace PixelTheater 