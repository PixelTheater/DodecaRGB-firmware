#include <doctest/doctest.h>
#include "PixelTheater/core/time.h"
#include <thread>

using namespace PixelTheater;

TEST_SUITE("TimeProvider") {
    TEST_CASE("DefaultTimeProvider") {
        DefaultTimeProvider time;

        SUBCASE("basic timing") {
            time.reset();
            CHECK(time.millis() == 0);
            CHECK(time.micros() == 0);

            time.advance(100);  // 100ms
            CHECK(time.millis() == 100);
            CHECK(time.micros() == 100000);

            time.advance(50);   // +50ms
            time.advance(150);  // +150ms
            CHECK(time.millis() == 300);
            CHECK(time.micros() == 300000);
        }

        SUBCASE("micros precision") {
            time.reset();
            
            // Test single millisecond
            time.advance(1);  // 1ms = 1000µs
            CHECK(time.micros() == 1000);
            CHECK(time.millis() == 1);
            
            // Test multiple milliseconds
            time.advance(999);  // Total 1000ms
            CHECK(time.millis() == 1000);
            CHECK(time.micros() == 1000000);
        }
    }

    TEST_CASE("SystemTimeProvider") {
        auto& time = getSystemTimeProvider();
        
#ifdef PLATFORM_NATIVE
        // Native-specific timing tests
        auto start = time.millis();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto elapsed = time.millis() - start;
        CHECK(elapsed >= 95);
        CHECK(elapsed <= 105);
#endif

        // Platform-agnostic tests
        auto t1 = time.millis();
        auto t2 = time.millis();
        CHECK(t2 >= t1);
    }
} 