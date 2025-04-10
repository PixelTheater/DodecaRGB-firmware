#pragma once
#include "platform.h"
#include <FastLED.h>
#include <Arduino.h> // For millis(), random(), etc.
#include <HardwareSerial.h> // For Serial.printf

namespace PixelTheater {

class FastLEDPlatform : public Platform {
public:
    // Takes pointer to pre-configured FastLED array
    explicit FastLEDPlatform(CRGB* leds, uint16_t num_leds) 
        : _leds(leds)
        , _num_leds(num_leds)
    {}

    CRGB* getLEDs() override { return _leds; }
    uint16_t getNumLEDs() const override { return _num_leds; }
    
    void show() override { FastLED.show(); }
    void setBrightness(uint8_t b) override { FastLED.setBrightness(b); }
    void clear() override { FastLED.clear(); }
    
    void setMaxRefreshRate(uint8_t fps) override { FastLED.setMaxRefreshRate(fps); }
    void setDither(uint8_t dither) override { FastLED.setDither(dither); }

    // --- Utility Method Implementations ---
    float deltaTime() override;
    uint32_t millis() override;

    uint8_t random8() override;
    uint16_t random16() override;
    uint32_t random(uint32_t max = RAND_MAX) override;
    uint32_t random(uint32_t min, uint32_t max) override;
    float randomFloat() override;
    float randomFloat(float max) override;
    float randomFloat(float min, float max) override;

    // Add explicit override declarations for variadic log functions
    void logInfo(const char* format, ...) override;
    void logWarning(const char* format, ...) override;
    void logError(const char* format, ...) override;

private:
    CRGB* _leds;
    uint16_t _num_leds;
};

// --- Inline Implementations (or move to .cpp) ---
// Add definitions if they weren't already inline
inline float FastLEDPlatform::deltaTime() {
    static uint32_t last_millis = 0;
    uint32_t current_millis = ::millis();
    float dt = (current_millis - last_millis) / 1000.0f;
    last_millis = current_millis;
    return min(dt, 0.1f); 
}
inline uint32_t FastLEDPlatform::millis() { return ::millis(); }
inline uint8_t FastLEDPlatform::random8() { return random(0, 256); }
inline uint16_t FastLEDPlatform::random16() { return random(0, 65536); }
inline uint32_t FastLEDPlatform::random(uint32_t max) { return ::random(max); }
inline uint32_t FastLEDPlatform::random(uint32_t min, uint32_t max) { return ::random(min, max); }
inline float FastLEDPlatform::randomFloat() { return ::random(0, 1001) / 1000.0f; }
inline float FastLEDPlatform::randomFloat(float max) { return randomFloat() * max; }
inline float FastLEDPlatform::randomFloat(float min, float max) { 
    if (min >= max) return min;
    return min + randomFloat() * (max - min);
}
inline void FastLEDPlatform::logInfo(const char* format, ...) {
    if (Serial) {
        va_list args;
        va_start(args, format);
        char buffer[256]; // Or use a more dynamic approach if needed
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.printf("[INFO] %s\n", buffer);
    }
}
inline void FastLEDPlatform::logWarning(const char* format, ...) {
     if (Serial) {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.printf("[WARN] %s\n", buffer);
    }
}
inline void FastLEDPlatform::logError(const char* format, ...) {
     if (Serial) {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.printf("[ERROR] %s\n", buffer);
    }
}

} // namespace PixelTheater 