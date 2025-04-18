#pragma once

#include "PixelTheater.h"
#include "benchmark.h"

#include <vector>
#include <memory>
#include <cmath>
#include <string>

// Use shorter Eigen types
using Vector3f = Eigen::Vector3f;

namespace Scenes {

// Forward declare the Boid class
class Boid;

class BoidsScene : public PixelTheater::Scene {
public:
    BoidsScene() = default;

    // Virtual methods - declarations only
    void setup() override;
    void tick() override;
    std::string status() const override;

    // Make sphere radius accessible, maybe calculate based on model?
    float sphere_radius = 300.0f; // Placeholder

    // Define default values used in setup()
    static constexpr int DEFAULT_NUM_BOIDS = 80;
    static constexpr float DEFAULT_VISUAL_RANGE = 0.40f;
    static constexpr float DEFAULT_PROTECTED_RANGE = 0.35f;
    static constexpr float DEFAULT_CENTERING_FACTOR = 0.10f;
    static constexpr float DEFAULT_AVOID_FACTOR = 0.75f;
    static constexpr float DEFAULT_MATCHING_FACTOR = 0.10f;
    static constexpr float DEFAULT_SPEED_LIMIT = 6.0f;
    static constexpr int DEFAULT_FADE = 30;
    static constexpr float DEFAULT_CHAOS = 0.55f;
    static constexpr float DEFAULT_INTENSITY = 0.60f;

private:
    std::vector<std::unique_ptr<Boid>> boids;

    // Add members to store last used parameter values
    int last_num_boids = -1; // Initialize to ensure first check triggers update
    float last_speed_limit = -1.0f;
    float last_chaos_factor = -1.0f;

    // Helper methods - declarations only
    void initBoids();
    void updateBoid(Boid& boid);
    void drawBoid(const Boid& boid);
    float sphericalDistance(const Boid& b1, const Boid& b2) const;
    void estimateSphereRadius();

    friend class Boid; // Allow Boid to access Scene members

    // Wrappers for Boid access
    float getRandomFloat(float min_val, float max_val) { return this->randomFloat(min_val, max_val); }
    uint32_t getRandom(uint32_t min_val, uint32_t max_val) { return this->random(min_val, max_val); }
};

// Define the Boid class within the same header for now
class Boid {
public:
    enum class State {
        FOLLOWING,
        EXPLORING
    };

    BoidsScene& scene; // Reference to the parent scene
    uint16_t boid_id = 0;
    PixelTheater::CRGB color = PixelTheater::CRGB::White;

    Vector3f pos; // Current position (Cartesian)
    Vector3f vel; // Current velocity (Cartesian)
    float max_speed;

    State state = State::FOLLOWING;
    uint32_t state_timer = 0;
    uint32_t heading_change_timer = 0;
    float chaos_factor = 0.3f; // Passed from scene settings

    // Constants for state timing (can be adjusted later)
    static constexpr uint32_t MIN_FOLLOW_TIME = 8000;
    static constexpr uint32_t MAX_FOLLOW_TIME = 12000;
    static constexpr uint32_t MIN_REST_TIME = 4000;
    static constexpr uint32_t MAX_REST_TIME = 8000;
    static constexpr uint32_t MIN_HEADING_TIME = 800;
    static constexpr uint32_t MAX_HEADING_TIME = 2000;

    Boid(BoidsScene& parent_scene, uint16_t unique_id, float speed_limit, float initial_chaos);
    void reset();

    // Position/velocity update logic
    void applyForce(const Vector3f& force);
    void tick();
    void updateState();

private:
    void limitSpeed();
    void constrainToSphere(); // Keep velocity tangent to sphere
    void setRandomTimer();
};

// --- Boid Class Implementation placeholders (inlined below for now) ---
// It's generally better practice to move Boid implementations to a separate Boid.cpp file as well,
// but keeping them here for simplicity as they don't involve virtual methods causing vtable issues.

inline Boid::Boid(BoidsScene& parent_scene, uint16_t unique_id, float speed_limit, float initial_chaos)
    : scene(parent_scene), boid_id(unique_id), max_speed(speed_limit), chaos_factor(initial_chaos) {
    reset();
}

inline void Boid::reset() {
    state = State::FOLLOWING;
    setRandomTimer();

    scene.logInfo("Boid %d reset: Using radius %.2f", boid_id, scene.sphere_radius); 

    Vector3f random_dir(scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f));
    if (random_dir.norm() > 1e-6f) { 
        pos = random_dir.normalized() * scene.sphere_radius;
    }

    Vector3f random_dir2(scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f));
    vel = pos.normalized().cross(random_dir2);
    if (vel.squaredNorm() < 1e-6f) { 
        Vector3f tangent;
        if (std::abs(pos.normalized().dot(Vector3f::UnitX())) < 0.9f) {
            tangent = pos.normalized().cross(Vector3f::UnitX());
        } else {
            tangent = pos.normalized().cross(Vector3f::UnitY());
        }
        vel = tangent;
    }
    vel.normalize();
    vel *= max_speed; 
    constrainToSphere(); 
}

inline void Boid::applyForce(const Vector3f& force) {
    vel += force;
}

inline void Boid::tick() {
    updateState();

    pos += vel;
    pos.normalize();
    pos *= scene.sphere_radius;

    constrainToSphere();
    limitSpeed();
}

inline void Boid::updateState() {
    state_timer -= static_cast<uint32_t>(scene.deltaTime() * 1000.0f);
    heading_change_timer -= static_cast<uint32_t>(scene.deltaTime() * 1000.0f);

    if (state == State::FOLLOWING) {
        if (state_timer <= 0) {
            state = State::EXPLORING;
            state_timer = scene.getRandom(MIN_REST_TIME, MAX_REST_TIME);
            vel = Vector3f::Zero();
        } else if (heading_change_timer <= 0) {
            Vector3f random_dir(scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f));
            if (random_dir.norm() > 1e-6f) {
                Vector3f chaos_force = random_dir.normalized() * chaos_factor * 0.2f;
                applyForce(chaos_force);
            }
            heading_change_timer = scene.getRandom(MIN_HEADING_TIME, MAX_HEADING_TIME);
        }
    } else { // State::EXPLORING
        if (state_timer <= 0) {
            state = State::FOLLOWING;
            state_timer = scene.getRandom(MIN_FOLLOW_TIME, MAX_FOLLOW_TIME);
            heading_change_timer = scene.getRandom(MIN_HEADING_TIME, MAX_HEADING_TIME);
            Vector3f boost_dir = pos.normalized();
            vel = boost_dir * (max_speed * 0.1f);
        }
    }
}

inline void Boid::limitSpeed() {
    float speed_sq = vel.squaredNorm();
    float max_speed_sq = max_speed * max_speed;
    if (speed_sq > max_speed_sq && speed_sq > 1e-9f) { 
        vel *= (max_speed / sqrt(speed_sq));
    }
}

inline void Boid::constrainToSphere() {
    Vector3f norm_pos = pos.normalized();
    vel -= vel.dot(norm_pos) * norm_pos;
}

inline void Boid::setRandomTimer() {
    uint32_t now = scene.millis(); 
    if (state == State::FOLLOWING) {
        state_timer = now + scene.random(MIN_FOLLOW_TIME, MAX_FOLLOW_TIME);
    } else { // EXPLORING
        state_timer = now + scene.random(MIN_REST_TIME, MAX_REST_TIME);
        heading_change_timer = now + scene.random(MIN_HEADING_TIME, MAX_HEADING_TIME);
    }
}

} // namespace Scenes 