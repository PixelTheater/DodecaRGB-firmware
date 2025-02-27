#include <doctest/doctest.h>
#include <Arduino.h>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/core/color.h"
#include "PixelTheater/platform/platform.h"
#include "PixelTheater/platform/fastled_platform.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/scene.h"
#include "PixelTheater/stage.h"
#include "PixelTheater/limits.h"

// Function to report available memory - renamed to avoid conflict with test_large_led_array.cpp
uint32_t BenchmarkFreeMem() {
    // Teensy 4.x specific memory reporting
    extern uint8_t _heap_start;
    extern uint8_t _heap_end;
    extern uint8_t *__brkval;
    uint32_t free_memory;
    
    if ((uint32_t)__brkval == 0) {
        // If __brkval is 0, no heap allocations have been made yet
        free_memory = (uint32_t)&_heap_end - (uint32_t)&_heap_start;
    } else {
        // Otherwise, __brkval is the current heap pointer
        free_memory = (uint32_t)&_heap_end - (uint32_t)__brkval;
    }
    
    return free_memory;
}

TEST_CASE("Framerate Benchmark Comparison" * doctest::skip(true)) {
    Serial.println("\n=== Framerate Benchmark: FastLED vs PixelTheater ===");
    
    // Report initial memory
    Serial.printf("Initial free memory: %lu bytes\n", BenchmarkFreeMem());
    
    Serial.println("IMPLEMENTATION REMOVED...");
    // Report final memory
    Serial.printf("Final free memory: %lu bytes\n", BenchmarkFreeMem());
    
    Serial.println("\nFramerate benchmark complete!");
} 