#pragma once

#include "PixelTheater/scene.h"
#include "benchmark.h"
#include "PixelTheater/core/color.h"  // For color utilities

// External declaration of debug mode flag
extern bool g_debug_mode;

namespace Scenes {

template<typename ModelDef>
class TestScene : public PixelTheater::Scene<ModelDef> {
public:
    using Scene = PixelTheater::Scene<ModelDef>;
    using Scene::Scene;  // Inherit constructor

    void setup() override {
        // Define parameters with valid default values
        this->param("speed", "ratio", 0.5f, "clamp");
        this->param("hue_shift", "ratio", 0.0f, "wrap");  // Make sure this is a valid value
        this->param("saturation", "ratio", 1.0f, "clamp");
        this->param("brightness", "ratio", 1.0f, "clamp");
        
        // Reset benchmark data when scene is set up
        BENCHMARK_RESET();
        
        // Print debug info
        std::cout << "TestScene setup complete. Model has " 
                  << this->stage.model.face_count() << " faces." << std::endl;
    }

    void tick() override {
        // Start overall scene benchmark
        BENCHMARK_START("scene_total");
        
        Scene::tick();  // Call base to increment counter

        // Get parameters - explicitly cast to float before multiplication
        BENCHMARK_START("get_parameters");
        float speed = static_cast<float>(this->settings["speed"]);
        float hue_shift = static_cast<float>(this->settings["hue_shift"]) * 255.0f;
        float saturation = static_cast<float>(this->settings["saturation"]) * 255.0f;
        float brightness = static_cast<float>(this->settings["brightness"]) * 255.0f;
        BENCHMARK_END();

        // Create a simple rainbow animation
        uint8_t hue_base = (this->tick_count() * speed) + hue_shift;
        
        // Debug output every 300 frames (5 seconds at 60fps) when debug mode is on
        if (this->tick_count() % 300 == 0 && g_debug_mode) {
            std::cout << "TestScene tick: " << this->tick_count() 
                      << ", hue_base: " << (int)hue_base 
                      << ", speed: " << speed 
                      << ", saturation: " << saturation / 255.0f
                      << ", brightness: " << brightness / 255.0f
                      << std::endl;
        }
        
        // Apply to all faces with different hue offsets
        BENCHMARK_START("update_leds");
        
        // Add null check for model - use size check instead of empty()
        if (this->stage.model.face_count() > 0) {
            size_t face_count = this->stage.model.face_count();
            
            for (size_t face_idx = 0; face_idx < face_count; face_idx++) {
                // Add bounds checking
                if (face_idx >= this->stage.model.faces.size()) {
                    continue;  // Skip this face if it's out of bounds
                }
                
                auto& face = this->stage.model.faces[face_idx];
                uint8_t face_hue = hue_base + (face_idx * 21); // Distribute colors across faces
                
                size_t led_count = face.led_count();
                for (size_t led_idx = 0; led_idx < led_count; led_idx++) {
                    // Create a gradient within each face
                    uint8_t led_hue = face_hue + (led_idx * 2);
                    
                    // Create HSV color and convert to RGB using existing function
                    PixelTheater::CHSV hsv(led_hue, saturation, brightness);
                    
                    // Add bounds checking for LED access
                    if (led_idx < led_count) {
                        PixelTheater::CRGB& rgb = face.leds[led_idx];
                        
                        // Use the existing hsv2rgb_rainbow function
                        PixelTheater::hsv2rgb_rainbow(hsv, rgb);
                        
                        // Ensure minimum brightness for visibility
                        rgb.r = std::max(rgb.r, (uint8_t)50);
                        rgb.g = std::max(rgb.g, (uint8_t)50);
                        rgb.b = std::max(rgb.b, (uint8_t)50);
                    }
                }
            }
        } else {
            std::cerr << "Error: Model has no faces!" << std::endl;
        }
        
        BENCHMARK_END();
        
        // End overall scene benchmark
        BENCHMARK_END();
    }

    // The Scene class might not have a status() method to override
    // Let's provide it without the override keyword
    std::string status() const {
        return "TestScene running";
    }
};

} // namespace Scenes 