// animation.h
#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <memory>
#include <vector>
#include <string>
#include "animation_params.h"

struct StatusBuffer {
    String buffer;
    
    void printf(const char* format, ...) {
        char buf[64];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        buffer += buf;
    }
    
    void println(const String& s) { buffer += s + "\n"; }
    void print(const String& s) { buffer += s; }
    
    String get() {
        String result = buffer;
        buffer = "";
        return result;
    }
};

class Animation {
protected:
    CRGB* leds = nullptr;
    uint8_t num_sides = 0;
    uint16_t leds_per_side = 0;
    AnimParams params;
    mutable StatusBuffer output;

public:
    Animation() = default;
    virtual ~Animation() = default;
    
    // First phase: Configure the LED setup (called by AnimationManager)
    void configure(CRGB* leds_, uint8_t num_sides_, uint16_t leds_per_side_) {
        leds = leds_;
        num_sides = num_sides_;
        leds_per_side = leds_per_side_;
    }
    
    // Second phase: Initialize animation-specific parameters
    virtual void init(const AnimParams& params_) {
        params = params_;
    }
    
    virtual void tick() = 0;
    virtual String getStatus() const { return output.get(); }
    
    uint16_t numLeds() const { return num_sides * leds_per_side; }
};

