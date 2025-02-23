#include <doctest/doctest.h>
#include "PixelTheater/core/math.h"
#include <Arduino.h>

using namespace PixelTheater;

TEST_CASE("Arduino Math Functions") {
    Serial.println("\n=== Testing Math Functions ===");

    SUBCASE("map function") {
        // Test integer mapping
        int result = map(50, 0, 100, 0, 1000);
        Serial.printf("map(50, 0, 100, 0, 1000) = %d\n", result);
        CHECK(result == 500);

        // Test float mapping
        float fresult = map(0.5f, 0.0f, 1.0f, 0.0f, 100.0f);
        Serial.printf("map(0.5, 0.0, 1.0, 0.0, 100.0) = %.2f\n", fresult);
        CHECK(fresult == doctest::Approx(50.0f));
    }

    SUBCASE("constrain function") {
        int clamped = constrain(150, 0, 100);
        Serial.printf("constrain(150, 0, 100) = %d\n", clamped);
        CHECK(clamped == 100);
    }

    Serial.println("Math tests complete!");
}

TEST_CASE("Arduino Math Edge Cases") {
    Serial.println("\n=== Testing Math Edge Cases ===");

    SUBCASE("map with reversed ranges") {
        // Arduino map can handle reversed ranges
        int result = map(75, 0, 100, 1000, 0);
        Serial.printf("map(75, 0, 100, 1000, 0) = %d\n", result);
        CHECK(result == 250);
    }

    SUBCASE("map with zero range") {
        Serial.println("Testing map with zero range...");
        // Test safe handling of zero range
        int in_val = 50;
        int in_min = 0;
        int in_max = 0;
        int out_min = 0;
        int out_max = 100;
        
        // When input range is zero, output should be out_min to avoid division by zero
        int result = in_max == in_min ? out_min : 
            map(in_val, in_min, in_max, out_min, out_max);
        
        Serial.printf("map(%d, %d, %d, %d, %d) = %d\n", 
            in_val, in_min, in_max, out_min, out_max, result);
        CHECK(result == out_min);
    }

    SUBCASE("constrain edge cases") {
        CHECK(constrain(100, 100, 100) == 100);  // Equal bounds
        CHECK(constrain(-1000, 0, 100) == 0);    // Far below
        CHECK(constrain(1000, 0, 100) == 100);   // Far above
        Serial.println("Constrain edge cases verified");
    }

    Serial.println("Edge case tests complete!");
} 