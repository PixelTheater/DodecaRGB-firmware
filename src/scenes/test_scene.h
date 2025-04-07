#pragma once

#include "PixelTheater/scene.h"
#include "benchmark.h"
#include "PixelTheater/core/color.h"  // For color utilities
#include "models/DodecaRGBv2/model.h" // Correct path to model header

// External declaration of debug mode flag
extern bool g_debug_mode;

namespace Scenes {

// Define the concrete model type used by this scene
using ModelDef = PixelTheater::Models::DodecaRGBv2; // Correct model struct name

// Test Scene Implementation
// Inherit directly from the non-templated Scene base class
class TestScene : public PixelTheater::Scene {
public:
    // Constructor - use the base class constructor or default
    TestScene() : PixelTheater::Scene() {} // Call base constructor explicitly if needed

    // setup() remains override, base is virtual
    void setup() override {
        // Metadata setting remains the same using base class methods
        this->set_name("Test Scene");
        this->set_description("A simple test scene that cycles through colors");
        this->set_version("1.0");
        this->set_author("PixelTheater Team");

        // Parameter definition remains the same using base class protected methods
        this->param("speed", "ratio", 0.5f, "clamp");
        this->param("hue_shift", "ratio", 0.0f, "wrap"); 
        this->param("saturation", "ratio", 1.0f, "clamp");
        this->param("brightness", "ratio", 1.0f, "clamp");

        // Example log using base class helper
        std::stringstream ss;
        ss << "TestScene setup complete. Model has " 
           << model().faceCount() << " faces."; // Use model() from base class, faceCount() is correct
        logInfo(ss.str().c_str()); // Use base class logging
    }

    // tick() remains override, base is virtual
    void tick() override {
        // Call the base class tick method correctly
        PixelTheater::Scene::tick(); 

        // Get parameters using the settings proxy and get_value()
        float speed = settings["speed"];
        float hue_shift = (float)settings["hue_shift"] * 255.0f; // Cast needed if direct multiplication ambiguous
        float saturation = (float)settings["saturation"] * 255.0f;
        float brightness = (float)settings["brightness"] * 255.0f;

        // Calculate hue based on time (tick_count) and speed
        // Use tick_count() from base class
        uint8_t hue = (uint8_t)(((tick_count() * speed) / 10.0f) + hue_shift); 

        // Fill all LEDs with the calculated color
        // Use leds proxy from base class and loop explicitly
        // fill_solid(leds, leds.size(), CHSV(hue, (uint8_t)saturation, (uint8_t)brightness)); // Old way
        size_t count = leds.size();
        PixelTheater::CHSV hsv_color(hue, (uint8_t)saturation, (uint8_t)brightness);
        PixelTheater::CRGB rgb_color;
        hsv2rgb_rainbow(hsv_color, rgb_color); // Convert HSV to RGB once

        for(size_t i = 0; i < count; ++i) {
            leds[i] = rgb_color; // Assign the same RGB color to all LEDs
        }
    }

    // The Scene class might not have a status() method to override
    // Let's provide it without the override keyword
    std::string status() const {
        return "TestScene running";
    }
};

} // namespace Scenes 