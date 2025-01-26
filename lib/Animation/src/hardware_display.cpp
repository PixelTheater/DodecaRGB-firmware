#include "hardware_display.h"

namespace Animation {

// Convert LED_Point to Point for display interface
Point toPoint(const LED_Point& led) {
    return Point{led.x, led.y, led.z};
}

HardwareDisplay::HardwareDisplay(const HardwareConfig& config)
    : _config(config) {
    // Pre-convert points if needed
    _converted_points.reserve(_config.num_leds);
    for (size_t i = 0; i < _config.num_leds; i++) {
        _converted_points.push_back(toPoint(_config.points[i]));
    }
}

void HardwareDisplay::setPixel(int i, RGB c) {
    if (i < _config.num_leds) {
        _config.leds[i] = c;
    }
}

RGB HardwareDisplay::getPixel(int i) const {
    return (i < _config.num_leds) ? _config.leds[i] : RGB();
}

const Point& HardwareDisplay::getPoint(int i) const {
    return _converted_points[i];  // Use converted points
}

size_t HardwareDisplay::size() const { 
    return _config.num_leds; 
}

std::vector<int> HardwareDisplay::getNeighbors(int i) const {
    if (i >= _config.num_leds) return {};
    
    std::vector<int> result;
    for (uint16_t n : _config.points[i].neighbors) {
        result.push_back(static_cast<int>(n));
    }
    return result;
}

} // namespace Animation 