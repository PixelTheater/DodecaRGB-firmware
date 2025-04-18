#pragma once

#include <cstdint>
#include <vector>
#include <memory> // Often included with vector of unique_ptr
#include "PixelTheater/core/crgb.h" // Include CRGB definition

namespace Scenes {

// Forward declaration
class WanderingParticlesScene;

// Particle class
class Particle {
public:
    WanderingParticlesScene& scene; // Reference to the parent scene
    uint16_t particle_id = 0;
    float a = 0.0f, c = 0.0f;        // Position (angular)
    float av = 0.0f, cv = 0.0f;    // Velocity (angular)
    int age = 0;
    int hold_time = 0;
    int led_number = -1; // Current LED index the particle occupies
    static constexpr size_t MAX_PATH_LENGTH = 10;
    std::vector<int> path; // Recent path (trail) of LED indices
    PixelTheater::CRGB color = PixelTheater::CRGB::White;
    
    Particle(WanderingParticlesScene& parent_scene, uint16_t unique_id);
    
    void reset();
    float x() const; // Calculated Cartesian position
    float y() const; // Calculated Cartesian position
    float z() const; // Calculated Cartesian position
    void tick();

private:
    void findNextLed(); // Made private as it's an internal helper for tick()
};

} // namespace Scenes 