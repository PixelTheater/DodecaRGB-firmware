#pragma once
#include "led_span.h"
#include "point.h"
#include "region_type.h"

namespace PixelTheater {

class Region {
public:
    Region(RegionType type = RegionType::None) : _type(type) {}
    
    // Basic properties
    RegionType type() const { return _type; }
    const Point& center() const { return _center; }
    const LedSpan& leds() const { return _leds; }
    
private:
    RegionType _type;
    Point _center;
    LedSpan _leds;
    
    template<typename T> friend class Model;  // For initialization
};

} // namespace PixelTheater 