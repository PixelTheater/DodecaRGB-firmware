#include "animations/identify_sides.h"

void IdentifySides::init(const AnimParams& params) {
    speed = params.getFloat("speed", 1.0f);
}

void IdentifySides::tick() {
    // identify each side uniquely
    for (int s = 0; s < num_sides; s++) {
        // New color for each side using HSV for even distribution
        CRGB side_color = CHSV(s * 255/num_sides, 255, 150);
        
        // Light N LEDs in the center, where N = side number
        for (int i = 0; i <= s; i++) {
            leds[s * leds_per_side + i] = side_color;
        }
        
        // Light up the top row of LEDs
        for (int i = 62; i < 71; i++) {
            if (i == 63 || i == 69) continue;  // Skip corners due to ordering
            leds[s * leds_per_side + i] = side_color;
        }
    }
    
    // Fade all LEDs slightly each frame
    fadeToBlackBy(leds, numLeds(), 2);
}

String IdentifySides::getStatus() const {
    output.printf("Speed: %.2f\n", speed);
    return output.get();
} 