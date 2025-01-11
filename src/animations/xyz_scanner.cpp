#include "animations/xyz_scanner.h"

void XYZScanner::init(const AnimParams& params) {
    Animation::init(params);
    speed = params.getFloat("speed", 0.005);
    blend = params.getInt("blend", 160);
    fade_amount = params.getInt("fade", 35);
}

void XYZScanner::tick() {
    FastLED.clear();
    CRGB c = CRGB(0,0,0);
    
    target = 140 + cos(counter/700.0)*130;
    target = constrain(target, 0, 255);
    
    for (int i = 0; i < numLeds(); i++) {    
        // z anim  
        float dz = (zi - points[i].z);
        if (abs(dz) < target) {
            float off = constrain(target - abs(dz), 0, max_range);
            c = CRGB(0, 0, map(off, min_off, target, 0, 200));
            nblend(leds[i], c, blend);
        }
        
        // y anim
        float dy = (yi - points[i].y);
        if (abs(dy) < target) {
            float off = constrain(target - abs(dy), 0, max_range);
            c = CRGB(map(off, min_off, target, 0, 200), 0, 0);
            nblend(leds[i], c, blend);
        }
        
        // x anim
        float dx = (xi - points[i].x);
        if (abs(dx) < target) {
            float off = constrain(target - abs(dx), 0, max_range);
            c = CRGB(0, map(off, min_off, target, 0, 200), 0);
            nblend(leds[i], c, blend);
        } 
    }
    
    // Update positions
    zi = (zi + speed*cos(counter/2000.0)*2);
    zi = constrain(zi, -max_range, max_range);
    if (abs(zi)==max_range) zi=-zi;
    
    yi = (yi + speed*constrain(tan(counter/1600.0)/4, -3, 3));
    yi = constrain(yi, -max_range, max_range);
    if (abs(yi)==max_range) yi=-yi;
    
    xi = (xi + speed*sin(counter/4000.0)*2);
    xi = constrain(xi, -max_range, max_range);
    if (abs(xi)==max_range) xi=-xi;
    
    fadeToBlackBy(leds, numLeds(), fade_amount);
    counter++;
} 