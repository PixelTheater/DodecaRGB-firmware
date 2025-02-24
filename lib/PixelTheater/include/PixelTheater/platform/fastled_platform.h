#pragma once
#include "platform.h"
#include <FastLED.h>

namespace PixelTheater {

class FastLEDPlatform : public Platform {
public:
    // Takes pointer to pre-configured FastLED array
    explicit FastLEDPlatform(CRGB* leds, uint16_t num_leds) 
        : _leds(leds)
        , _num_leds(num_leds)
    {}

    CRGB* getLEDs() override { return _leds; }
    uint16_t getNumLEDs() const override { return _num_leds; }
    
    void show() override { FastLED.show(); }
    void setBrightness(uint8_t b) override { FastLED.setBrightness(b); }
    void clear() override { FastLED.clear(); }
    
    void setMaxRefreshRate(uint8_t fps) override { FastLED.setMaxRefreshRate(fps); }
    void setDither(uint8_t dither) override { FastLED.setDither(dither); }

private:
    CRGB* _leds;
    uint16_t _num_leds;
};

} // namespace PixelTheater 