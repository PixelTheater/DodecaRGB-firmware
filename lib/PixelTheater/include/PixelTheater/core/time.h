#pragma once
#include <cstdint>

// Platform-specific includes must be outside namespace
#ifdef PLATFORM_NATIVE
#include <chrono>
#elif defined(PLATFORM_TEENSY)
#include <Arduino.h>
#endif

namespace PixelTheater {

// Base class must be defined before derived classes
class TimeProvider {
public:
    virtual uint32_t millis() = 0;     // Milliseconds since start
    virtual uint32_t micros() = 0;     // Microseconds since start
    virtual ~TimeProvider() = default;
};

// Platform-specific implementations
#ifdef PLATFORM_NATIVE
class SystemTimeProvider : public TimeProvider {
private:
    using clock = std::chrono::steady_clock;
    const clock::time_point _start = clock::now();
public:
    uint32_t millis() override {
        auto now = clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - _start);
        return duration.count();
    }

    uint32_t micros() override {
        auto now = clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - _start);
        return duration.count();
    }
};

#elif defined(PLATFORM_TEENSY)
class SystemTimeProvider : public TimeProvider {
public:
    uint32_t millis() override { return ::millis(); }
    uint32_t micros() override { return ::micros(); }
};

#else
#error "No system time implementation for this platform"
#endif

// Default implementation for controlled testing
class DefaultTimeProvider : public TimeProvider {
private:
    uint32_t _millis_offset = 0;
    uint32_t _micros_offset = 0;

public:
    uint32_t millis() override {
        return _millis_offset;
    }

    uint32_t micros() override {
        return _micros_offset;
    }

    // Test helpers
    void advance(uint32_t ms) {
        // Ensure microseconds stay in sync with milliseconds
        _micros_offset = (_millis_offset + ms) * 1000;
        _millis_offset += ms;
    }

    void reset() {
        _millis_offset = 0;
        _micros_offset = 0;
    }
};

// Helper to get the system time provider
inline TimeProvider& getSystemTimeProvider() {
    static SystemTimeProvider provider;
    return provider;
}

} // namespace PixelTheater 