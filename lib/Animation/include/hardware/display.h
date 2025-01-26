#pragma once
#include "../display.h"
#include "types.h"

namespace Animation {

class HardwareDisplay : public Display {
public:
    explicit HardwareDisplay(const HardwareConfig& config);
    ~HardwareDisplay() override = default;

    // Core interface
    void setPixel(int i, RGB c) override;
    RGB getPixel(int i) const override;
    const Point& getPoint(int i) const override;
    
    // Hardware-specific info
    size_t size() const override { return _config.num_leds; }
    size_t numSides() const { return _config.num_sides; }
    size_t ledsPerSide() const { return _config.leds_per_side; }

private:
    HardwareConfig _config;
    std::vector<Point> _converted_points;
};

} // namespace Animation 