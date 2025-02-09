#pragma once
#include "PixelTheater/core/time.h"
#include <Arduino.h>

namespace PixelTheater {

class ArduinoTimeProvider : public TimeProvider {
public:
    uint32_t millis() override {
        return ::millis();  // Arduino's millis()
    }

    uint32_t micros() override {
        return ::micros();  // Arduino's micros()
    }
};

} // namespace PixelTheater 