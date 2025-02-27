#pragma once

#include "PixelTheater/scene.h"
#include "PixelTheater/core/color.h"  // Include color.h for fadeToBlackBy
#include "benchmark.h"
#include <FastLED.h>
#include <vector>
#include <memory>

namespace Scenes {

// Forward declaration
template<typename ModelDef>
class BlobScene;

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
    
    CRGB color = CRGB::White;
    
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
            max_dist = max(max_dist, (int)sqrt(dist_sq));
        }
        
        // Set sphere radius to the maximum distance
        if (max_dist > 0) {
            sphere_radius = max_dist;
            Serial.printf("Estimated sphere radius: %d\n", sphere_radius);
        } else {
            Serial.println("Could not estimate sphere radius, using default");
        }
    }
    
    void reset() {
        age = 0;
        lifespan = random(max_age/2) + max_age/2;
        radius = random(min_radius, max_radius);
        max_accel = random(5,27)/1000.0 * speed_scale;  // Apply speed scaling
        av = random(-max_accel * 1000, max_accel * 1000) / 1000.0;
        cv = random(-max_accel * 1000, max_accel * 1000) / 1000.0;
        a = random(TWO_PI*1000)/1000.0 - PI;
        c = random(TWO_PI*10000)/10000.0 - PI;
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
        c = fmod(c + PI, TWO_PI) - PI;  // Normalize c to be within [-PI, PI]
        float force_cv = 0.00035 * (c - PI/2);
        if (c < -PI/2) {
            force_cv = -0.0003 * (c + PI/2);
        }
        applyForce(force_av, force_cv);
        
        age++;
        av *= 0.99; 
        cv *= 0.99; 
        a += av; 
        c += cv;
        
        if (abs(cv) < 0.001) {
            float af = random(-max_accel*1000, max_accel*1000);
            float cf = random(-max_accel*1000, max_accel*1000);
            applyForce(af/2000.0, cf/1000.0);
        }
        
        if (lifespan - age < max_age/20) {
            radius *= 0.99;
        }
        if (age > lifespan) {
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
    static constexpr uint8_t DEFAULT_FADE = 3;
    
    void setup() override {
        // Define parameters with the correct types and required min/max values
        // For count/integer parameters, use add_count_parameter with min, max, default format
        
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
        
        // Now define parameters with proper ranges using direct methods
        this->settings.add_count_parameter("num_blobs", MIN_BLOBS, MAX_BLOBS, DEFAULT_NUM_BLOBS, "clamp");
        this->settings.add_count_parameter("min_radius", MIN_RADIUS_LOW, MIN_RADIUS_HIGH, DEFAULT_MIN_RADIUS, "clamp");
        this->settings.add_count_parameter("max_radius", MAX_RADIUS_LOW, MAX_RADIUS_HIGH, DEFAULT_MAX_RADIUS, "clamp");
        this->settings.add_count_parameter("max_age", MIN_AGE, MAX_AGE, DEFAULT_MAX_AGE, "clamp");
        
        // For float between 0-1, use "ratio" type (doesn't need min/max as it's fixed to 0-1)
        this->settings.add_parameter_from_strings("speed", "ratio", DEFAULT_SPEED, "clamp");
        
        // For the fade parameter (which is a small integer), use count type with range
        this->settings.add_count_parameter("fade", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp");
        
        // Debug output for parameters
        Serial.printf("Parameters defined with ranges:\n");
        Serial.printf("  num_blobs: %d-%d (default: %d)\n", MIN_BLOBS, MAX_BLOBS, DEFAULT_NUM_BLOBS);
        Serial.printf("  min_radius: %d-%d (default: %d)\n", MIN_RADIUS_LOW, MIN_RADIUS_HIGH, DEFAULT_MIN_RADIUS);
        Serial.printf("  max_radius: %d-%d (default: %d)\n", MAX_RADIUS_LOW, MAX_RADIUS_HIGH, DEFAULT_MAX_RADIUS);
        Serial.printf("  max_age: %d-%d (default: %d)\n", MIN_AGE, MAX_AGE, DEFAULT_MAX_AGE);
        Serial.printf("  speed: 0.0-1.0 (default: %.2f)\n", DEFAULT_SPEED);
        Serial.printf("  fade: %d-%d (default: %d)\n", MIN_FADE, MAX_FADE, DEFAULT_FADE);
        
        // Reset benchmark data when scene is set up
        BENCHMARK_RESET();
        
        // Initialize blobs
        initBlobs();
        
        // Test pattern - set first 10 LEDs to bright colors
        for (size_t i = 0; i < 10 && i < this->stage.model.led_count(); i++) {
            this->stage.leds[i].r = 255;
            this->stage.leds[i].g = 0;
            this->stage.leds[i].b = 0;
        }
        
        Serial.println("Setup complete, test pattern applied");
    }
    
    void initBlobs() {
        // Get parameters with proper access via settings
        int num_blobs = this->settings["num_blobs"];
        int min_radius = this->settings["min_radius"];
        int max_radius = this->settings["max_radius"];
        int max_age = this->settings["max_age"];
        float speed = this->settings["speed"];
        
        // Debug output
        Serial.printf("Creating %d blobs with radius %d-%d, max_age %d, speed %.2f\n", 
                     num_blobs, min_radius, max_radius, max_age, speed);
        
        // Create blobs with unique random colors
        blobs.clear();
        for (int i = 0; i < num_blobs; i++) {
            auto blob = std::make_unique<Blob<ModelDef>>(*this, i, min_radius, max_radius, max_age, speed);
            
            // Assign a random vibrant color to each blob
            blob->color = CHSV(random(256), 255, 255);  // Random hue, full saturation and brightness
            
            blobs.push_back(std::move(blob));
        }
        
        Serial.printf("Created %d blobs\n", blobs.size());
        
        // If no blobs were created, create some with hardcoded values
        if (blobs.empty()) {
            Serial.println("No blobs created from parameters, using hardcoded values");
            for (int i = 0; i < 5; i++) {
                auto blob = std::make_unique<Blob<ModelDef>>(*this, i, 50, 80, 4000, 1.0f);
                blob->color = CHSV(i * 50, 255, 255);  // Evenly spaced colors
                blobs.push_back(std::move(blob));
            }
            Serial.printf("Created %d hardcoded blobs\n", blobs.size());
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
        if (millis() - last_param_debug > 5000) {
            Serial.printf("Fade parameter: %d\n", fade_amount);
            last_param_debug = millis();
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
        if (millis() - last_debug_pos > 5000 && !blobs.empty()) {
            auto& blob = blobs[0];
            Serial.printf("Blob 0 position: (%d, %d, %d), radius: %d\n", 
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
                    Serial.printf("  Blob is within range of LED %d at (%d, %d, %d), dist: %d\n", 
                                 i, point.x(), point.y(), point.z(), (int)sqrt(dist));
                    found_led = true;
                    break;
                }
            }
            
            if (!found_led) {
                Serial.println("  Blob is not within range of first 10 LEDs");
            }
            
            last_debug_pos = millis();
        }
        
        // End overall scene benchmark
        BENCHMARK_END();
    }
    
    void updateBlobs() {
        static const float forceStrength = 0.000005;  // Tuning variable for repelling force
        
        // Debug output - only print occasionally to avoid flooding
        static uint32_t last_debug = 0;
        if (millis() - last_debug > 5000) {
            Serial.printf("Updating %d blobs\n", blobs.size());
            last_debug = millis();
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
        if (millis() - last_debug > 5000) {
            Serial.printf("Drawing %d blobs, model has %d faces\n", 
                         blobs.size(), this->stage.model.face_count());
            last_debug = millis();
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
                        // Convert FastLED CRGB to PixelTheater CRGB
                        PixelTheater::CRGB c(blob->color.r, blob->color.g, blob->color.b);
                        
                        // Apply fade-in effect for new blobs
                        if (blob->age < 150) {
                            // Calculate fade amount based on age
                            uint8_t fade_amount = map(blob->age, 0, 150, 180, 1);
                            // Use PixelTheater's fadeToBlackBy for the fade-in effect
                            PixelTheater::fadeToBlackBy(c, fade_amount);
                        }
                        
                        // Blend color based on distance (closer = brighter)
                        uint8_t blend_amount = map(dist, 0, rad_sq, 7, 3);
                        
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
        String output;
        
        // Get parameters directly from settings
        float speed = this->settings["speed"];
        int fade = this->settings["fade"];
        int min_radius = this->settings["min_radius"];
        int max_radius = this->settings["max_radius"];
        int max_age = this->settings["max_age"];
        
        output += "Blobs: " + String(blobs.size()) + " active (speed=" + 
                  String(speed, 2) + 
                  ", fade=" + String(fade) + ")\n";
        
        output += "Radius: " + String(min_radius) + 
                  "-" + String(max_radius) + 
                  ", MaxAge: " + String(max_age) + "\n";
        
        // Show info for first 3 blobs to avoid cluttering the display
        for (size_t i = 0; i < std::min(size_t(3), blobs.size()); i++) {
            const auto& blob = blobs[i];
            output += "Blob " + String(blob->blob_id) + 
                      ": age=" + String(blob->age) + "/" + String(blob->lifespan) + 
                      " accel=" + String(blob->av, 2) + "/" + String(blob->cv, 2) + "\n";
        }
        
        return output.c_str();
    }
    
private:
    std::vector<std::unique_ptr<Blob<ModelDef>>> blobs;
};

} // namespace Scenes 