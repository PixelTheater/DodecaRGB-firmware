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
    static const int sphere_r = 317;  // Same radius as blob animation
    int16_t boid_id = 0;
    CRGB color = CRGB::White;
    
    // Use 3D vectors for position and velocity
    Vector3d pos;  // Always normalized (on sphere surface)
    Vector3d vel;  // Tangent to sphere at pos
    float max_speed;

    Boid(uint16_t unique_id, float speed_limit);
    void reset(float speed_limit);
    
    // Get spherical coordinates for LED mapping
    void getSphericalCoords(float& a, float& c) const;
    
    void applyForce(const Vector3d& force);
    void tick();

private:
    void limitSpeed();
    // Project velocity to be tangent to sphere at current position
    void constrainToSphere();
};

class BoidsAnimation : public Animation {
private:
    static const int sphere_r = Boid::sphere_r;  // Add sphere radius constant
    std::vector<std::unique_ptr<Boid>> boids;
    
    // Animation parameters
    float visual_range;      // How far boids can see
    float protected_range;   // Minimum distance between boids
    float centering_factor;  // Strength of flocking behavior
    float avoid_factor;      // Strength of collision avoidance
    float matching_factor;   // Strength of velocity matching
    float speed_limit;       // Maximum speed
    uint8_t fade_amount;     // How quickly trails fade
    
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
        p.setInt("num_boids", 70);             // Number of boids
        p.setFloat("visual_range", 2.0f);     // Visual range in radians
        p.setFloat("protected_range", 0.3f);  // Protection range
        p.setFloat("centering_factor", 0.15f);// Flocking strength
        p.setFloat("avoid_factor", 0.2f);     // Avoidance
        p.setFloat("matching_factor", 0.3f);  // Velocity matching
        p.setFloat("speed_limit", 8.0f);     // Speed limit
        p.setInt("fade", 60);                 // Trail fade rate
        p.setPalette("palette", RainbowColors_p);
        return p;
    }

    String getStatus() const override;
}; 