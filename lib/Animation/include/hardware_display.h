#pragma once
#include "display.h"
#include "hardware_types.h"  // Use real hardware types

namespace Animation {

class HardwareDisplay : public Display {
public:
    explicit HardwareDisplay(const HardwareConfig& config);
    ~HardwareDisplay() override = default;

    // Core display interface
    void setPixel(int i, RGB c) override;
    RGB getPixel(int i) const override;
    const Point& getPoint(int i) const override;
    
    // Hardware-specific info
    size_t size() const override;
    size_t numSides() const { return _config.num_sides; }
    size_t ledsPerSide() const { return _config.leds_per_side; }
    
    // Hardware capabilities
    bool supports3D() const override { return true; }
    bool supportsNeighbors() const override { return true; }
    std::vector<int> getNeighbors(int i) const override;

private:
    HardwareConfig _config;
    std::vector<Point> _converted_points;  // Store converted points
};

} // namespace Animation 