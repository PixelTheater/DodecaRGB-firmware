// In PixelTheater/led.h
#pragma once
#include "core/crgb.h"
#include "model/point.h"

namespace PixelTheater {

// Individual LED with color, position and index
class Led {
private:
    CRGB* _color;  // Pointer instead of reference
    const Point* _point;
    size_t _index;

public:
    Led() : _color(nullptr), _point(nullptr), _index(0) {}  // Default constructor
    Led(CRGB& c, const Point& p, size_t i) 
        : _color(&c), _point(&p), _index(i) {}

    CRGB& color() { return *_color; }
    const Point& point() const { return *_point; }
    size_t index() const { return _index; }
};


} // namespace PixelTheater