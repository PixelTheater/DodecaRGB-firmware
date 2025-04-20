#pragma once

#include "PixelTheater/SceneKit.h" 
#include "benchmark.h"
#include "particle.h" // Include the new particle header

#include <vector>
#include <memory>
#include <cmath>
#include <algorithm> // For std::max, std::min, std::clamp, std::find, std::fill
#include <string>

namespace Scenes {

// Forward declaration removed (now included via particle.h)
// class WanderingParticlesScene;

// Particle class definition removed 
// ...

// WanderingParticlesScene class
class WanderingParticlesScene : public Scene { 
public:
    WanderingParticlesScene() = default;
    
    // Static constants for default parameters
    static constexpr int DEFAULT_NUM_PARTICLES = 80;
    static constexpr uint8_t DEFAULT_FADE = 30;
    static constexpr float DEFAULT_BLEND = 130.0f;
    static constexpr float DEFAULT_GRAVITY = 2.2f; // Default no gravity
    static constexpr int MAX_RESET = 20; // Define MAX_RESET here
    
    // Member accessible by Particle (can be private if Particle is friend)
    int sphere_radius = 100; 
    
    // Scene lifecycle methods (Declarations only)
    void setup() override;
    void tick() override;
    std::string status() const override;
    // void reset() override; // Optional: Add declaration if needed

    // Scene-specific logic (Declarations only)
    void initParticles();
    void estimateSphereRadius();

    // Allow Particle to access private members if necessary (e.g., sphere_radius if private)
    friend class Particle; 

private:
    std::vector<std::unique_ptr<Particle>> particles;
};

// Implementations removed (moved to .cpp file)

} // namespace Scenes 