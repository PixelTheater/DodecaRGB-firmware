#pragma once
#include <cstdint>
#include <memory>
#include "model/model.h"
#include "core/crgb.h"  // Need complete type for array access

namespace PixelTheater {

class Stage {
public:
    Stage() noexcept;
    ~Stage();


private:
    CRGB* _leds{nullptr};  // Points to FastLED array
};

} // namespace PixelTheater 