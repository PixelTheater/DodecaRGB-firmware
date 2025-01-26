#pragma once
#include <Arduino.h>
#include "time_provider.h"

class ArduinoTimeProvider : public TimeProvider {
public:
    unsigned long millis() const override {
        return ::millis();
    }
}; 