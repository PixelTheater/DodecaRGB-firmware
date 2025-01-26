#pragma once
#include <Animation/include/animation.h>
#include <FastLED.h>
#include "points.h"

// Hardware-specific extension of core Animation
class HardwareAnimation : public Animation {
public:
    void configure(CRGB* leds_, const LED_Point* points_, 
                  uint8_t num_sides_, uint16_t leds_per_side_);
    
protected:
    CRGB* leds = nullptr;
    const LED_Point* points = nullptr;
    uint8_t num_sides = 0;
    uint16_t leds_per_side = 0;
    
    uint16_t numLeds() const { return num_sides * leds_per_side; }
}; 