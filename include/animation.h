// animation.h
#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <memory>
#include <vector>
#include <string>
#include "animation_params.h"
#include "points.h"

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
public:
    Animation() = default;
    virtual ~Animation() = default;
    
    // First phase: Configure the LED setup (called by AnimationManager)
    virtual void configure(CRGB* leds_, const LED_Point* points_, uint8_t num_sides_, uint16_t leds_per_side_) {
        leds = leds_;
        points = points_;
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
    
    virtual const char* getName() const = 0;  // Pure virtual
    virtual AnimParams getDefaultParams() const { return AnimParams(); }
    virtual AnimParams getPreset(const String& preset_name) const { return getDefaultParams(); }

    static void setBrightness(uint8_t b) { global_brightness = b; }
    static uint8_t getBrightness() { return global_brightness; }

protected:
    CRGB* leds = nullptr;
    const LED_Point* points = nullptr;
    uint8_t num_sides = 0;
    uint16_t leds_per_side = 0;
    AnimParams params;
    mutable StatusBuffer output;
    static uint8_t global_brightness;  // 0-255 brightness for all animations
};

