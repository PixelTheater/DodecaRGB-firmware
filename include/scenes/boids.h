#pragma once

#include "animation.h"
#include "palettes.h"
#include <vector>
#include <set>
#include <functional>
#include <queue>
#include "ArduinoEigen.h"

class Boid {
public:
    enum class State {
        FOLLOWING,  // Normal flocking behavior
        EXPLORING  // Independent exploration with random heading changes
    };

    static const int sphere_r = 317;  // Same radius as blob animation
    int16_t boid_id = 0;
    CRGB color = CRGB::White;
    
    // Use 3D vectors for position and velocity
    Vector3d pos;  // Always normalized (on sphere surface)
    Vector3d vel;  // Tangent to sphere at pos
    float max_speed;
    
    // Add timer and state
    State state = State::FOLLOWING;
    uint32_t state_timer = 0;
    static const uint32_t MIN_FOLLOW_TIME = 8000;   // 8 seconds minimum following
    static const uint32_t MAX_FOLLOW_TIME = 12000;  // 15 seconds maximum following
    static const uint32_t MIN_REST_TIME = 4000;     // 4 seconds minimum exploring
    static const uint32_t MAX_REST_TIME = 8000;     // 8 seconds maximum exploring

    // Add heading change timer
    uint32_t heading_change_timer = 0;
    static const uint32_t MIN_HEADING_TIME = 800;   // 0.8 seconds minimum between changes
    static const uint32_t MAX_HEADING_TIME = 2000;  // 2 seconds maximum between changes

    float chaos_factor = 0.3f;  // Add chaos factor with default

    Boid(uint16_t unique_id, float speed_limit);
    void reset(float speed_limit);
    
    // Get spherical coordinates for LED mapping
    void getSphericalCoords(float& a, float& c) const;
    
    void applyForce(const Vector3d& force);
    void tick();
    void updateState();  // New function to handle state changes

private:
    void limitSpeed();
    // Project velocity to be tangent to sphere at current position
    void constrainToSphere();
    void setRandomTimer();  // New helper function
};

class BoidsAnimation : public Animation {
private:
    static const int sphere_r = Boid::sphere_r;
    std::vector<std::unique_ptr<Boid>> boids;
    
    // Animation parameters
    float visual_range;      // How far boids can see
    float protected_range;   // Minimum distance between boids
    float centering_factor;  // Strength of flocking behavior
    float avoid_factor;      // Strength of collision avoidance
    float matching_factor;   // Strength of velocity matching
    float speed_limit;       // Maximum speed
    uint8_t fade_amount;     // How quickly trails fade
    float chaos_factor;      // 0-1: How likely boids are to explore vs follow
    uint8_t boid_size;      // Size of boid light spread in pixels
    float intensity;        // Overall brightness and blend intensity
    
    // Helper functions
    void updateBoid(Boid& boid);
    void drawBoid(const Boid& boid);
    float sphericalDistance(const Boid& b1, const Boid& b2) const;
    int findClosestLED(float a, float c) const;
    bool pointInTriangle(const Vector3d& p, const Vector3d& a, const Vector3d& b, const Vector3d& c) const;

public:
    void init(const AnimParams& params) override;
    void tick() override;
    const char* getName() const override { return "boids"; }
    
    AnimParams getDefaultParams() const override {
        AnimParams p;
        p.setInt("num_boids", 80);              // Number of boids in simulation [10-150]
        p.setFloat("visual_range", 0.90f);        // How far boids can see others, in radians [0.1-2.0]
        p.setFloat("protected_range", 0.75);    // Minimum distance between boids [0.05-0.5]
        p.setFloat("centering_factor", 0.10);   // How strongly boids move toward flock center [0.0-1.0]
        p.setFloat("avoid_factor", 0.45f);       // How strongly boids avoid each other [0.0-1.0]
        p.setFloat("matching_factor", 0.25);    // How strongly boids match neighbors' velocity [0.0-1.0]
        p.setFloat("speed_limit", 5.0f);         // Maximum movement speed per frame [1.0-15.0]
        p.setInt("fade", 25);                    // How quickly trails fade out [1-100]
        p.setFloat("chaos", 0.25f);               // Probability of random movement [0.0-1.0]
        p.setInt("size", 20);                    // Boid size as % of sphere radius [5-100]
        p.setFloat("intensity", 0.60f);          // LED brightness multiplier [0.0-1.0]
        p.setPalette("palette", OceanColors_p);  // Color palette for the boids
        return p;
    }

    String getStatus() const override;
}; 