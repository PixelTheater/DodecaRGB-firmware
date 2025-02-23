#include <doctest/doctest.h>
#include <Arduino.h>
#include <array>
#include <vector>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;

// Hardware configuration for Teensy 41
#define LED_PIN_1 19
#define LED_PIN_2 18
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
        
        // Create LED array
        ::CRGB leds[NUM_LEDS_PER_PIN * 2];
        Serial.println("LED array created");
        
        // Configure FastLED
        FastLED.addLeds<WS2812B, LED_PIN_1, GRB>(leds, 0, NUM_LEDS_PER_PIN);
        FastLED.addLeds<WS2812B, LED_PIN_2, GRB>(leds + NUM_LEDS_PER_PIN, NUM_LEDS_PER_PIN);
        Serial.println("FastLED configured");
        
        // Basic LED test pattern
        Serial.println("Running LED test pattern:");
        
        Serial.println("1. All LEDs off");
        fill_solid(leds, NUM_LEDS_PER_PIN * 2, ::CRGB::Black);
        FastLED.show();
        delay(500);
        
        Serial.println("2. First 5 LEDs red");
        for(int i = 0; i < 5; i++) {
            leds[i] = ::CRGB::Red;
        }
        FastLED.show();
        delay(500);
        
        Serial.println("3. Back to black");
        fill_solid(leds, NUM_LEDS_PER_PIN * 2, ::CRGB::Black);
        FastLED.show();
        
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

    Serial.println("Hardware platform tests complete!");
} 