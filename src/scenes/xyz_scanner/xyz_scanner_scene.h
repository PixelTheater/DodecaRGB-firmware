#pragma once

#include "PixelTheater/scene.h"
#include "PixelTheater/core/color.h"  // Include color.h for fadeToBlackBy
#include "PixelTheater/core/math.h"   // For cross-platform math functions
#include "PixelTheater/constants.h"   // For PI and TWO_PI constants
#include "benchmark.h"
#include <cmath>

// Include FastLED only for non-web platforms
#ifndef PLATFORM_WEB
#include <FastLED.h>
#endif

namespace Scenes {

// Get access to the math provider
extern PixelTheater::MathProvider& getMathProvider();

/**
 * XYZ Scanner Scene
 * 
 * This scene creates three scanning planes (red, green, and blue) that move through
 * the model along the X, Y, and Z axes. The planes create interesting visual effects
 * as they intersect and blend.
 */
template<typename ModelDef>
class XYZScannerScene : public PixelTheater::Scene<ModelDef> {
public:
    using Scene = PixelTheater::Scene<ModelDef>;
    using Scene::Scene;  // Inherit constructor
    
    // Default values - match original
    static constexpr float DEFAULT_SPEED = 3.0f;
    static constexpr int DEFAULT_BLEND = 130;
    static constexpr uint8_t DEFAULT_FADE = 3;
    static constexpr float DEFAULT_MAX_RANGE = 450.0f;
    
    // Scene state variables
    float max_range = DEFAULT_MAX_RANGE;
    float zi = -max_range;
    float yi = -max_range;
    float xi = -max_range;
    float target = 140.0f;
    int counter = 0;
    float min_off = 0.0f;
    
    void setup() override {
        // Set scene metadata
        this->set_name("XYZ Scanner");
        this->set_description("Scans through the model along the X, Y, and Z axes with colorful planes of light");
        this->set_version("1.0");
        this->set_author("PixelTheater Team");
        
        // Define parameter ranges
        const float MIN_SPEED = 0.001f;
        const float MAX_SPEED = 5.0f;
        
        const int MIN_BLEND = 10;
        const int MAX_BLEND = 255;
        
        const int MIN_FADE = 1;
        const int MAX_FADE = 100;
        
        // Define parameters
        this->param("speed", "range", MIN_SPEED, MAX_SPEED, DEFAULT_SPEED, "clamp", "Animation speed");
        this->param("blend", "count", MIN_BLEND, MAX_BLEND, DEFAULT_BLEND, "clamp", "Color blend amount");
        this->param("fade", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame");
        
        // Initialize state
        max_range = DEFAULT_MAX_RANGE;
        zi = -max_range;
        yi = -max_range;
        xi = -max_range;
        target = 140.0f;
        counter = 0;
        min_off = 0.0f;
    }
    
    void tick() override {
        Scene::tick();  
        
        // Get parameters
        float speed = this->settings["speed"];
        int blend = static_cast<int>(this->settings["blend"]);
        uint8_t fade_amount = static_cast<uint8_t>(this->settings["fade"]);
        
        // Clear LEDs (original used FastLED.clear())
        for (auto& led : this->stage.leds) {
            led = PixelTheater::CRGB(0, 0, 0);
        }
        
        // Calculate target value based on cosine function (original logic)
        target = 100.0f + std::cos(counter / 700.0f) * 90.0f;
        target = std::max(0.0f, std::min(target, 255.0f));  // constrain
        
        // Process each LED
        for (size_t i = 0; i < this->stage.model.led_count(); i++) {
            const auto& point = this->stage.model.points[i];
            PixelTheater::CRGB c(0, 0, 0);
            
            // Z animation
            float dz = (zi - point.z());
            if (std::abs(dz) < target) {
                float off = std::max(min_off, std::min(target - std::abs(dz), max_range));  // constrain
                c = PixelTheater::CRGB(0, 0, map(off, min_off, target, 0, 200));
                PixelTheater::nblend(this->stage.leds[i], c, blend);
            }
            
            // Y animation
            float dy = (yi - point.y());
            if (std::abs(dy) < target) {
                float off = std::max(min_off, std::min(target - std::abs(dy), max_range));  // constrain
                c = PixelTheater::CRGB(map(off, min_off, target, 0, 200), 0, 0);
                PixelTheater::nblend(this->stage.leds[i], c, blend);
            }
            
            // X animation
            float dx = (xi - point.x());
            if (std::abs(dx) < target) {
                float off = std::max(min_off, std::min(target - std::abs(dx), max_range));  // constrain
                c = PixelTheater::CRGB(0, map(off, min_off, target, 0, 200), 0);
                PixelTheater::nblend(this->stage.leds[i], c, blend);
            }
        }
        
        // Update positions (original logic)
        zi = (zi + speed * std::cos(counter / 2000.0f) * 2.0f);
        zi = std::max(-max_range, std::min(zi, max_range));  // constrain
        if (std::abs(zi) == max_range) zi = -zi;
        
        yi = (yi + speed * std::max(-3.0f, std::min(std::tan(counter / 1600.0f) / 4.0f, 3.0f)));  // constrain tan
        yi = std::max(-max_range, std::min(yi, max_range));  // constrain
        if (std::abs(yi) == max_range) yi = -yi;
        
        xi = (xi + speed * std::sin(counter / 4000.0f) * 2.0f);
        xi = std::max(-max_range, std::min(xi, max_range));  // constrain
        if (std::abs(xi) == max_range) xi = -xi;
        
        // Apply fade
        for (auto& led : this->stage.leds) {
            PixelTheater::fadeToBlackBy(led, fade_amount);
        }
        counter++;
    }
    
    std::string status() const {
        std::string output;
        
        // Get parameters
        float speed = this->settings["speed"];
        int blend = static_cast<int>(this->settings["blend"]);
        uint8_t fade = static_cast<uint8_t>(this->settings["fade"]);
        
        output += "XYZ Scanner: counter=" + std::to_string(counter) + "\n";
        output += "Positions: x=" + std::to_string(xi) + " y=" + std::to_string(yi) + " z=" + std::to_string(zi) + "\n";
        output += "Target: " + std::to_string(target) + " Speed: " + std::to_string(speed) + 
                  " Blend: " + std::to_string(blend) + " Fade: " + std::to_string(fade);
        
        return output;
    }
    
private:
    // Helper function to map a value from one range to another
    int map(float value, float fromLow, float fromHigh, int toLow, int toHigh) {
        float normalized = (value - fromLow) / (fromHigh - fromLow);
        return toLow + normalized * (toHigh - toLow);
    }
};

} // namespace Scenes