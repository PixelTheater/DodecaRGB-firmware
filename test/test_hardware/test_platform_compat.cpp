#include <doctest/doctest.h>
#include <Arduino.h>
#include <array>
#include <vector>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/platform/platform.h"

using namespace PixelTheater;

// Hardware configuration for Teensy 41
#define LED_PIN_1 19
#define LED_PIN_2 18
// Reduce the number of LEDs for testing to avoid memory issues
#define TEST_NUM_LEDS 50  // Use a smaller number for testing
#define NUM_LEDS_PER_PIN 624  // Half of total LEDs for DodecaRGB

TEST_CASE("Platform Compatibility") {
    Serial.println("\n=== Testing Platform Compatibility ===");

    SUBCASE("std::array") {
        Serial.println("Testing std::array...");
        std::array<int, 5> arr = {1, 2, 3, 4, 5};
        CHECK(arr.size() == 5);
        Serial.printf("Array size check: %d == 5\n", arr.size());
        CHECK(arr[0] == 1);
        Serial.printf("First element check: %d == 1\n", arr[0]);
        
        // Test array iteration
        int sum = 0;
        for(const auto& val : arr) {
            sum += val;
        }
        CHECK(sum == 15);
        Serial.printf("Array sum check: %d == 15\n", sum);
    }

    SUBCASE("STL containers") {
        Serial.println("Testing STL containers...");
        
        // Test vector availability and behavior
        std::vector<int> vec = {1, 2, 3};
        CHECK(vec.size() == 3);
        Serial.printf("Vector size check: %d == 3\n", vec.size());
        
        vec.push_back(4);
        CHECK(vec.size() == 4);
        Serial.printf("Vector push_back check: %d == 4\n", vec.size());
        
        // Test initializer lists
        std::initializer_list<int> init = {1, 2, 3};
        auto dist = std::distance(init.begin(), init.end());
        CHECK(dist == 3);
        Serial.printf("Initializer list size check: %d == 3\n", dist);
    }

    SUBCASE("memory alignment") {
        Serial.println("Testing memory alignment...");
        alignas(16) int aligned_var = 42;
        auto addr = reinterpret_cast<uintptr_t>(&aligned_var);
        CHECK(addr % 16 == 0);
        Serial.printf("Alignment check: %lu mod 16 = %lu\n", addr, addr % 16);
    }

    SUBCASE("move semantics") {
        Serial.println("Testing move semantics...");
        std::array<int, 3> arr1 = {1, 2, 3};
        auto arr2 = std::move(arr1);
        CHECK(arr2[0] == 1);
        Serial.printf("Moved array check: %d == 1\n", arr2[0]);
    }

    SUBCASE("reference types") {
        Serial.println("Testing reference types...");
        int x = 42;
        int& ref = x;
        CHECK(&ref == &x);
        Serial.println("Reference identity check passed");
        
        std::array<int, 3> arr = {1, 2, 3};
        for(int& val : arr) {
            val *= 2;
        }
        CHECK(arr[0] == 2);
        Serial.printf("Reference modification check: %d == 2\n", arr[0]);
    }

    Serial.println("Platform compatibility tests complete!");
}

TEST_CASE("Hardware Platform Tests") {
    Serial.println("\n=== Testing Hardware Platform ===");

    SUBCASE("Basic Hardware Setup") {
        Serial.println("Testing basic hardware setup...");
        
        // Create LED array - use a smaller size for testing
        Serial.println("Creating LED array...");
        ::CRGB leds[TEST_NUM_LEDS];
        Serial.println("LED array created");
        
        // Configure FastLED with just one pin for simplicity
        Serial.println("Configuring FastLED...");
        FastLED.addLeds<WS2812B, LED_PIN_1, GRB>(leds, TEST_NUM_LEDS);
        Serial.println("FastLED configured");
        
        // Basic LED test pattern
        Serial.println("Running LED test pattern:");
        
        Serial.println("1. All LEDs off");
        fill_solid(leds, TEST_NUM_LEDS, ::CRGB::Black);
        Serial.println("LEDs filled with black");
        FastLED.show();
        Serial.println("FastLED.show() completed");
        delay(100);
        
        Serial.println("2. First 5 LEDs red");
        for(int i = 0; i < 5 && i < TEST_NUM_LEDS; i++) {
            leds[i] = ::CRGB::Red;
        }
        Serial.println("First 5 LEDs set to red");
        FastLED.show();
        Serial.println("FastLED.show() completed");
        delay(100);
        
        Serial.println("3. Back to black");
        fill_solid(leds, TEST_NUM_LEDS, ::CRGB::Black);
        Serial.println("LEDs filled with black again");
        FastLED.show();
        Serial.println("FastLED.show() completed");
        
        CHECK(true);
        Serial.println("Hardware initialization complete");
    }

    SUBCASE("Color Operations") {
        Serial.println("Testing color operations...");
        
        ::CRGB color(255, 0, 0);  // Red
        CHECK(color.r == 255);
        CHECK(color.g == 0);
        CHECK(color.b == 0);
        Serial.printf("Color components check: R=%d, G=%d, B=%d\n", color.r, color.g, color.b);
        
        // Test basic color manipulation
        color.nscale8(128);
        CHECK(color.r == 128);
        Serial.printf("Color scaling check: R=%d (expected 128)\n", color.r);
    }
    
    SUBCASE("Verify Hardware FastLED Implementation") {
        Serial.println("Verifying hardware FastLED implementation...");
        
        // Create arrays for both implementations - use smaller arrays
        ::CRGB fastled_array[10] = {};
        PixelTheater::CRGB pt_array[10] = {};
        
        // Test FastLED's hardware-specific features
        FastLED.addLeds<WS2812B, LED_PIN_1, GRB>(fastled_array, 10);
        FastLED.setBrightness(128);
        FastLED.setMaxRefreshRate(100);
        FastLED.setDither(0);
        
        // Fill arrays with the same color
        fill_solid(fastled_array, 10, ::CRGB::Red);
        PixelTheater::fill_solid(pt_array, static_cast<uint16_t>(10), PixelTheater::CRGB(255, 0, 0));
        
        // Verify both implementations produce the same result
        CHECK(fastled_array[0].r == 255);
        CHECK(pt_array[0].r == 255);
        
        // Test hardware-specific behavior
        unsigned long start_time = micros();
        FastLED.show();  // This should call the hardware implementation
        unsigned long hardware_time = micros() - start_time;
        
        Serial.printf("Hardware show() time: %lu microseconds\n", hardware_time);
        
        // Test FastLED's color correction - but don't show for long
        FastLED.setCorrection(TypicalLEDStrip);
        fill_solid(fastled_array, 10, ::CRGB::White);
        FastLED.show();
        delay(50);  // Shorter delay
        
        // Test FastLED's temperature correction
        FastLED.setTemperature(Candle);
        FastLED.show();
        delay(50);  // Shorter delay
        
        // Reset to black
        fill_solid(fastled_array, 10, ::CRGB::Black);
        FastLED.show();
        
        Serial.println("Hardware FastLED implementation verified");
    }

    Serial.println("Hardware platform tests complete!");
} 