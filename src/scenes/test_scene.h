#pragma once

#include "PixelTheater/scene.h"
#include "benchmark.h"
#include <FastLED.h>

namespace Scenes {

template<typename ModelDef>
class TestScene : public PixelTheater::Scene<ModelDef> {
public:
    using Scene = PixelTheater::Scene<ModelDef>;
    using Scene::Scene;  // Inherit constructor

    void setup() override {
        // Define parameters
        this->param("speed", "ratio", 0.5f, "clamp");
        this->param("hue_shift", "ratio", 0.0f, "wrap");
        this->param("saturation", "ratio", 1.0f, "clamp");
        this->param("brightness", "ratio", 1.0f, "clamp");
        
        // Reset benchmark data when scene is set up
        BENCHMARK_RESET();
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
        
        // Apply to all faces with different hue offsets
        BENCHMARK_START("update_leds");
        for (size_t face_idx = 0; face_idx < this->stage.model.face_count(); face_idx++) {
            auto& face = this->stage.model.faces[face_idx];
            uint8_t face_hue = hue_base + (face_idx * 21); // Distribute colors across faces
            
            for (size_t led_idx = 0; led_idx < face.led_count(); led_idx++) {
                // Create a gradient within each face
                uint8_t led_hue = face_hue + (led_idx * 2);
                // Use FastLED's hsv2rgb_rainbow to convert HSV to RGB
                ::CRGB fastled_color;
                hsv2rgb_rainbow(::CHSV(led_hue, saturation, brightness), fastled_color);
                // Copy RGB values to PixelTheater's CRGB
                face.leds[led_idx].r = fastled_color.r;
                face.leds[led_idx].g = fastled_color.g;
                face.leds[led_idx].b = fastled_color.b;
            }
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