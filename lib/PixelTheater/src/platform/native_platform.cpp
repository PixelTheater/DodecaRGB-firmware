#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/core/color.h"

namespace PixelTheater {

NativePlatform::NativePlatform(uint16_t num_leds) 
    : _num_leds(num_leds)
{
    _leds = new CRGB[num_leds]();  // () for zero-initialization
}

NativePlatform::~NativePlatform() {
    delete[] _leds;
}

CRGB* NativePlatform::getLEDs() {
    return _leds;
}

uint16_t NativePlatform::getNumLEDs() const {
    return _num_leds;
}

void NativePlatform::show() {
    // No-op in native environment
}

void NativePlatform::setBrightness(uint8_t brightness) {
    _brightness = brightness;
}

void NativePlatform::clear() {
    fill_solid(_leds, _num_leds, CRGB::Black);
}

void NativePlatform::setMaxRefreshRate(uint8_t fps) {
    _max_refresh_rate = fps;
}

void NativePlatform::setDither(uint8_t dither) {
    _dither = dither;
}

} // namespace PixelTheater 