#pragma once

#include "PixelTheater/core/crgb.h"

namespace PixelTheater {

class Platform {
public:
    virtual ~Platform() = default;

    // Core LED array management
    virtual CRGB* getLEDs() = 0;
    virtual size_t getNumLEDs() const = 0;

    // Hardware control operations
    virtual void show() = 0;
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual void clear() = 0;

    // Performance settings
    virtual void setMaxRefreshRate(uint8_t fps) = 0;
    virtual void setDither(uint8_t dither) = 0;
};
} 