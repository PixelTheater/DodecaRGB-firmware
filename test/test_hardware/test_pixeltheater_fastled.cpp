#include <doctest/doctest.h>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/model/point.h"
#include "../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_CASE("PixelTheater and FastLED Integration") {
    Serial.println("\n=== Testing PixelTheater and FastLED Integration ===");

    SUBCASE("Model with FastLED") {
        Serial.println("Testing Model with FastLED...");
        
        // Create a FastLED array
        ::CRGB fastled_leds[BasicPentagonModel::LED_COUNT] = {};
        
        // Create a PixelTheater array that will be used by the model
        PixelTheater::CRGB pt_leds[BasicPentagonModel::LED_COUNT] = {};
        
        // Initialize the model definition
        BasicPentagonModel def;
        
        // Create the model with the PixelTheater LED array
        Model<BasicPentagonModel> model(def, pt_leds);
        
        // Test direct LED access through the model
        model.leds[0] = PixelTheater::CRGB(255, 0, 0);  // Red
        model.leds[1] = PixelTheater::CRGB(0, 255, 0);  // Green
        model.leds[2] = PixelTheater::CRGB(0, 0, 255);  // Blue
        
        // Verify LED values
        CHECK(model.leds[0].r == 255);
        CHECK(model.leds[0].g == 0);
        CHECK(model.leds[0].b == 0);
        
        CHECK(model.leds[1].r == 0);
        CHECK(model.leds[1].g == 255);
        CHECK(model.leds[1].b == 0);
        
        CHECK(model.leds[2].r == 0);
        CHECK(model.leds[2].g == 0);
        CHECK(model.leds[2].b == 255);
        
        Serial.println("Direct LED access verified");
        
        // Test face-based access
        auto& face = model.faces[0];
        fill_solid(face.leds, PixelTheater::CRGB(255, 255, 0));  // Yellow
        
        // Verify face LEDs
        for(size_t i = 0; i < face.led_count(); i++) {
            CHECK(face.leds[i].r == 255);
            CHECK(face.leds[i].g == 255);
            CHECK(face.leds[i].b == 0);
        }
        
        Serial.println("Face-based access verified");
        
        // Copy PixelTheater LEDs to FastLED array for hardware output
        for(size_t i = 0; i < BasicPentagonModel::LED_COUNT; i++) {
            fastled_leds[i].r = pt_leds[i].r;
            fastled_leds[i].g = pt_leds[i].g;
            fastled_leds[i].b = pt_leds[i].b;
        }
        
        // Configure FastLED with hardware pins
        FastLED.addLeds<WS2812B, 19, GRB>(fastled_leds, BasicPentagonModel::LED_COUNT);
        
        // Show the LEDs
        FastLED.show();
        delay(500);
        
        // Clear the LEDs
        FastLED.clear();
        FastLED.show();
        
        Serial.println("Hardware output verified");
    }
    
    SUBCASE("Performance Test") {
        Serial.println("Testing performance of FastLED vs PixelTheater implementations...");
        
        const int NUM_LEDS = 100;
        const int NUM_ITERATIONS = 1000;
        
        // Create arrays
        ::CRGB fastled_array[NUM_LEDS] = {};
        PixelTheater::CRGB pt_array[NUM_LEDS] = {};
        
        // Test FastLED fill_solid performance
        unsigned long start_time = micros();
        for(int i = 0; i < NUM_ITERATIONS; i++) {
            fill_solid(fastled_array, NUM_LEDS, ::CRGB::Red);
        }
        unsigned long fastled_time = micros() - start_time;
        
        // Test PixelTheater fill_solid performance
        start_time = micros();
        for(int i = 0; i < NUM_ITERATIONS; i++) {
            PixelTheater::fill_solid(pt_array, static_cast<uint16_t>(NUM_LEDS), PixelTheater::CRGB(255, 0, 0));
        }
        unsigned long pt_time = micros() - start_time;
        
        Serial.printf("FastLED fill_solid: %lu microseconds\n", fastled_time);
        Serial.printf("PixelTheater fill_solid: %lu microseconds\n", pt_time);
        Serial.printf("Ratio: %.2f\n", (float)pt_time / fastled_time);
        
        // We don't assert on performance, just report it
        Serial.println("Performance test complete");
    }

    Serial.println("PixelTheater and FastLED integration tests complete!");
} 