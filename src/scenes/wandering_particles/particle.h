#pragma once

#include <cstdint>
#include <vector>
#include <memory> // Often included with vector of unique_ptr
#include "PixelTheater/core/crgb.h" // Include CRGB definition

namespace Scenes {

// Forward declaration
class WanderingParticlesScene;

// Particle State Enum
enum class ParticleState {
    FADING_IN,
    ALIVE,
    FADING_OUT
};

// Particle class
class Particle {
public:
    // Constants for fade durations (in ticks)
    static constexpr int FADE_IN_DURATION = 20;
    static constexpr int FADE_OUT_DURATION = 30;

    WanderingParticlesScene& scene; // Reference to the parent scene
    uint16_t particle_id = 0;
    float a = 0.0f, c = 0.0f;        // Position (angular)
    float av = 0.0f, cv = 0.0f;    // Velocity (angular)
    int age = 0;
    int hold_time = 0;
    int lifespan = 500; // Add lifespan member
    int led_number = -1; // Current LED index the particle occupies
    static constexpr size_t MAX_PATH_LENGTH = 10;
    std::vector<int> path; // Recent path (trail) of LED indices
    PixelTheater::CRGB color = PixelTheater::CRGB::White;
    int ticks_at_pole = 0; // Add counter for ticks stuck at a pole
    ParticleState state = ParticleState::FADING_IN; // Add state member
    
    Particle(WanderingParticlesScene& parent_scene, uint16_t unique_id);
    
    void reset();
    float x() const; // Calculated Cartesian position
    float y() const; // Calculated Cartesian position
    float z() const; // Calculated Cartesian position
    void tick();

private:
    void findNextLed(float gravity_strength); // Add gravity parameter
    void resetAtOppositePole(bool stuck_at_north_pole); // Add helper for pole reset
    void initializeParticleState(int start_led_number); // Add helper declaration
};

} // namespace Scenes 