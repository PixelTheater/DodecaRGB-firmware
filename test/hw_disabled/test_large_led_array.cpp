#include <doctest/doctest.h>
#include <Arduino.h>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/core/color.h"
#include "PixelTheater/platform/platform.h"

// Function to get free memory on Teensy
uint32_t FreeMem() {
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

// Test with more conservative LED counts to avoid crashes
#define SMALL_LED_COUNT 256
#define MEDIUM_LED_COUNT 512
#define LARGE_LED_COUNT 4096  // Start with 1024 as our "large" test

TEST_CASE("Large LED Array Handling") {
    Serial.println("\n=== Testing Large LED Array Handling ===");
    
    // Track memory before and after tests
    uint32_t initialFreeMemory = 0;
    uint32_t finalFreeMemory = 0;
    
    // Get initial free memory (Teensy-specific)
    initialFreeMemory = FreeMem();
    Serial.printf("Initial free memory: %u bytes\n", initialFreeMemory);

    SUBCASE("Memory Allocation for Large Arrays") {
        Serial.println("Testing memory allocation for large arrays...");
        
        // Test allocation of a large array on the heap
        Serial.printf("Allocating array with %u LEDs...\n", LARGE_LED_COUNT);
        CRGB* largeArray = new CRGB[LARGE_LED_COUNT];
        
        if (largeArray == nullptr) {
            Serial.println("ERROR: Failed to allocate large array");
            CHECK(false); // Force test failure
        } else {
            // Verify we can access the beginning and end of the array
            largeArray[0] = CRGB::Red;
            largeArray[LARGE_LED_COUNT - 1] = CRGB::Blue;
            
            CHECK(largeArray[0].r == 255);
            CHECK(largeArray[0].g == 0);
            CHECK(largeArray[0].b == 0);
            
            CHECK(largeArray[LARGE_LED_COUNT - 1].r == 0);
            CHECK(largeArray[LARGE_LED_COUNT - 1].g == 0);
            CHECK(largeArray[LARGE_LED_COUNT - 1].b == 255);
            
            Serial.println("Large array allocation and access verified");
            
            // Clean up
            delete[] largeArray;
            largeArray = nullptr;
        }
    }
    
    SUBCASE("uint16_t Index Handling") {
        Serial.println("Testing uint16_t index handling...");
        
        // Create an array just over the uint8_t limit to test uint16_t indexing
        const uint16_t testSize = 300;
        CRGB* testArray = new CRGB[testSize];
        
        if (testArray == nullptr) {
            Serial.println("ERROR: Failed to allocate test array");
            CHECK(false); // Force test failure
        } else {
            // Fill with a pattern using uint16_t indices
            for (uint16_t i = 0; i < testSize; i++) {
                testArray[i] = CRGB(i % 256, (i * 2) % 256, (i * 3) % 256);
            }
            
            // Verify values at specific indices, especially near uint8_t boundaries
            CHECK(testArray[255].r == 255);
            CHECK(testArray[256].r == 0);
            CHECK(testArray[257].r == 1);
            
            Serial.println("uint16_t index handling verified");
            
            // Clean up
            delete[] testArray;
            testArray = nullptr;
        }
    }
    
    SUBCASE("PixelTheater Operations with Large Arrays") {
        Serial.println("Testing PixelTheater operations with large arrays...");
        
        // Create separate arrays for FastLED and PixelTheater
        CRGB* fastled_array = new CRGB[MEDIUM_LED_COUNT];
        PixelTheater::CRGB* pt_array = new PixelTheater::CRGB[MEDIUM_LED_COUNT];
        
        bool allocationSucceeded = (fastled_array != nullptr && pt_array != nullptr);
        CHECK(allocationSucceeded);
        
        if (allocationSucceeded) {
            // Test PixelTheater's fill_solid with large count
            PixelTheater::fill_solid(pt_array, static_cast<uint16_t>(MEDIUM_LED_COUNT), PixelTheater::CRGB(0, 255, 0));
            
            // Verify random samples
            CHECK(pt_array[0].g == 255);
            CHECK(pt_array[0].r == 0);
            CHECK(pt_array[0].b == 0);
            
            CHECK(pt_array[MEDIUM_LED_COUNT/2].g == 255);
            CHECK(pt_array[MEDIUM_LED_COUNT - 1].g == 255);
            
            // Test performance comparison between FastLED and PixelTheater with large arrays
            unsigned long startTime, fastLedTime, pixelTheaterTime;
            
            // Measure FastLED fill_solid
            startTime = micros();
            fill_solid(fastled_array, MEDIUM_LED_COUNT, CRGB::Red);
            fastLedTime = micros() - startTime;
            
            // Measure PixelTheater fill_solid
            startTime = micros();
            PixelTheater::fill_solid(pt_array, static_cast<uint16_t>(MEDIUM_LED_COUNT), PixelTheater::CRGB(255, 0, 0));
            pixelTheaterTime = micros() - startTime;
            
            Serial.printf("FastLED fill_solid with %u LEDs: %lu microseconds\n", 
                         MEDIUM_LED_COUNT, fastLedTime);
            Serial.printf("PixelTheater fill_solid with %u LEDs: %lu microseconds\n", 
                         MEDIUM_LED_COUNT, pixelTheaterTime);
            Serial.printf("Ratio: %.2f\n", (float)pixelTheaterTime / fastLedTime);
            
            // Copy data between arrays to test conversion
            for (uint16_t i = 0; i < MEDIUM_LED_COUNT; i++) {
                // Copy from PixelTheater to FastLED
                fastled_array[i].r = pt_array[i].r;
                fastled_array[i].g = pt_array[i].g;
                fastled_array[i].b = pt_array[i].b;
            }
            
            // Verify the conversion worked
            CHECK(fastled_array[0].r == 255);
            CHECK(fastled_array[0].g == 0);
            CHECK(fastled_array[0].b == 0);
            
            Serial.println("PixelTheater operations with large arrays verified");
        } else {
            Serial.println("ERROR: Failed to allocate arrays for PixelTheater operations test");
        }
        
        // Clean up
        if (fastled_array) {
            delete[] fastled_array;
            fastled_array = nullptr;
        }
        if (pt_array) {
            delete[] pt_array;
            pt_array = nullptr;
        }
    }
    
    SUBCASE("Incremental Size Testing") {
        Serial.println("Testing incremental LED array sizes...");
        
        // Start with a small size and gradually increase
        uint16_t currentSize = 256;
        uint16_t maxTestedSize = 0;
        const uint16_t maxSizeToTest = 4096;  // Cap at 4096 LEDs
        const uint16_t sizeIncrement = 256;   // Increase by 256 LEDs each time
        bool testFailed = false;
        
        while (currentSize <= maxSizeToTest && !testFailed) {
            Serial.printf("Testing with %u LEDs...\n", currentSize);
            
            // Allocate arrays
            CRGB* fastled_array = new CRGB[currentSize];
            PixelTheater::CRGB* pt_array = new PixelTheater::CRGB[currentSize];
            
            if (fastled_array == nullptr || pt_array == nullptr) {
                Serial.printf("Allocation failed at %u LEDs\n", currentSize);
                testFailed = true;
            } else {
                // Test basic operations
                fill_solid(fastled_array, currentSize, CRGB::Red);
                PixelTheater::fill_solid(pt_array, static_cast<uint16_t>(currentSize), PixelTheater::CRGB(0, 255, 0));
                
                // Verify a few random elements
                bool verificationPassed = (fastled_array[0].r == 255 && pt_array[0].g == 255);
                
                if (verificationPassed) {
                    // Record successful size
                    maxTestedSize = currentSize;
                    Serial.printf("Successfully tested %u LEDs\n", currentSize);
                } else {
                    Serial.printf("Verification failed at %u LEDs\n", currentSize);
                    testFailed = true;
                }
            }
            
            // Clean up before next iteration
            if (fastled_array) {
                delete[] fastled_array;
                fastled_array = nullptr;
            }
            if (pt_array) {
                delete[] pt_array;
                pt_array = nullptr;
            }
            
            // Add a small delay to allow system to stabilize
            delay(10);
            
            // Increase size for next iteration
            currentSize += sizeIncrement;
        }
        
        Serial.printf("Maximum verified LED count: %u\n", maxTestedSize);
        CHECK(maxTestedSize >= 1024); // At minimum, we should handle 1024 LEDs
    }
    
    // Get final free memory
    finalFreeMemory = FreeMem();
    Serial.printf("Final free memory: %u bytes\n", finalFreeMemory);
    Serial.printf("Memory used during tests: %u bytes\n", initialFreeMemory - finalFreeMemory);
    
    Serial.println("Large LED array tests complete!");
} 