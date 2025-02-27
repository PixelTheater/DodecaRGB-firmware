#include <doctest/doctest.h>
#include "PixelTheater/settings.h"
#include <Arduino.h>

using namespace PixelTheater;

// Memory checking functions for Teensy
extern unsigned long _heap_start;
extern unsigned long _heap_end;
extern char *__brkval;

uint32_t freeRAMTeensy() {
    return (char *)&_heap_end - __brkval;
}

uint32_t freeStackTeensy() {
    extern char _ebss[];
    auto sp = (char*) __builtin_frame_address(0);
    const uint32_t stack = sp - _ebss;
    return stack;
}

TEST_CASE("Hardware Parameter System") {
    Serial.println("\n=== Testing Parameter System ===");
    
    SUBCASE("Real-time Performance") {
        Settings settings;
        settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::CLAMP, "Speed parameter"));
        
        // Test rapid parameter updates
        uint32_t start = micros();
        for(int i = 0; i < 1000; i++) {
            float value = float(i) / 1000.0f;  // 0.0 to 0.999
            settings.set_value("speed", ParamValue(value));
        }
        uint32_t duration = micros() - start;
        
        // Verify final value and timing
        float final_speed = settings.get_value("speed").as_float();
        CHECK(final_speed == doctest::Approx(0.999f));
        
        Serial.printf("1000 parameter updates took %d microseconds\n", duration);
        CHECK(duration < 2000);  // Allow ~2µs per update with safety margin
    }
    
    SUBCASE("Parameter Validation") {
        Settings settings;
        
        // Test parameter creation and access
        settings.add_parameter(PARAM_RATIO("brightness", 0.5f, Flags::CLAMP, "Brightness control"));
        CHECK(settings.has_parameter("brightness"));
        CHECK(settings.get_value("brightness").as_float() == doctest::Approx(0.5f));
        
        // Test clamping behavior
        settings.set_value("brightness", ParamValue(1.5f));  // Should clamp to 1.0
        CHECK(settings.get_value("brightness").as_float() == doctest::Approx(1.0f));
        
        // Test wrapping behavior
        settings.add_parameter(PARAM_RATIO("wrapped", 0.5f, Flags::WRAP, "Wrapped value"));
        settings.set_value("wrapped", ParamValue(1.5f));  // Should wrap to 0.5
        CHECK(settings.get_value("wrapped").as_float() == doctest::Approx(0.5f));
        
        // Test invalid parameter access
        CHECK_FALSE(settings.has_parameter("nonexistent"));
        
        Serial.println("Parameter validation complete");
    }
    
    SUBCASE("Parameter Stress Test") {
        Settings settings;
        const int NUM_PARAMS = 100;  // Test with large number of parameters
        
        // Add many parameters
        for(int i = 0; i < NUM_PARAMS; i++) {
            String name = "param" + String(i);
            settings.add_parameter(PARAM_RATIO(
                name.c_str(), 
                0.5f, 
                Flags::CLAMP, 
                "Test parameter"
            ));
        }
        
        // Verify all parameters are accessible
        bool all_accessible = true;
        for(int i = 0; i < NUM_PARAMS; i++) {
            String name = "param" + String(i);
            if(!settings.has_parameter(name.c_str()) || 
               settings.get_value(name.c_str()).as_float() != 0.5f) {
                all_accessible = false;
                break;
            }
        }
        
        CHECK(all_accessible);
        Serial.printf("Successfully created and verified %d parameters\n", NUM_PARAMS);
    }
    
    SUBCASE("Range Parameter Creation") {
        // Create a settings object
        Settings settings;
        
        // Add a count parameter with min/max range
        settings.add_parameter(PARAM_COUNT("particles", 10, 100, 50, Flags::CLAMP, "Number of particles"));
        
        // Verify parameter was created correctly
        CHECK(settings.has_parameter("particles"));
        CHECK(settings.get_value("particles").as_int() == 50);
        
        // Test min/max validation
        settings.set_value("particles", ParamValue(5));  // Below min
        CHECK(settings.get_value("particles").as_int() == 10);  // Should clamp to min
        
        settings.set_value("particles", ParamValue(150));  // Above max
        CHECK(settings.get_value("particles").as_int() == 100);  // Should clamp to max
        
        settings.set_value("particles", ParamValue(75));  // Within range
        CHECK(settings.get_value("particles").as_int() == 75);  // Should use provided value
        
        Serial.println("Range parameter validation complete");
    }
} 