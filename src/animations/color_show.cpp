#include <vector>
#include <FastLED.h>
#include "animations/color_show.h"
#include "palettes.h"

ColorShow::Segment ColorShow::createRandomSegment() {
    return Segment{
        .start_led = static_cast<uint8_t>(random(numLeds())),
        .len = random8(3, 8),
        .pos = random(100)/100.0f,
        .speed = (random(4, 20)/60000.0f) * (random(2) ? 1 : -1),
        .center_color = ColorFromPaletteExtended(highlightPalette, random16(), 150, LINEARBLEND),
        .lifetime = random(1200, 5000)
    };
}

void ColorShow::init(const AnimParams& params) {
    int num_segments = 300;
    segments.clear();
    segments.reserve(num_segments);
    
    for (int i = 0; i < num_segments; i++) {
        segments.push_back(createRandomSegment());
    }
}

void ColorShow::tick() {
    fadeToBlackBy(leds, numLeds(), 10);

    for (auto& seg : segments) {
        // Calculate the current position in LED space
        int current_pos = (int)(seg.pos * numLeds());
        
        // Light up LEDs in the segment
        for (int i = 0; i < seg.len; i++) {
            int led_pos = (current_pos + i) % numLeds();
            if (led_pos < 0) led_pos += numLeds();  // Handle negative positions
            nblend(leds[led_pos], seg.center_color, 2);
        }
        
        // Update position
        seg.pos += seg.speed;
        if (seg.pos >= 1.0f) seg.pos -= 1.0f;
        if (seg.pos < 0.0f) seg.pos += 1.0f;
        
        // reduce brightness over time. if lifetime is less than 500, fade out
        if (seg.lifetime < 250) {
            seg.center_color.fadeToBlackBy(1);
        }

        // Update lifetime and reset if expired
        if (--seg.lifetime <= 0) {
            seg = createRandomSegment();
        }
    }
}

String ColorShow::getStatus() const {
    output.printf("Position: %d Color: %d\n", show_pos, show_color);
    
    // Calculate average lifetime
    float avg_lifetime = 0;
    int active_segments = 0;
    for (const auto& seg : segments) {
        avg_lifetime += seg.lifetime;
        if (seg.lifetime > 0) active_segments++;
    }
    avg_lifetime = active_segments > 0 ? avg_lifetime / active_segments : 0;
    
    output.printf("Active Segments: %d, Avg Lifetime: %.1f\n", active_segments, avg_lifetime);
    
    // Add color information like Sparkles does
    CRGB current_color = CHSV(show_color, 255, 255);
    output.print(getAnsiColorString(current_color));
    output.printf(" Current Color: %02X%02X%02X (%s)", 
        current_color.r, current_color.g, current_color.b,
        getClosestColorName(current_color).c_str());
    
    return output.get();
} 