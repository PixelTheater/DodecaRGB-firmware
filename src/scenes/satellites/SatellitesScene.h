#pragma once

#include "PixelTheater/SceneKit.h"
#include <vector>
// #include <Eigen/Core> // Removed: Eigen is included via SceneKit.h -> math.h

namespace Scenes {

class SatellitesScene : public Scene {
public:
    SatellitesScene() = default;
    ~SatellitesScene() override = default;

    void setup() override;
    void tick() override;
    // void reset() override; // Optional override
    std::string status() const override; // Add status method declaration

private:
    struct Satellite {
        enum class State {
            Orbiting,   // Normal path following
            Crashing,   // Crashing after impact
            Spawning,   // Spawning in
            Dead        // Waiting to respawn
        };

        Eigen::Vector3f position;       // Current position on sphere surface (calculated)
        State state = State::Dead;      // Current state
        float timer = 0.0f;             // Multi-purpose timer (Spawn/Crash/Respawn)

        // --- Surface Path Representation ---
        float phaseAngle = 0.0f;        // Angle along the orbital path (0 to 2pi)
        float angularSpeed = 0.0f;      // Rate of change of phaseAngle
        Eigen::Vector3f orbitAxis;      // Normalized axis of the orbit rotation
        Eigen::Vector3f orbitRefVec;    // Normalized reference vector on orbit equator (phase=0)
        // --- Removed old representations ---
        // float orbitRadius = 0.0f;       
        // float inclination = 0.0f;       
        // Eigen::Matrix3f rotationMatrix; 
        // float health = 1.0f;         // Removed - Crash on collision
        // --- End Removed ---

        uint32_t uniqueId = 0;          // Unique identifier
    };

    // --- Add Spark Particle Struct ---
    struct SparkParticle {
        Eigen::Vector3f position;
        Eigen::Vector3f velocity;
        CRGB color;
        float lifetime; // Remaining time in seconds
        float initialLifetime; // Initial lifetime for calculations
    };
    // --- End Spark Particle Struct ---

    std::vector<Satellite> satellites;
    std::vector<SparkParticle> sparkParticles; // Add vector for spark particles

    uint32_t nextUniqueId = 1;  // Start at 1 for more human-readable IDs

    // Constants
    // --- Remove Physics Constants --- 
    // static const float GRAVITATIONAL_CONSTANT_GM;
    // static const float DAMAGED_DRAG_COEFFICIENT;
    // static const float MAX_SATELLITE_SPEED;     
    // static const float MIN_RECOVERY_SPEED_FACTOR;
    // --- End Removed Constants --- 

    static const float BASE_ORBITAL_RADIUS;      
    static const float BASE_ANGULAR_SPEED;       
    static const float RESPAWN_DELAY;            
    static const float SPAWN_DURATION;           
    static const float CRASH_DURATION;           
    static const float MAX_FADE_AMOUNT;          
    static const float CHAOS_FORCE_SCALE;        
    static const float COLLISION_PROXIMITY;      // Renamed from DAMAGE_PROXIMITY

    // Added Spark Constants
    static const int   MIN_SPARKS_PER_COLLISION;
    static const int   MAX_SPARKS_PER_COLLISION;
    static const float SPARK_BASE_SPEED;
    static const float MIN_SPARK_LIFETIME;
    static const float MAX_SPARK_LIFETIME;
    static const uint8_t SPARK_BLEND_AMOUNT;

    // Helper methods
    void initializeSatellite(Satellite& sat);
    void updateSatellitePath(Satellite& sat, float dt, float chaosSetting);
    void applySatelliteInteractions(float chaosSetting);
    void renderSatellites();
    // void createVisualEffects(); // Keep commented out
    void updateSatelliteState(Satellite& sat, float dt);
};

} // namespace Scenes 