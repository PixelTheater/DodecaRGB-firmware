#pragma once

#include "PixelTheater/scene.h"
#include "PixelTheater/core/color.h"
#include "PixelTheater/core/time.h"
#include "PixelTheater/core/log.h"
#include "PixelTheater/core/math.h"
#include "PixelTheater/constants.h"
#include <vector>
#include <memory>
#include <cmath>

namespace Scenes {

// Get access to the math provider
extern PixelTheater::MathProvider& getMathProvider();

// Forward declaration
template<typename ModelDef>
class WanderingParticlesScene;

// Particle class - represents a single wandering particle
template<typename ModelDef>
class Particle {
public:
    WanderingParticlesScene<ModelDef>& scene;
    uint16_t particle_id = 0;
    
    // Position and movement
    float a = 0;        // azimuth angle (0 to 2π)
    float c = 0;        // polar angle (0 to π)
    float av = 0;       // azimuthal velocity
    float cv = 0;       // polar velocity
    
    // Particle state
    int age = 0;
    int hold_time = 0;
    int led_number = -1;
    
    // Path history - stores the last few LEDs the particle visited
    static constexpr size_t MAX_PATH_LENGTH = 10;
    std::vector<int> path;
    
    // Color
    PixelTheater::CRGB color = PixelTheater::CRGB(255, 255, 255);
    
    Particle(WanderingParticlesScene<ModelDef>& parent_scene, uint16_t unique_id)
        : scene(parent_scene), particle_id(unique_id), path(MAX_PATH_LENGTH, -1) {
        reset();
    }
    
    void reset() {
        // Random starting position around the sphere
        led_number = getMathProvider().random(scene.stage.model.led_count());
        
        // Initialize path with current LED
        std::fill(path.begin(), path.end(), -1);
        path[0] = led_number;
        
        // Random color (mostly blue/green with a bit of red)
        // Match original color scheme
        uint8_t lev = getMathProvider().random8(10, 50);
        color = PixelTheater::CRGB(lev, getMathProvider().random8(100, 230), lev);
        
        // Random starting angles - use much smaller values like the original
        // Original: this->a = random(20, 40)/15000.0;
        // Original: this->c = random(TWO_PI*1000)/15000.0;
        a = getMathProvider().random(20, 40) / 15000.0;
        c = getMathProvider().random(PixelTheater::Constants::PT_TWO_PI * 1000) / 15000.0;
        
        // Random velocities - use much smaller values like the original
        // Original: this->av = random(20, 60)/25000.0;
        // Original: this->cv = random(20, 60)/24000.0;
        av = getMathProvider().random(20, 60) / 25000.0;
        cv = getMathProvider().random(20, 60) / 24000.0;
        
        // Reset age and hold time - match original
        // Original: this->hold_time = random(6,12);
        age = 0;
        hold_time = getMathProvider().random8(6, 12);
    }
    
    // Helper methods to calculate 3D position
    float x() const { return scene.sphere_radius * sin(c) * cos(a); }
    float y() const { return scene.sphere_radius * sin(c) * sin(a); }
    float z() const { return scene.sphere_radius * cos(c); }
    
    void tick() {
        // Increment age
        age++;
        
        // Only move to next LED after hold time expires
        if (age > hold_time) {
            // Update position based on velocities
            a += av;
            c += cv;
            
            // Normalize angles
            if (a > PixelTheater::Constants::PT_TWO_PI) a -= PixelTheater::Constants::PT_TWO_PI;
            if (a < 0) a += PixelTheater::Constants::PT_TWO_PI;
            if (c > PixelTheater::Constants::PT_PI) c = PixelTheater::Constants::PT_PI;
            if (c < 0) c = 0;
            
            // Find the next LED to move to
            findNextLed();
            
            // Update path history
            path.insert(path.begin(), led_number);
            path.resize(MAX_PATH_LENGTH);
            
            // Reset age for next hold
            age = 0;
        }
    }
    
    void findNextLed() {
        // Calculate theoretical 3D position
        float px = x();
        float py = y();
        float pz = z();
        
        // Get current LED position
        const auto& current_point = scene.stage.model.points[led_number];
        
        // Find the best neighbor to move to
        float closest_dist = 1e9;
        int closest_led = -1;
        
        // Search a subset of LEDs that are likely to be neighbors
        // Use a much smaller search radius to better match the original behavior
        // which only looked at immediate neighbors
        const int SEARCH_RADIUS = 30;  // Reduced from 50 to 30 mm
        
        // Limit the number of candidates to check, similar to the original
        // which only checked MAX_LED_NEIGHBORS (typically 7)
        const int MAX_CANDIDATES = 7;
        int candidates_checked = 0;
        
        // Create a temporary point at the theoretical position
        // Note: This is just for distance calculation and doesn't need to be added to the model
        PixelTheater::Point theoretical_point(0, 0, px, py, pz);
        
        for (size_t led_idx = 0; led_idx < scene.stage.model.led_count() && candidates_checked < MAX_CANDIDATES; led_idx++) {
            // Skip current LED
            if (static_cast<int>(led_idx) == led_number) continue;
            
            // Skip LEDs already in the recent path to avoid backtracking
            if (std::find(path.begin(), path.begin() + std::min(size_t(3), path.size()), 
                         static_cast<int>(led_idx)) != path.begin() + std::min(size_t(3), path.size())) {
                
                const auto& point = scene.stage.model.points[led_idx];
                
                // Calculate distance between points using Point::distanceTo if available
                // Otherwise fall back to manual calculation
                float point_dist;
                try {
                    // Try to use the Point::distanceTo method
                    point_dist = current_point.distanceTo(point);
                } catch (...) {
                    // Fall back to manual calculation
                    float dx = point.x() - current_point.x();
                    float dy = point.y() - current_point.y();
                    float dz = point.z() - current_point.z();
                    point_dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                }
                
                // Only consider points within the search radius (likely neighbors)
                if (point_dist < SEARCH_RADIUS) {
                    candidates_checked++;
                    
                    // Calculate distance to theoretical position
                    float dist;
                    try {
                        // Try to use the Point::distanceTo method
                        dist = point.distanceTo(theoretical_point);
                    } catch (...) {
                        // Fall back to manual calculation
                        float dx = point.x() - px;
                        float dy = point.y() - py;
                        float dz = point.z() - pz;
                        dist = dx*dx + dy*dy + dz*dz;
                    }
                    
                    // Check if this is closer than current best
                    if (dist < closest_dist) {
                        closest_dist = dist;
                        closest_led = led_idx;
                    }
                }
            }
        }
        
        // If we found a valid LED, move to it
        if (closest_led >= 0) {
            led_number = closest_led;
        }
    }
};

// WanderingParticlesScene class - manages multiple particles
template<typename ModelDef>
class WanderingParticlesScene : public PixelTheater::Scene<ModelDef> {
public:
    using Scene = PixelTheater::Scene<ModelDef>;
    using Scene::Scene;  // Inherit constructor
    
    // Default values - match original
    static constexpr int DEFAULT_NUM_PARTICLES = 80;  // Original: static const int NUM_PARTICLES = 80;
    static constexpr uint8_t DEFAULT_FADE = 20;       // Original: fadeToBlackBy(leds, numLeds(), 20);
    static constexpr float DEFAULT_BLEND = 80.0f;     // Original: 80/(particles[i]->hold_time - particles[i]->age + 1)
    static constexpr int DEFAULT_RESET_CHANCE = 2;    // Original: if (random8() < 2)
    
    // Sphere radius for positioning
    int sphere_radius = 100;
    
    void setup() override {
        // Set scene metadata
        this->set_name("Wandering Particles");
        this->set_description("Particles that wander across the model's surface");
        this->set_version("1.0");
        this->set_author("PixelTheater Team");
        
        // Define parameter ranges
        const int MIN_PARTICLES = 5;
        const int MAX_PARTICLES = 200;  // Increased from 100 to 200
        
        const int MIN_FADE = 1;
        const int MAX_FADE = 50;
        
        const float MIN_BLEND = 10.0f;
        const float MAX_BLEND = 200.0f;
        
        const int MIN_RESET = 1;
        const int MAX_RESET = 20;  // Increased from 10 to 20
        
        // Define parameters
        this->param("num_particles", "count", MIN_PARTICLES, MAX_PARTICLES, DEFAULT_NUM_PARTICLES, "clamp", "Number of particles");
        this->param("fade_amount", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame (lower = longer trails)");
        this->param("blend_amount", "range", MIN_BLEND, MAX_BLEND, DEFAULT_BLEND, "clamp", "Blend intensity (higher = brighter particles)");
        this->param("reset_chance", "count", MIN_RESET, MAX_RESET, DEFAULT_RESET_CHANCE, "clamp", "Chance of particle reset (higher = more frequent)");
        
        // Estimate sphere radius from model
        estimateSphereRadius();
        
        // Initialize particles
        initParticles();
        
        PixelTheater::Log::warning("WanderingParticlesScene setup complete with %d particles", particles.size());
    }
    
    void estimateSphereRadius() {
        // Find the maximum distance from origin to any point
        int max_dist = 0;
        for (size_t i = 0; i < this->stage.model.led_count(); i++) {
            const auto& point = this->stage.model.points[i];
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
    
    void initParticles() {
        // Get number of particles from settings
        int num_particles = this->settings["num_particles"];
        
        // Create particles
        particles.clear();
        for (int i = 0; i < num_particles; i++) {
            auto particle = std::make_unique<Particle<ModelDef>>(*this, i);
            particles.push_back(std::move(particle));
        }
        
        PixelTheater::Log::warning("Created %d particles", particles.size());
    }
    
    void tick() override {
        Scene::tick();  // Call base to increment counter
        
        // Get parameters
        uint8_t fade_amount = static_cast<uint8_t>(this->settings["fade_amount"]);
        float blend_amount = this->settings["blend_amount"];
        int reset_chance = this->settings["reset_chance"];
        
        // Fade all LEDs - match original fade amount
        for (auto& led : this->stage.leds) {
            PixelTheater::fadeToBlackBy(led, fade_amount);
        }
        
        // Update and render particles
        for (auto& particle : particles) {
            // Update particle position
            particle->tick();
            
            // Render particle at current position (head of the trail)
            // This matches the original implementation more closely
            int led_num = particle->led_number;
            if (led_num >= 0 && led_num < this->stage.model.led_count()) {
                // Use original blend calculation:
                // nblend(leds[led_num], particles[i]->color, 80/(particles[i]->hold_time - particles[i]->age + 1));
                uint8_t blend = std::min(255.0f, std::max(1.0f, 
                    blend_amount / (particle->hold_time - particle->age + 1)));
                
                // Use nblend for a more authentic matrix-like trail effect
                PixelTheater::nblend(this->stage.leds[led_num], particle->color, blend);
            }
            
            // Also render the trail (previous positions)
            for (size_t i = 1; i < particle->path.size(); i++) {
                int trail_led = particle->path[i];
                if (trail_led >= 0 && trail_led < this->stage.model.led_count()) {
                    // Decrease brightness for older positions in the trail
                    uint8_t trail_blend = std::min(255.0f, std::max(1.0f, 
                        blend_amount / (i * 2 + 1)));
                    
                    PixelTheater::nblend(this->stage.leds[trail_led], particle->color, trail_blend);
                }
            }
            
            // Randomly reset particles
            if (getMathProvider().random8() < reset_chance) {
                particle->reset();
            }
        }
    }
    
    std::string status() const {
        std::string output;
        
        // Get parameters
        int num_particles = this->settings["num_particles"];
        int fade_amount = this->settings["fade_amount"];
        float blend_amount = this->settings["blend_amount"];
        int reset_chance = this->settings["reset_chance"];
        
        output += "Particles: " + std::to_string(num_particles) + 
                  " (fade=" + std::to_string(fade_amount) + 
                  ", blend=" + std::to_string(blend_amount) + 
                  ", reset=" + std::to_string(reset_chance) + ")\n";
        
        // Show info for first 3 particles
        for (size_t i = 0; i < std::min(size_t(3), particles.size()); i++) {
            const auto& particle = particles[i];
            output += "Particle " + std::to_string(particle->particle_id) + 
                      ": age=" + std::to_string(particle->age) + "/" + std::to_string(particle->hold_time) + 
                      " led=" + std::to_string(particle->led_number) + "\n";
        }
        
        return output;
    }
    
private:
    std::vector<std::unique_ptr<Particle<ModelDef>>> particles;
};

} // namespace Scenes 