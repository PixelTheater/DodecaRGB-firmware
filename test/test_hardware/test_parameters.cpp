#include <doctest/doctest.h>
#include "PixelTheater/settings.h"
#include <Arduino.h>

using namespace PixelTheater;

TEST_CASE("Hardware Parameter System") {
    Serial.println("\n=== Testing Parameter System ===");
    
    SUBCASE("Memory Constraints") {
        Settings settings;
        
        // Test parameter storage limits
        for(int i = 0; i < 50; i++) {  // Reasonable number for hardware
            String param_name = "param";
            param_name += String(i);
            settings.add_parameter_from_strings(
                param_name.c_str(),  // Convert to C string
                "ratio", 
                ParamValue(0.5f), 
                "none"
            );
        }
        
        Serial.println("Parameter storage test complete");
    }
    
    SUBCASE("Real-time Behavior") {
        Settings settings;
        settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::CLAMP, ""));
        
        // Test rapid parameter updates
        uint32_t start = micros();
        for(int i = 0; i < 1000; i++) {
            settings.set_value("speed", ParamValue(float(i) / 1000.0f));
        }
        uint32_t duration = micros() - start;
        
        Serial.printf("1000 parameter updates took %d microseconds\n", duration);
    }
} 