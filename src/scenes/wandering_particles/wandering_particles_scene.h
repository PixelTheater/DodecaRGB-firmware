#pragma once

#include "PixelTheater.h" 
#include "benchmark.h"
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm> // For std::max, std::min, std::clamp, std::find, std::fill
#include <string>

namespace Scenes {

// Forward declaration
class WanderingParticlesScene;

// Particle class
class Particle {
public:
    WanderingParticlesScene& scene; // Use non-templated Scene type
    uint16_t particle_id = 0;
    float a = 0.0f, c = 0.0f;        
    float av = 0.0f, cv = 0.0f;    
    int age = 0;
    int hold_time = 0;
    int led_number = -1;
    static constexpr size_t MAX_PATH_LENGTH = 10;
    std::vector<int> path;
    PixelTheater::CRGB color = PixelTheater::CRGB::White;
    
    Particle(WanderingParticlesScene& parent_scene, uint16_t unique_id);
    
    void reset();
    float x() const; // Implementation needs access to scene.sphere_radius
    float y() const; 
    float z() const; 
    void tick();
    void findNextLed();
};

// WanderingParticlesScene class
class WanderingParticlesScene : public PixelTheater::Scene { 
public:
    WanderingParticlesScene() = default;
    
    static constexpr int DEFAULT_NUM_PARTICLES = 80;
    static constexpr uint8_t DEFAULT_FADE = 20;
    static constexpr float DEFAULT_BLEND = 80.0f;
    static constexpr int DEFAULT_RESET_CHANCE = 2;    
    
    int sphere_radius = 100; // Make public or provide getter for Particle
    
    void setup() override;
    void initParticles();
    void estimateSphereRadius();
    void tick() override;
    std::string status() const override;
    
    friend class Particle; // Allow Particle to access helpers and sphere_radius

private:
    std::vector<std::unique_ptr<Particle>> particles;
};

// --- Implementations --- 

// Particle Implementation
Particle::Particle(WanderingParticlesScene& parent_scene, uint16_t unique_id)
    : scene(parent_scene), particle_id(unique_id), path(MAX_PATH_LENGTH, -1) {
    reset();
}

void Particle::reset() {
    led_number = scene.random(scene.ledCount()); 
    std::fill(path.begin(), path.end(), -1);
    path[0] = led_number;
    uint8_t lev = scene.random(10, 51); 
    color = PixelTheater::CRGB(lev, scene.random(100, 231), lev);
    a = scene.randomFloat(0.02f, 0.04f) / 15.0f; 
    c = scene.randomFloat(0.0f, PixelTheater::Constants::PT_TWO_PI) / 15.0f;
    av = scene.randomFloat(0.02f, 0.06f) / 25.0f;
    cv = scene.randomFloat(0.02f, 0.06f) / 24.0f;
    age = 0;
    hold_time = scene.random(6, 12);
}

float Particle::x() const { return scene.sphere_radius * sin(c) * cos(a); }
float Particle::y() const { return scene.sphere_radius * sin(c) * sin(a); }
float Particle::z() const { return scene.sphere_radius * cos(c); }

void Particle::tick() {
    age++;
    if (age > hold_time) {
        a += av;
        c += cv;
        using PixelTheater::Constants::PT_PI;
        using PixelTheater::Constants::PT_TWO_PI;
        if (a > PT_TWO_PI) a -= PT_TWO_PI;
        if (a < 0.0f) a += PT_TWO_PI;
        c = std::clamp(c, 0.0f, PT_PI); // Clamp polar angle
        findNextLed();
        path.insert(path.begin(), led_number);
        path.resize(MAX_PATH_LENGTH);
        age = 0;
    }
}

void Particle::findNextLed() {
    float px = x();
    float py = y();
    float pz = z();
    if (led_number < 0 || led_number >= (int)scene.ledCount()) { // Safety check
         reset(); // Reset if current LED is invalid
         return;
    }
    // Access the current point and its pre-calculated neighbors
    const auto& current_point = scene.model().point(led_number);
    const auto& neighbors = current_point.getNeighbors(); 
    
    float closest_dist_sq = 1e18f; // Use squared distance
    int closest_led = -1;
    
    // Iterate through the pre-calculated neighbors
    for (const auto& neighbor : neighbors) {
        // Check if the neighbor is valid (using the sentinel ID from point.cpp)
        if (neighbor.id == 0xFFFF || neighbor.distance <= 0.0f) {
            continue; // Skip invalid or padding entries
        }

        int led_idx = neighbor.id;

        // Check if this neighbor is in the recent path
        bool in_path = false;
        for(size_t p_idx = 0; p_idx < std::min(size_t(3), path.size()); ++p_idx) {
            if (path[p_idx] == led_idx) { in_path = true; break; }
        }
        if (in_path) continue;

        // Get the neighbor's point data
        // Check bounds just in case generated data is bad
        if (led_idx >= (int)scene.ledCount()) continue;

        const auto& neighbor_point = scene.model().point(led_idx);

        // Calculate distance from the particle's target position to the neighbor point
        float dx = neighbor_point.x() - px;
        float dy = neighbor_point.y() - py;
        float dz = neighbor_point.z() - pz;
        float dist_sq = dx*dx + dy*dy + dz*dz;

        // If this neighbor is closer to the target position, select it
        if (dist_sq < closest_dist_sq) {
            closest_dist_sq = dist_sq;
            closest_led = led_idx;
        }
    }
        
    if (closest_led >= 0) {
        led_number = closest_led;
    }
     // else: stay on the current LED if no suitable neighbor found
}

// WanderingParticlesScene Implementation
void WanderingParticlesScene::setup() {
    set_name("Wandering Particles");
    set_description("Particles that wander across the model surface");
    set_version("1.0");
    set_author("PixelTheater Team");
    
    const int MIN_PARTICLES = 5, MAX_PARTICLES = 200;  
    const int MIN_FADE = 1, MAX_FADE = 50;
    const float MIN_BLEND = 10.0f, MAX_BLEND = 200.0f;
    const int MIN_RESET = 1, MAX_RESET = 20; 
        
    param("num_particles", "count", MIN_PARTICLES, MAX_PARTICLES, DEFAULT_NUM_PARTICLES, "clamp", "Number of particles");
    param("fade_amount", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame");
    param("blend_amount", "range", MIN_BLEND, MAX_BLEND, DEFAULT_BLEND, "clamp", "Blend intensity");
    param("reset_chance", "count", MIN_RESET, MAX_RESET, DEFAULT_RESET_CHANCE, "clamp", "Chance of particle reset");
    
    estimateSphereRadius(); // Estimate based on model
    initParticles();
    logInfo("WanderingParticlesScene setup complete");
}

void WanderingParticlesScene::estimateSphereRadius() {
    int max_dist_sq = 0;
    size_t count = model().pointCount();
    if (count == 0) { 
        logWarning("Cannot estimate sphere radius: No points in model");
        return; 
    }
    for (size_t i = 0; i < count; i++) { 
        const auto& p = model().point(i);
        int dist_sq = p.x() * p.x() + p.y() * p.y() + p.z() * p.z();
        max_dist_sq = std::max(max_dist_sq, dist_sq);
    }
    if (max_dist_sq > 0) {
        sphere_radius = static_cast<int>(sqrt(static_cast<float>(max_dist_sq)));
        logInfo("Radius estimate done");
    } else {
        logWarning("Could not estimate sphere radius, using default");
        sphere_radius = 100; // Keep default if estimation fails
    }
}

void WanderingParticlesScene::initParticles() {
    int num_particles = settings["num_particles"];
    logInfo("Creating particles...");
    particles.clear();
    particles.reserve(num_particles);
    for (int i = 0; i < num_particles; i++) {
        particles.push_back(std::make_unique<Particle>(*this, i));
    }
    logInfo("Particles created.");
}

void WanderingParticlesScene::tick() {
    Scene::tick();  
    
    uint8_t fade_amount = static_cast<uint8_t>(settings["fade_amount"]);
    float blend_amount = settings["blend_amount"];
    int reset_chance = settings["reset_chance"];
    
    size_t count = ledCount();
    for(size_t i = 0; i < count; ++i) {
        PixelTheater::fadeToBlackBy(leds[i], fade_amount);
    }
        
    for (auto& particle : particles) {
        particle->tick();
        int head_led = particle->led_number;
        if (head_led >= 0 && head_led < (int)count) {
            uint8_t blend = std::min(255.0f, std::max(1.0f, 
                blend_amount / (particle->hold_time - particle->age + 1)));
            PixelTheater::nblend(leds[head_led], particle->color, blend);
        }
        for (size_t i = 1; i < particle->path.size(); i++) {
            int trail_led = particle->path[i];
            if (trail_led >= 0 && trail_led < (int)count) {
                uint8_t trail_blend = std::min(255.0f, std::max(1.0f, 
                    blend_amount / (i * 3 + 1))); 
                PixelTheater::nblend(leds[trail_led], particle->color, trail_blend);
            }
        }
        if (random(256) < reset_chance) {
            particle->reset();
        }
    }
}
    
std::string WanderingParticlesScene::status() const {
    std::string output;
    int num_particles = settings["num_particles"];
    int fade_amount = settings["fade_amount"];
    float blend_amount = settings["blend_amount"];
    int reset_chance = settings["reset_chance"];
    output += "Particles: " + std::to_string(num_particles) + 
              " (fade=" + std::to_string(fade_amount) + 
              ", blend=" + std::to_string(blend_amount) + 
              ", reset=" + std::to_string(reset_chance) + ")\n";
    // Show info for first 3 particles
    for (size_t i = 0; i < std::min(size_t(3), particles.size()); i++) {
        const auto& p = particles[i];
        output += "P" + std::to_string(p->particle_id) + 
                  ": age=" + std::to_string(p->age) + "/" + std::to_string(p->hold_time) + 
                  " led=" + std::to_string(p->led_number) + "\n";
    }
    return output;
}
    
} // namespace Scenes 