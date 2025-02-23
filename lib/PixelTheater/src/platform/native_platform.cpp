#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/core/color.h"
#include <algorithm>

namespace PixelTheater {

NativePlatform::NativePlatform(size_t num_leds)
    : _leds(num_leds, CRGB::Black) // Initialize all LEDs to black
{
}

CRGB* NativePlatform::getLEDs() {
    return _leds.data();
}

size_t NativePlatform::getNumLEDs() const {
    return _leds.size();
}

void NativePlatform::show() {
    // Native environment doesn't need to do anything for show()
    // This would trigger the hardware update in FastLED
}

void NativePlatform::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    // In native environment, brightness is just stored
    // Hardware would apply this to the output
}

void NativePlatform::clear() {
    std::fill(_leds.begin(), _leds.end(), CRGB::Black);
}

void NativePlatform::setMaxRefreshRate(uint8_t fps) {
    _max_refresh_rate = fps;
    // Native environment just stores the value
    // Hardware would configure timing
}

void NativePlatform::setDither(uint8_t dither) {
    _dither = dither;
    // Native environment just stores the value
    // Hardware would configure dithering
}

} // namespace PixelTheater 