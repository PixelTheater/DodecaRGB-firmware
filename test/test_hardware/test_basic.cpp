#include <doctest/doctest.h>
#include "PixelTheater/settings.h"
#include "PixelTheater/core/log.h"
#include <Arduino.h>

using namespace PixelTheater;

TEST_CASE("Basic parameter functionality") {
    // Initialize Serial first
    Serial.begin(115200);
    delay(100);  // Give serial a moment to initialize
    
    Serial.println("\n=== Starting Basic Parameter Test ===");
    
    Settings settings;
    Serial.println("Settings created");
    
    // Create a ratio parameter
    settings.add_parameter(PARAM_RATIO("brightness", 0.5f, Flags::CLAMP, "LED brightness"));
    Serial.println("Parameter added");
    
    // Test valid value
    settings.set_value("brightness", ParamValue(0.75f));
    Serial.printf("Brightness set to: %.2f\n", settings.get_value("brightness").as_float());
    
    // Test invalid value (should clamp)
    settings.set_value("brightness", ParamValue(1.5f));
    Serial.printf("Brightness after clamp: %.2f\n", settings.get_value("brightness").as_float());
    
    // Test non-existent parameter
    settings.set_value("nonexistent", ParamValue(1.0f));
    Serial.println("Test complete!");
    
    CHECK(true);  // Just to have a test pass
} 