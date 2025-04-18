#include "wandering_particles_scene.h"
#include "PixelTheater.h" // Include base library for Scene members, types, utils
#include "benchmark.h"   // Include benchmark helpers if used

// Include standard libraries used by the implementations
#include <vector>
#include <memory>
#include <cmath>      // For sqrt
#include <algorithm>  // For std::min, std::max
#include <string>     // For std::to_string in status()

namespace Scenes {

// --- WanderingParticlesScene Implementation ---

void WanderingParticlesScene::setup() {
    set_name("Wandering Particles");
    set_description("Particles that wander across the model surface");
    set_version("1.0");
    set_author("PixelTheater Team");
    
    // Define parameter ranges and defaults
    const int MIN_PARTICLES = 5, MAX_PARTICLES = 200;  
    const int MIN_FADE = 1, MAX_FADE = 50;
    const float MIN_BLEND = 10.0f, MAX_BLEND = 200.0f;
    const int MIN_RESET = 1, MAX_RESET = 20; // Note: Old version used 0-255 random check
        
    // Define parameters
    param("num_particles", "count", MIN_PARTICLES, MAX_PARTICLES, DEFAULT_NUM_PARTICLES, "clamp", "Number of particles");
    param("fade_amount", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame");
    param("blend_amount", "range", MIN_BLEND, MAX_BLEND, DEFAULT_BLEND, "clamp", "Blend intensity");
    param("reset_chance", "count", MIN_RESET, MAX_RESET, DEFAULT_RESET_CHANCE, "clamp", "Chance of particle reset (1-20)");
    
    // Estimate model radius (affects particle x,y,z calculations)
    estimateSphereRadius(); 
    
    // Initialize particles based on parameters
    initParticles();
    
    logInfo("WanderingParticlesScene setup complete");
}

void WanderingParticlesScene::estimateSphereRadius() {
    int max_dist_sq = 0;
    size_t count = model().pointCount(); // Use model().pointCount() for safety
    if (count == 0) { 
        logWarning("Cannot estimate sphere radius: No points in model");
        sphere_radius = 100; // Use default if no points
        return; 
    }
    for (size_t i = 0; i < count; i++) { 
        const auto& p = model().point(i);
        int dist_sq = p.x() * p.x() + p.y() * p.y() + p.z() * p.z();
        max_dist_sq = std::max(max_dist_sq, dist_sq);
    }
    if (max_dist_sq > 1) { // Use > 1 to avoid issues with tiny models
        sphere_radius = static_cast<int>(sqrt(static_cast<float>(max_dist_sq)));
        logInfo("Estimated sphere radius: %d", sphere_radius);
    } else {
        logWarning("Could not estimate sphere radius or radius too small, using default %d", sphere_radius);
        // Keep default sphere_radius = 100 if estimation fails or is too small
    }
}

void WanderingParticlesScene::initParticles() {
    int num_particles = settings["num_particles"];
    logInfo("Creating %d particles...", num_particles);
    particles.clear(); // Clear existing particles
    particles.reserve(num_particles); // Reserve space
    for (int i = 0; i < num_particles; i++) {
        // Create particles using unique_ptr
        particles.push_back(std::make_unique<Particle>(*this, i));
    }
    logInfo("%d Particles created.", (int)particles.size());
}

void WanderingParticlesScene::tick() {
    Scene::tick(); // Call base class tick
    
    // Get current parameter values
    uint8_t fade_amount = static_cast<uint8_t>(settings["fade_amount"]);
    float blend_amount = settings["blend_amount"];
    int reset_chance = settings["reset_chance"]; // This is 1-20
    // Convert reset_chance (1-20) to a probability check (approx 0-255)
    // A higher reset_chance param value means MORE resets
    int reset_check_threshold = reset_chance * (255 / MAX_RESET); 

    // Fade all LEDs first
    size_t count = ledCount();
    for(size_t i = 0; i < count; ++i) {
        // Using the global fade utility function
        PixelTheater::fadeToBlackBy(leds[i], fade_amount);
    }
        
    // Update and draw each particle
    for (auto& particle : particles) {
        particle->tick(); // Update particle state (position, LED)
        
        // Draw particle head
        int head_led = particle->led_number;
        if (head_led >= 0 && head_led < (int)count) {
            // Calculate blend amount based on age within hold_time
            uint8_t blend = 255; // Default to full brightness
            if (particle->hold_time > 0) {
                 blend = std::min(255.0f, std::max(1.0f, 
                    blend_amount / (particle->hold_time - particle->age + 1)));
            }
            PixelTheater::nblend(leds[head_led], particle->color, blend);
        }
        
        // Draw particle trail
        for (size_t i = 1; i < particle->path.size(); i++) {
            int trail_led = particle->path[i];
            if (trail_led >= 0 && trail_led < (int)count) {
                // Blend amount decreases along the trail
                uint8_t trail_blend = std::min(255.0f, std::max(1.0f, 
                    blend_amount / (i * 3 + 1))); // Trail fades faster
                PixelTheater::nblend(leds[trail_led], particle->color, trail_blend);
            }
        }
        
        // Randomly reset particles based on chance
        if (random(256) < static_cast<uint32_t>(reset_check_threshold)) {
            particle->reset();
        }
    }
}

// Implementation for the status string    
std::string WanderingParticlesScene::status() const {
    std::string output;
    // Safely access settings - maybe they haven't been initialized?
    int num_particles = particles.size(); // Use actual size
    int fade_amount = 0; 
    float blend_amount = 0.0f;
    int reset_chance = 0;
    if (has_parameter("fade_amount")) fade_amount = settings["fade_amount"];
    if (has_parameter("blend_amount")) blend_amount = settings["blend_amount"];
    if (has_parameter("reset_chance")) reset_chance = settings["reset_chance"];
    
    output += "Particles: " + std::to_string(num_particles) + 
              " (fade=" + std::to_string(fade_amount) + 
              ", blend=" + std::to_string(static_cast<int>(blend_amount)) + // Cast float for display
              ", reset=" + std::to_string(reset_chance) + ")\n";
              
    // Show info for first 3 particles if they exist
    for (size_t i = 0; i < std::min(size_t(3), particles.size()); i++) {
        const auto& p = particles[i];
        if (p) { // Check if unique_ptr is valid
            output += "P" + std::to_string(p->particle_id) + 
                      ": age=" + std::to_string(p->age) + "/" + std::to_string(p->hold_time) + 
                      " led=" + std::to_string(p->led_number) + "\n";
        }
    }
    return output;
}

} // namespace Scenes 