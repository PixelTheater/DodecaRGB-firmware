#pragma once

#include "PixelTheater/scene.h"
#include "PixelTheater/core/color.h"  // Include color.h for fadeToBlackBy
#include "PixelTheater/core/time.h"   // For cross-platform time functions
#include "PixelTheater/core/log.h"    // For cross-platform logging
#include "PixelTheater/core/math.h"   // For cross-platform math functions
#include "PixelTheater/constants.h"   // For PI and TWO_PI constants
#include "benchmark.h"
#include <vector>
#include <memory>
#include <cmath>

// Include FastLED only for non-web platforms
#ifndef PLATFORM_WEB
#include <FastLED.h>
#endif

namespace Scenes {

// Forward declaration
template<typename ModelDef>
class BlobScene;

// Get access to the math provider
extern PixelTheater::MathProvider& getMathProvider();

// Blob class - represents a single blob in the animation
template<typename ModelDef>
class Blob {
public:
    // The sphere radius will be determined dynamically
    int sphere_radius = 100;  // Default value, will be updated
    
    // Reference to the parent scene
    BlobScene<ModelDef>& scene;
    uint16_t blob_id = 0;
    int radius;      // radius of blob
    float a, c = 0;  // polar angles
    float av, cv;    // velocity of angles in radians
    float max_accel = 0.0;  // maximum acceleration
    
    int age;
    int lifespan;
    
    // Use PixelTheater's CRGB for cross-platform compatibility
    PixelTheater::CRGB color = PixelTheater::CRGB(255, 255, 255);
    
    Blob(BlobScene<ModelDef>& parent_scene, uint16_t unique_id, int min_r, int max_r, int max_a, float speed)
        : scene(parent_scene),
          blob_id(unique_id),
          min_radius(min_r),
          max_radius(max_r),
          max_age(max_a),
          speed_scale(speed) {
        // Estimate sphere radius from model
        estimateSphereRadius();
        reset();
    }
    
    void estimateSphereRadius() {
        // Find the maximum distance from origin to any point
        int max_dist = 0;
        for (size_t i = 0; i < scene.stage.model.led_count(); i++) {
            const auto& point = scene.stage.model.points[i];
            int dist_sq = point.x() * point.x() + point.y() * point.y() + point.z() * point.z();
            max_dist = std::max(max_dist, (int)sqrt(dist_sq));
        }
        
        // Set sphere radius to the maximum distance
        if (max_dist > 0) {
            sphere_radius = max_dist;
            PixelTheater::Log::warning("Estimated sphere radius: %d", sphere_radius);
        } else {
            PixelTheater::Log::warning("Could not estimate sphere radius, using default");
        }
    }
    
    void reset() {
        age = 0;
        lifespan = getMathProvider().random(max_age/2) + max_age/2;
        radius = getMathProvider().random(min_radius, max_radius);
        max_accel = getMathProvider().random(5,27)/1000.0 * speed_scale;  // Apply speed scaling
        av = getMathProvider().random(-max_accel * 1000, max_accel * 1000) / 1000.0;
        cv = getMathProvider().random(-max_accel * 1000, max_accel * 1000) / 1000.0;
        a = getMathProvider().random(PixelTheater::Constants::PT_TWO_PI*1000)/1000.0 - PixelTheater::Constants::PT_PI;
        c = getMathProvider().random(PixelTheater::Constants::PT_TWO_PI*10000)/10000.0 - PixelTheater::Constants::PT_PI;
    }
    
    int x() const { return sphere_radius * sin(c) * cos(a); }
    int y() const { return sphere_radius * sin(c) * sin(a); }
    int z() const { return sphere_radius * cos(c); }
    
    void applyForce(float af, float cf) {
        av += af;
        if (av > max_accel) av = max_accel;
        if (av < -max_accel) av = -max_accel;
        cv += cf;
        if (cv > max_accel) cv = max_accel;
        if (cv < -max_accel) cv = -max_accel;
    }
    
    void applyForce(float fx, float fy, float fz) {
        float af = atan2(fy, fx);
        float cf = atan2(sqrt(fx*fx + fy*fy), fz);
        applyForce(af, cf);
    }
    
    void tick() {
        float force_av = av * 1.001;
        c = fmod(c + PixelTheater::Constants::PT_PI, PixelTheater::Constants::PT_TWO_PI) - PixelTheater::Constants::PT_PI;  // Normalize c to be within [-PI, PI]
        float force_cv = 0.00035 * (c - PixelTheater::Constants::PT_PI/2);
        if (c < -PixelTheater::Constants::PT_PI/2) {
            force_cv = -0.0003 * (c + PixelTheater::Constants::PT_PI/2);
        }
        applyForce(force_av, force_cv);
        
        age++;
        av *= 0.99; 
        cv *= 0.99; 
        a += av; 
        c += cv;
        
        if (abs(cv) < 0.001) {
            float af = getMathProvider().random(-max_accel*1000, max_accel*1000);
            float cf = getMathProvider().random(-max_accel*1000, max_accel*1000);
            applyForce(af/2000.0, cf/1000.0);
        }
        
        if (lifespan - age < max_age/20) {
            radius *= 0.99;
        }
        if (age++ > lifespan) {
            reset();
        }
    }
    
private:
    int min_radius;
    int max_radius;
    int max_age;
    float speed_scale;
};

// BlobScene class - manages multiple blobs and renders them
template<typename ModelDef>
class BlobScene : public PixelTheater::Scene<ModelDef> {
public:
    using Scene = PixelTheater::Scene<ModelDef>;
    using Scene::Scene;  // Inherit constructor
    
    // Default values
    static constexpr int DEFAULT_NUM_BLOBS = 8;
    static constexpr int DEFAULT_MIN_RADIUS = 80;
    static constexpr int DEFAULT_MAX_RADIUS = 130;
    static constexpr int DEFAULT_MAX_AGE = 4000;
    static constexpr float DEFAULT_SPEED = 1.2;
    static constexpr uint8_t DEFAULT_FADE = 2;
    
    void setup() override {
        // Define parameters with the correct types and required min/max values
        // For count/integer parameters, use param method with name, type, min, max, default format
        
        // Define ranges for parameters
        const int MIN_BLOBS = 1;
        const int MAX_BLOBS = 20;
        
        const int MIN_RADIUS_LOW = 10;
        const int MIN_RADIUS_HIGH = 100;
        
        const int MAX_RADIUS_LOW = 50;
        const int MAX_RADIUS_HIGH = 200;
        
        const int MIN_AGE = 500;
        const int MAX_AGE = 10000;
        
        const int MIN_FADE = 1;
        const int MAX_FADE = 20;
        
        // Now define parameters with proper ranges using the param() method
        this->param("num_blobs", "count", MIN_BLOBS, MAX_BLOBS, DEFAULT_NUM_BLOBS, "clamp", "Number of blobs");
        this->param("min_radius", "count", MIN_RADIUS_LOW, MIN_RADIUS_HIGH, DEFAULT_MIN_RADIUS, "clamp", "Minimum blob radius");
        this->param("max_radius", "count", MAX_RADIUS_LOW, MAX_RADIUS_HIGH, DEFAULT_MAX_RADIUS, "clamp", "Maximum blob radius");
        this->param("max_age", "count", MIN_AGE, MAX_AGE, DEFAULT_MAX_AGE, "clamp", "Maximum blob lifetime");
        
        // For float between 0-1, use "ratio" type (doesn't need min/max as it's fixed to 0-1)
        this->param("speed", "ratio", DEFAULT_SPEED, "clamp", "Animation speed");
        
        // For the fade parameter (which is a small integer), use count type with range
        this->param("fade", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame");
        
        // Debug output for parameters
        PixelTheater::Log::warning("Parameters defined with ranges:");
        PixelTheater::Log::warning("  num_blobs: %d-%d (default: %d)", MIN_BLOBS, MAX_BLOBS, DEFAULT_NUM_BLOBS);
        PixelTheater::Log::warning("  min_radius: %d-%d (default: %d)", MIN_RADIUS_LOW, MIN_RADIUS_HIGH, DEFAULT_MIN_RADIUS);
        PixelTheater::Log::warning("  max_radius: %d-%d (default: %d)", MAX_RADIUS_LOW, MAX_RADIUS_HIGH, DEFAULT_MAX_RADIUS);
        PixelTheater::Log::warning("  max_age: %d-%d (default: %d)", MIN_AGE, MAX_AGE, DEFAULT_MAX_AGE);
        PixelTheater::Log::warning("  speed: 0.0-1.0 (default: %.2f)", DEFAULT_SPEED);
        PixelTheater::Log::warning("  fade: %d-%d (default: %d)", MIN_FADE, MAX_FADE, DEFAULT_FADE);
        
        // Reset benchmark data when scene is set up
        BENCHMARK_RESET();
        
        // Initialize blobs
        initBlobs();
        
        PixelTheater::Log::warning("Setup complete, test pattern applied");
    }
    
    void initBlobs() {
        // Get parameters with proper access via settings
        int num_blobs = this->settings["num_blobs"];
        int min_radius = this->settings["min_radius"];
        int max_radius = this->settings["max_radius"];
        int max_age = this->settings["max_age"];
        float speed = this->settings["speed"];
        
        // Debug output
        PixelTheater::Log::warning("Creating %d blobs with radius %d-%d, max_age %d, speed %.2f", 
                   num_blobs, min_radius, max_radius, max_age, speed);
        
        // Create blobs with unique random colors
        blobs.clear();
        for (int i = 0; i < num_blobs; i++) {
            auto blob = std::make_unique<Blob<ModelDef>>(*this, i, min_radius, max_radius, max_age, speed);
            
            // Assign a random vibrant color to each blob
            PixelTheater::CHSV hsv(getMathProvider().random8(), 255, 255);  // Random hue, full saturation and brightness
            PixelTheater::hsv2rgb_rainbow(hsv, blob->color);
            
            blobs.push_back(std::move(blob));
        }
        
        PixelTheater::Log::warning("Created %d blobs", blobs.size());
        
        // If no blobs were created, create some with hardcoded values
        if (blobs.empty()) {
            PixelTheater::Log::warning("No blobs created from parameters, using hardcoded values");
            for (int i = 0; i < 5; i++) {
                auto blob = std::make_unique<Blob<ModelDef>>(*this, i, 50, 80, 4000, 1.0f);
                
                // Create evenly spaced colors
                PixelTheater::CHSV hsv(i * 50, 255, 255);
                PixelTheater::hsv2rgb_rainbow(hsv, blob->color);
                
                blobs.push_back(std::move(blob));
            }
            PixelTheater::Log::warning("Created %d hardcoded blobs", blobs.size());
        }
    }
    
    void tick() override {
        // Start overall scene benchmark
        BENCHMARK_START("scene_total");
        
        Scene::tick();  // Call base to increment counter
        
        // Get fade parameter from settings
        BENCHMARK_START("get_parameters");
        uint8_t fade_amount = static_cast<uint8_t>(this->settings["fade"]);
        
        // Debug the fade parameter occasionally
        static uint32_t last_param_debug = 0;
        uint32_t current_time = PixelTheater::getSystemTimeProvider().millis();
        if (current_time - last_param_debug > 5000) {
            PixelTheater::Log::warning("Fade parameter: %d", fade_amount);
            last_param_debug = current_time;
        }
        BENCHMARK_END();
        
        // Update blobs
        BENCHMARK_START("update_blobs");
        updateBlobs();
        BENCHMARK_END();
        
        // Draw blobs
        BENCHMARK_START("draw_blobs");
        drawBlobs();
        BENCHMARK_END();
        
        // Apply fade to all LEDs
        BENCHMARK_START("fade_leds");
        for (auto& led : this->stage.leds) {
            // Use PixelTheater's fadeToBlackBy function properly
            PixelTheater::fadeToBlackBy(led, fade_amount);
        }
        BENCHMARK_END();
        
        // Debug output - print first blob position and check if it's within range of any LEDs
        static uint32_t last_debug_pos = 0;
        if (current_time - last_debug_pos > 5000 && !blobs.empty()) {
            auto& blob = blobs[0];
            PixelTheater::Log::warning("Blob 0 position: (%d, %d, %d), radius: %d", 
                       blob->x(), blob->y(), blob->z(), blob->radius);
            
            // Check if the blob is within range of any LEDs
            bool found_led = false;
            for (size_t i = 0; i < 10; i++) {  // Just check first 10 LEDs for debug
                const auto& point = this->stage.model.points[i];
                int dx = point.x() - blob->x();
                int dy = point.y() - blob->y();
                int dz = point.z() - blob->z();
                int dist = dx*dx + dy*dy + dz*dz;
                
                if (dist < blob->radius * blob->radius) {
                    PixelTheater::Log::warning("  Blob is within range of LED %d at (%d, %d, %d), dist: %d", 
                               i, point.x(), point.y(), point.z(), (int)sqrt(dist));
                    found_led = true;
                    break;
                }
            }
            
            if (!found_led) {
                PixelTheater::Log::warning("  Blob is not within range of first 10 LEDs");
            }
            
            last_debug_pos = current_time;
        }
        
        // End overall scene benchmark
        BENCHMARK_END();
    }
    
    void updateBlobs() {
        static const float forceStrength = 0.000005;  // Tuning variable for repelling force
        
        // Debug output - only print occasionally to avoid flooding
        static uint32_t last_debug = 0;
        uint32_t current_time = PixelTheater::getSystemTimeProvider().millis();
        if (current_time - last_debug > 5000) {
            PixelTheater::Log::warning("Updating %d blobs", blobs.size());
            last_debug = current_time;
        }
        
        // Update all blobs
        for (auto& blob : blobs) {
            blob->tick();
        }
        
        // Apply repelling forces between blobs
        for (size_t i = 0; i < blobs.size(); i++) {
            for (size_t j = i + 1; j < blobs.size(); j++) {
                float min_dist = (blobs[i]->radius + blobs[j]->radius) / 2;
                float min_dist_sq = min_dist * min_dist;
                
                float dx = blobs[i]->x() - blobs[j]->x();
                float dy = blobs[i]->y() - blobs[j]->y();
                float dz = blobs[i]->z() - blobs[j]->z();
                float dist_sq = dx*dx + dy*dy + dz*dz;
                
                if (dist_sq < min_dist_sq && dist_sq > 20) {
                    float dist = sqrt(dist_sq);
                    float force = ((min_dist - dist) / min_dist) * forceStrength;
                    
                    // Normalize direction vector
                    float nx = dx / dist;
                    float ny = dy / dist;
                    float nz = dz / dist;
                    
                    // Apply repelling forces
                    blobs[i]->applyForce(nx * force, ny * force, nz * force);
                    blobs[j]->applyForce(-nx * force, -ny * force, -nz * force);
                }
            }
        }
    }
    
    void drawBlobs() {
        // Debug output - only print occasionally to avoid flooding
        static uint32_t last_debug = 0;
        uint32_t current_time = PixelTheater::getSystemTimeProvider().millis();
        if (current_time - last_debug > 5000) {
            PixelTheater::Log::warning("Drawing %d blobs, model has %d faces", 
                       blobs.size(), this->stage.model.face_count());
            last_debug = current_time;
        }
        
        // Draw each blob
        for (auto& blob : blobs) {
            auto rad_sq = blob->radius * blob->radius;
            
            // For each face in the model
            for (size_t face_idx = 0; face_idx < this->stage.model.face_count(); face_idx++) {
                auto& face = this->stage.model.faces[face_idx];
                
                // For each LED in the face
                for (size_t led_idx = 0; led_idx < face.led_count(); led_idx++) {
                    // Get the global LED index for this face's LED
                    size_t global_led_idx = face.led_offset() + led_idx;
                    
                    // Get the LED's position from the model's points collection
                    const auto& point = this->stage.model.points[global_led_idx];
                    
                    // Calculate distance to blob using point coordinates
                    int dx = point.x() - blob->x();
                    int dy = point.y() - blob->y();
                    int dz = point.z() - blob->z();
                    int dist = dx*dx + dy*dy + dz*dz;
                    
                    // If LED is within blob radius, color it
                    if (dist < rad_sq) {
                        // Use blob's color directly (already PixelTheater::CRGB)
                        PixelTheater::CRGB c = blob->color;
                        
                        // Apply fade-in effect for new blobs
                        if (blob->age < 150) {
                            // Calculate fade amount based on age
                            uint8_t fade_amount = PixelTheater::map(blob->age, 0, 150, 180, 1);
                            // Use PixelTheater's fadeToBlackBy for the fade-in effect
                            PixelTheater::fadeToBlackBy(c, fade_amount);
                        }
                        
                        // Blend color based on distance (closer = brighter)
                        uint8_t blend_amount = PixelTheater::map(dist, 0, rad_sq, 30, 7);
                        
                        // Get reference to the LED in the face
                        PixelTheater::CRGB& led = face.leds[led_idx];
                        
                        // Blend colors using PixelTheater's blend8 function
                        led.r = PixelTheater::blend8(led.r, c.r, blend_amount);
                        led.g = PixelTheater::blend8(led.g, c.g, blend_amount);
                        led.b = PixelTheater::blend8(led.b, c.b, blend_amount);
                    }
                }
            }
        }
    }
    
    std::string status() const {
        std::string output;
        
        // Get parameters directly from settings
        float speed = this->settings["speed"];
        int fade = this->settings["fade"];
        int min_radius = this->settings["min_radius"];
        int max_radius = this->settings["max_radius"];
        int max_age = this->settings["max_age"];
        
        output += "Blobs: " + std::to_string(blobs.size()) + " active (speed=" + 
                  std::to_string(speed) + 
                  ", fade=" + std::to_string(fade) + ")\n";
        
        output += "Radius: " + std::to_string(min_radius) + 
                  "-" + std::to_string(max_radius) + 
                  ", MaxAge: " + std::to_string(max_age) + "\n";
        
        // Show info for first 3 blobs to avoid cluttering the display
        for (size_t i = 0; i < std::min(size_t(3), blobs.size()); i++) {
            const auto& blob = blobs[i];
            output += "Blob " + std::to_string(blob->blob_id) + 
                      ": age=" + std::to_string(blob->age) + "/" + std::to_string(blob->lifespan) + 
                      " accel=" + std::to_string(blob->av) + "/" + std::to_string(blob->cv) + "\n";
        }
        
        return output;
    }
    
private:
    std::vector<std::unique_ptr<Blob<ModelDef>>> blobs;
};

} // namespace Scenes 