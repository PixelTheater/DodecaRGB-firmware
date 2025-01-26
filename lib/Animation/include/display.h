#pragma once
#include <vector>
#include "point.h"
#include "color.h"
#include "hardware_types.h"

namespace Animation {

class CRGBPalette16;  // Forward declare

// Interface for addressing pixels in 3D space
class Display {
public:
    virtual ~Display() = default;  // Fix virtual destructor

    // Core display interface
    virtual void setPixel(int i, RGB c) = 0;
    virtual RGB getPixel(int i) const = 0;
    
    // Geometry interface
    virtual const Point& getPoint(int i) const = 0;
    virtual size_t size() const = 0;
    
    // Palette support
    virtual void setPalette(const CRGBPalette16& palette) { _palette = palette; }
    virtual CRGB colorFromPalette(uint8_t index) const { return ColorFromPalette(_palette, index); }

    // Optional capabilities
    virtual bool supports3D() const { return false; }
    virtual bool supportsNeighbors() const { return false; }
    virtual std::vector<int> getNeighbors(int i) const { return {}; }

protected:
    CRGBPalette16 _palette;
};

} // namespace Animation 