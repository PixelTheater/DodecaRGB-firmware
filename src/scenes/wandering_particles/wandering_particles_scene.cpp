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
    set_description("Particles that wander across the model surface, affected by gravity.");
    set_version("1.1"); // Version bump
    set_author("PixelTheater Team");
    
    // Define parameter ranges and defaults
    const int MIN_PARTICLES = 5, MAX_PARTICLES = 200;  
    const int MIN_FADE = 1, MAX_FADE = 50;
    const float MIN_BLEND = 10.0f, MAX_BLEND = 200.0f;
    const float MIN_GRAVITY = -2.5f, MAX_GRAVITY = 2.5f; // Gravity range
        
    // Define parameters
    param("num_particles", "count", MIN_PARTICLES, MAX_PARTICLES, DEFAULT_NUM_PARTICLES, "clamp", "Number of particles");
    param("fade_amount", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame");
    param("blend_amount", "range", MIN_BLEND, MAX_BLEND, DEFAULT_BLEND, "clamp", "Blend intensity");
    param("gravity", "range", MIN_GRAVITY, MAX_GRAVITY, DEFAULT_GRAVITY, "clamp", "Z-axis gravity (+down/-up)"); // Add gravity param
    
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
    // int reset_chance = settings["reset_chance"]; // Remove usage
    // Convert reset_chance (1-20) to a probability check (approx 0-255)
    // A higher reset_chance param value means MORE resets
    // int reset_check_threshold = settings["reset_chance"] * (255 / MAX_RESET); // Remove usage

    // Fade all LEDs first
    size_t count = ledCount();
    for(size_t i = 0; i < count; ++i) {
        // Using the global fade utility function
        PixelTheater::fadeToBlackBy(leds[i], fade_amount);
    }
        
    // Update and draw each particle
    for (auto& particle_ptr : particles) {
        if (!particle_ptr) continue; // Skip if unique_ptr is null for some reason
        Particle& particle = *particle_ptr; // Dereference for easier access

        particle.tick(); // Update particle state (position, LED, age, state)
        
        // --- Calculate brightness multiplier based on state ---
        float brightness_multiplier = 1.0f;
        if (particle.state == ParticleState::FADING_IN) {
            brightness_multiplier = std::clamp((float)particle.age / Particle::FADE_IN_DURATION, 0.0f, 1.0f);
        } else if (particle.state == ParticleState::FADING_OUT) {
            brightness_multiplier = std::clamp((float)(particle.lifespan - particle.age) / Particle::FADE_OUT_DURATION, 0.0f, 1.0f);
        }
        // --- End brightness calculation ---
        
        // Draw particle head
        int head_led = particle.led_number;
        if (head_led >= 0 && head_led < (int)count) {
            uint8_t blend = 255; 
            if (particle.hold_time > 0) {
                 blend = std::min(255.0f, std::max(1.0f, 
                    blend_amount / (particle.hold_time - particle.age + 1)));
            }
            // Apply brightness multiplier
            uint8_t final_blend = static_cast<uint8_t>(blend * brightness_multiplier);
            if (final_blend > 0) { // Avoid blending black
                 PixelTheater::nblend(leds[head_led], particle.color, final_blend/2);
            }
        }
        
        // Draw particle trail
        for (size_t i = 1; i < particle.path.size(); i++) {
            int trail_led = particle.path[i];
            if (trail_led >= 0 && trail_led < (int)count) {
                uint8_t trail_blend = std::min(255.0f, std::max(1.0f, 
                    blend_amount / (i * 3 + 1))); 
                // Apply brightness multiplier to trail as well, but less harshly
                // Blend the multiplier towards 1.0 so the trail dims slower than the head
                float trail_brightness_mult = (brightness_multiplier + 1.0f) / 2.0f; // Average with 1.0
                uint8_t final_trail_blend = static_cast<uint8_t>(trail_blend * trail_brightness_mult);
                if (final_trail_blend > 0) { // Avoid blending black
                    PixelTheater::nblend(leds[trail_led], particle.color, final_trail_blend);
                }
            }
        }
    }
    
    // --- ADD PARTICLE INTERACTION LOGIC HERE --- 
    // Check for collisions after all particles have been updated and drawn for this tick
    for (size_t i = 0; i < particles.size(); ++i) {
        if (!particles[i] || particles[i]->led_number < 0) continue; // Skip invalid particles
        for (size_t j = i + 1; j < particles.size(); ++j) {
            if (!particles[j] || particles[j]->led_number < 0) continue; // Skip invalid particles
            
            // Check if particles are on the same LED
            if (particles[i]->led_number == particles[j]->led_number) {
                // Collision! Randomize angular velocities (direction tendencies) for both
                float max_rand_av = 0.02f; // Adjust magnitude as needed
                float max_rand_cv = 0.02f;
                
                particles[i]->av = randomFloat(-max_rand_av, max_rand_av);
                particles[i]->cv = randomFloat(-max_rand_cv, max_rand_cv);
                
                particles[j]->av = randomFloat(-max_rand_av, max_rand_av);
                particles[j]->cv = randomFloat(-max_rand_cv, max_rand_cv);
                
                // Optional: Reset their age/hold timer to force immediate movement next tick?
                // particles[i]->age = particles[i]->hold_time; 
                // particles[j]->age = particles[j]->hold_time; 
            }
        }
    }
    // --- END PARTICLE INTERACTION LOGIC --- 

}

// Implementation for the status string    
std::string WanderingParticlesScene::status() const {
    std::string output;
    // Safely access settings - maybe they haven't been initialized?
    int num_particles = particles.size(); // Use actual size
    int fade_amount = 0; 
    float blend_amount = 0.0f;
    // int reset_chance = 0; // Removed
    if (has_parameter("fade_amount")) fade_amount = settings["fade_amount"];
    if (has_parameter("blend_amount")) blend_amount = settings["blend_amount"];
    // if (has_parameter("reset_chance")) reset_chance = settings["reset_chance"]; // Removed
    
    output += "Particles: " + std::to_string(num_particles) + 
              " (fade=" + std::to_string(fade_amount) + 
              ", blend=" + std::to_string(static_cast<int>(blend_amount)) + // Cast float for display
              // ", reset=" + std::to_string(reset_chance) + ")\n"; // Removed reset part
              ")\n";
              
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