#pragma once
#include <cstdint>
#include "face_type.h"
#include "PixelTheater/core/crgb.h"

namespace PixelTheater {

class Face {
private:
    uint8_t _id;
    FaceType _type;
    uint16_t _led_offset;
    uint16_t _led_count;
    CRGB* _leds;  // Just a pointer to the LED array

public:
    Face() = default;
    
    Face(FaceType type, uint8_t id, uint16_t offset, uint16_t count, CRGB* leds)
        : _id(id)
        , _type(type)
        , _led_offset(offset)
        , _led_count(count)
        , _leds(leds)
        , leds{leds, offset, count}  // Initialize the leds member
    {}

    // Simple accessors
    uint8_t id() const { return _id; }
    FaceType type() const { return _type; }
    uint16_t led_offset() const { return _led_offset; }
    uint16_t led_count() const { return _led_count; }

    // LED array access
    struct Leds {
        CRGB* _data;
        uint16_t _offset;
        uint16_t _count;

        // Non-const access always allowed (like FastLED)
        CRGB& operator[](size_t i) const {  // Made const
            if (i >= _count) i = _count - 1;
            return _data[_offset + i];
        }

        void fill(const CRGB& color) const {  // Made const
            for(size_t i = 0; i < _count; i++) {
                _data[_offset + i] = color;
            }
        }
    } leds;  // Direct member, not a method

    // Collection operations
    void fill(const CRGB& color) {
        for(size_t i = 0; i < _led_count; i++) {
            _leds[_led_offset + i] = color;
        }
    }
};

} // namespace PixelTheater 