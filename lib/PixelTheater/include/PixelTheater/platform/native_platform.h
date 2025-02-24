#pragma once

#include "PixelTheater/core/crgb.h"
#include "platform.h"

namespace PixelTheater {

class NativePlatform : public Platform {
public:
    explicit NativePlatform(uint16_t num_leds);
    ~NativePlatform() override;

    // Prevent copying
    NativePlatform(const NativePlatform&) = delete;
    NativePlatform& operator=(const NativePlatform&) = delete;

    // Core LED array management
    CRGB* getLEDs() override;
    uint16_t getNumLEDs() const override;

    // Hardware control operations
    void show() override;
    void setBrightness(uint8_t brightness) override;
    void clear() override;

    // Performance settings
    void setMaxRefreshRate(uint8_t fps) override;
    void setDither(uint8_t dither) override;

private:
    CRGB* _leds{nullptr};
    uint16_t _num_leds{0};
    uint8_t _brightness{255};
    uint8_t _max_refresh_rate{0};
    uint8_t _dither{0};
};

} // namespace PixelTheater 