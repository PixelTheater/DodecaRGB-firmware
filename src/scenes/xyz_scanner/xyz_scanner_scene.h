#pragma once

#include "PixelTheater.h" // Use consolidated header
#include "benchmark.h"
#include <cmath>
#include <algorithm> // For std::max, std::min
#include <string> // For status method

// Add using directives
// REMOVED: using namespace PixelTheater; 
// REMOVED: using namespace PixelTheater::Constants; // Add for constants
// using PixelTheater::Scene; // Included via namespace
// using PixelTheater::CRGB; 
// using PixelTheater::map; 
// using PixelTheater::nblend;
// using PixelTheater::fadeToBlackBy;

namespace Scenes {

/**
 * XYZ Scanner Scene
 * 
 * This scene creates three scanning planes (red, green, and blue) that move through
 * the model along the X, Y, and Z axes. The planes create interesting visual effects
 * as they intersect and blend.
 */
class XYZScannerScene : public PixelTheater::Scene { // Inherit non-templated Scene
public:
    XYZScannerScene() = default; // Add default constructor
    
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
        // <<< ADDED METADATA >>>
        set_name("XYZ Scanner");
        set_description("Scans through the model along the X, Y, and Z axes with colorful planes of light");
        set_version("1.0");
        set_author("PixelTheater Team");
        // <<< END ADDED METADATA >>>
        
        // Define parameter ranges
        const float MIN_SPEED = 0.001f;
        const float MAX_SPEED = 5.0f;
        
        const int MIN_BLEND = 10;
        const int MAX_BLEND = 255;
        
        const int MIN_FADE = 1;
        const int MAX_FADE = 100;
        
        // Use base class param method
        param("speed", "range", MIN_SPEED, MAX_SPEED, DEFAULT_SPEED, "clamp", "Animation speed");
        param("blend", "count", MIN_BLEND, MAX_BLEND, DEFAULT_BLEND, "clamp", "Color blend amount");
        param("fade", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame");
        
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
        PixelTheater::Scene::tick();  
        
        float speed = settings["speed"];
        int blend = static_cast<int>(settings["blend"]);
        uint8_t fade_amount = static_cast<uint8_t>(settings["fade"]);
        
        size_t count = ledCount();
        for (size_t i = 0; i < count; ++i) {
            leds[i] = PixelTheater::CRGB(0, 0, 0); 
        }
        
        target = 100.0f + std::cos(counter / 700.0f) * 90.0f;
        target = std::clamp(target, 0.0f, 255.0f); 
        
        PixelTheater::CRGB c(0, 0, 0); // Declare qualified c outside the loop
        for (size_t i = 0; i < count; i++) {
            const auto& point = model().point(i); 
            
            float dz = (zi - point.z());
            if (std::abs(dz) < target) {
                float off = std::clamp(target - std::abs(dz), min_off, max_range); 
                c = PixelTheater::CRGB(0, 0, PixelTheater::map(off, min_off, target, 0.0f, 200.0f)); 
                PixelTheater::nblend(leds[i], c, blend); 
            }
            
            float dy = (yi - point.y());
            if (std::abs(dy) < target) {
                float off = std::clamp(target - std::abs(dy), min_off, max_range); 
                c = PixelTheater::CRGB(PixelTheater::map(off, min_off, target, 0.0f, 200.0f), 0, 0); 
                PixelTheater::nblend(leds[i], c, blend); 
            }
            
            float dx = (xi - point.x());
            if (std::abs(dx) < target) {
                float off = std::clamp(target - std::abs(dx), min_off, max_range); 
                c = PixelTheater::CRGB(0, PixelTheater::map(off, min_off, target, 0.0f, 200.0f), 0); 
                PixelTheater::nblend(leds[i], c, blend); 
            }
        }
        
        // Update positions (use std::clamp, std::tan)
        zi = (zi + speed * std::cos(counter / 2000.0f) * 2.0f);
        zi = std::clamp(zi, -max_range, max_range);
        if (std::abs(zi) >= max_range) zi = -zi * 0.99f; // Reverse slightly inside boundary
        
        yi = (yi + speed * std::clamp(std::tan(counter / 1600.0f) / 4.0f, -3.0f, 3.0f));
        yi = std::clamp(yi, -max_range, max_range);
        if (std::abs(yi) >= max_range) yi = -yi * 0.99f;
        
        xi = (xi + speed * std::sin(counter / 4000.0f) * 2.0f);
        xi = std::clamp(xi, -max_range, max_range);
        if (std::abs(xi) >= max_range) xi = -xi * 0.99f;
        
        // Apply fade using leds[] proxy
        for (size_t i = 0; i < count; ++i) {
            leds[i].fadeToBlackBy(fade_amount);
        }
        counter++;
    }
    
    std::string status() const {
        std::string output;
        // Ensure settings access is correctly handled here (might need PixelTheater:: qualification if not inherited?)
        float speed = settings["speed"];
        int blend = static_cast<int>(settings["blend"]);
        uint8_t fade = static_cast<uint8_t>(settings["fade"]);
        output += "XYZ Scanner: counter=" + std::to_string(counter) + "\n";
        output += "Positions: x=" + std::to_string(xi) + " y=" + std::to_string(yi) + " z=" + std::to_string(zi) + "\n";
        output += "Target: " + std::to_string(target) + " Speed: " + std::to_string(speed) + 
                  " Blend: " + std::to_string(blend) + " Fade: " + std::to_string(fade);
        return output;
    }
    
private:
    // No members needed here 
};

} // namespace Scenes