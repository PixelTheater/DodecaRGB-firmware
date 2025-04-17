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

    void setup() override;
    void tick() override;
    std::string status() const;

    // Make sphere radius accessible, maybe calculate based on model?
    float sphere_radius = 300.0f; // Placeholder

    // Define default values used in setup()
    static constexpr int DEFAULT_NUM_BOIDS = 80;
    static constexpr float DEFAULT_VISUAL_RANGE = 0.90f;
    static constexpr float DEFAULT_PROTECTED_RANGE = 0.75f;
    static constexpr float DEFAULT_CENTERING_FACTOR = 0.10f;
    static constexpr float DEFAULT_AVOID_FACTOR = 0.45f;
    static constexpr float DEFAULT_MATCHING_FACTOR = 0.25f;
    static constexpr float DEFAULT_SPEED_LIMIT = 5.0f;
    static constexpr int DEFAULT_FADE = 25;
    static constexpr float DEFAULT_CHAOS = 0.25f;
    static constexpr int DEFAULT_SIZE = 20; // Size needs review
    static constexpr float DEFAULT_INTENSITY = 0.60f;

private:
    std::vector<std::unique_ptr<Boid>> boids;

    // Parameters are now stored in the 'settings' map from the base Scene class
    // float visual_range = 0.9f; // MOVED to settings
    // float protected_range = 0.75f; // MOVED to settings
    // float centering_factor = 0.1f; // MOVED to settings
    // float avoid_factor = 0.45f; // MOVED to settings
    // float matching_factor = 0.25f; // MOVED to settings
    // float speed_limit = 5.0f; // MOVED to settings
    // uint8_t fade_amount = 25; // MOVED to settings
    // float chaos_factor = 0.25f; // MOVED to settings
    // uint8_t boid_size = 20; // MOVED to settings
    // float intensity = 0.6f; // MOVED to settings

    void initBoids();
    void updateBoid(Boid& boid);
    void drawBoid(const Boid& boid);
    float sphericalDistance(const Boid& b1, const Boid& b2) const;
    void estimateSphereRadius(); // Add declaration

    friend class Boid; // Allow Boid to access Scene members

    // --- ADDED: Wrappers for Boid access ---
    float getRandomFloat(float min_val, float max_val) { return this->randomFloat(min_val, max_val); }
    uint32_t getRandom(uint32_t min_val, uint32_t max_val) { return this->random(min_val, max_val); }
    // --- END WRAPPERS ---
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

inline void BoidsScene::initBoids() {
    // TODO: Implement based on old boids.cpp:BoidsAnimation::init
    logInfo("BoidsScene::initBoids() called");
    boids.clear(); // Clear existing boids if any
    // Use implicit conversion from settings proxy, matching WanderingParticlesScene
    int num_boids_setting = settings["num_boids"]; 

    // --- Add buffer for logging ---
    char log_buffer[128]; 
    // --- End buffer ---
    snprintf(log_buffer, sizeof(log_buffer), "  Retrieved 'num_boids' setting: %d", (int)num_boids_setting);
    logInfo(log_buffer); // Log raw retrieved value

    float speed_limit_setting = settings["speed_limit"];
    float chaos_setting = settings["chaos"];

    // Add a safety check for the number of boids
    bool defaulted = false;
    if (num_boids_setting <= 0 || num_boids_setting > 1000) { // Set a reasonable upper limit
        snprintf(log_buffer, sizeof(log_buffer), "Invalid number of boids retrieved from settings: %d. Defaulting to %d", 
                 (int)num_boids_setting, DEFAULT_NUM_BOIDS);
        logError(log_buffer);
        num_boids_setting = DEFAULT_NUM_BOIDS;
        defaulted = true;
    }
    snprintf(log_buffer, sizeof(log_buffer), "  Value after safety check: %d %s", (int)num_boids_setting, defaulted ? "(defaulted)" : "");
    logInfo(log_buffer);

    boids.reserve(num_boids_setting);
    for (int i = 0; i < num_boids_setting; ++i) {
        auto boid = std::make_unique<Boid>(*this, i, speed_limit_setting, chaos_setting);
        // Assign color from palette using PixelTheater::CHSV and hsv2rgb_rainbow
        // Mimics FastLED ColorFromPalette behavior for a standard rainbow
        uint8_t hue = i * (255 / num_boids_setting);
        boid->color = PixelTheater::CHSV(hue, 240, 255); // Use CHSV constructor, then implicit conversion
        boids.push_back(std::move(boid));
    }
    // Explicitly cast to int for the logger
    snprintf(log_buffer, sizeof(log_buffer), "BoidsScene::initBoids() complete, created %d boids", (int)num_boids_setting);
    logInfo(log_buffer);
}

inline void BoidsScene::updateBoid(Boid& boid) {
    // Get parameters from settings
    float visual_range_rad = settings["visual_range"];
    float protected_range_rad = settings["protected_range"];
    float centering_factor = settings["centering_factor"];
    float avoid_factor = settings["avoid_factor"];
    float matching_factor = settings["matching_factor"];

    // Accumulate forces directly
    Vector3f total_separation_force = Vector3f::Zero(); 
    Vector3f alignment_force = Vector3f::Zero(); // Store desired velocity alignment
    Vector3f cohesion_force = Vector3f::Zero();  // Store vector towards center of mass

    Vector3f center_of_mass = Vector3f::Zero();
    Vector3f average_velocity = Vector3f::Zero();
    int visual_neighbors = 0;

    for (const auto& other_boid_ptr : boids) {
        const Boid& other_boid = *other_boid_ptr;
        if (&boid == &other_boid) continue; // Skip self

        float dist_rad = sphericalDistance(boid, other_boid);

        if (dist_rad < visual_range_rad) {
            visual_neighbors++;
            center_of_mass += other_boid.pos;
            average_velocity += other_boid.vel;

            // Accumulate separation force for each neighbor in protected range
            if (dist_rad < protected_range_rad && dist_rad > 1e-6f) {
                Vector3f away_vec = boid.pos - other_boid.pos;
                // Add force directly, scaled by inverse distance and factor
                total_separation_force += (away_vec.normalized() / dist_rad) * avoid_factor;
                // Note: No longer dividing by protected_neighbors later
            }
        }
    }

    // Calculate alignment and cohesion forces (remain averaged)
    if (visual_neighbors > 0) {
        average_velocity /= visual_neighbors;
        alignment_force = (average_velocity - boid.vel) * matching_factor;

        center_of_mass /= visual_neighbors;
        cohesion_force = (center_of_mass - boid.pos) * centering_factor;
    }

    // Combine forces 
    // Separation force is already scaled by avoid_factor during accumulation
    Vector3f total_force = total_separation_force + alignment_force + cohesion_force;
    boid.applyForce(total_force);
}

inline void BoidsScene::drawBoid(const Boid& boid) {
    // Revert to simple: Find closest LED and light it up.
    // Ignore 'size' parameter for now.
    float min_dist_sq = 1e18f; // Use squared distance
    int closest_led_index = -1;
    size_t num_leds = this->ledCount(); // Use base class helper

    // --- Add buffer for logging ---
    char log_buffer[128]; 
    // --- End buffer ---

    // Check if num_leds is valid before proceeding
    if (num_leds == 0) {
        logError("BoidsScene::drawBoid: Cannot draw, ledCount() is zero.");
        return; // Cannot proceed without LEDs
    }

    for (size_t i = 0; i < num_leds; ++i) {
        const auto& point = this->model().point(i); // Use base class helper
        float dx = point.x() - boid.pos.x();
        float dy = point.y() - boid.pos.y();
        float dz = point.z() - boid.pos.z();
        float dist_sq = dx * dx + dy * dy + dz * dz;

        if (dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            closest_led_index = static_cast<int>(i);
        }
    }

    if (closest_led_index >= 0 && static_cast<size_t>(closest_led_index) < num_leds) {
        // Blend the boid's color onto the closest LED
        float intensity_setting = settings["intensity"]; // Get intensity from settings
        uint8_t blend_amount = static_cast<uint8_t>(intensity_setting * 255.0f); // Scale intensity to 0-255
        PixelTheater::nblend(leds[closest_led_index], boid.color, blend_amount); // Use base class leds[] proxy and qualified nblend
    } else {
        // Log error if closest LED is invalid
        if (closest_led_index >= static_cast<int>(num_leds)) { // Check bounds specifically
            snprintf(log_buffer, sizeof(log_buffer), "BoidsScene::drawBoid: Closest LED index %d out of bounds (%zu)", closest_led_index, num_leds);
            logError(log_buffer);
        } else { // Handle other cases (e.g., closest_led_index remained -1)
            snprintf(log_buffer, sizeof(log_buffer), "BoidsScene::drawBoid: Could not find closest LED for boid %u", boid.boid_id);
            logError(log_buffer);
        }
    }
}

inline float BoidsScene::sphericalDistance(const Boid& b1, const Boid& b2) const {
    // TODO: Implement using Eigen vectors
    Vector3f dir1 = b1.pos.normalized();
    Vector3f dir2 = b2.pos.normalized();
    // Ensure dot product is within [-1, 1] for acos
    float dot = std::clamp(dir1.dot(dir2), -1.0f, 1.0f);
    return acos(dot); // Returns angle in radians
}

// --- Boid Class Implementation placeholders ---

inline Boid::Boid(BoidsScene& parent_scene, uint16_t unique_id, float speed_limit, float initial_chaos)
    : scene(parent_scene), boid_id(unique_id), max_speed(speed_limit), chaos_factor(initial_chaos) {
    reset();
}

inline void Boid::reset() {
    // Based on old boids.cpp:Boid::reset
    state = State::FOLLOWING;
    setRandomTimer();

    // Log the radius being used via the scene reference
    scene.logInfo("Boid %d reset: Using radius %.2f", boid_id, scene.sphere_radius); 

    // Use random direction on the sphere for initial position
    // scene.randomFloat should be accessible via the scene reference
    Vector3f random_dir(scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f));
    if (random_dir.norm() > 1e-6f) { // Avoid zero vector
        pos = random_dir.normalized() * scene.sphere_radius;
    }

    // Random initial velocity tangent to sphere
    Vector3f random_dir2(scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f));
    vel = pos.normalized().cross(random_dir2); // Calculate tangent vector
    if (vel.squaredNorm() < 1e-6f) { // Avoid zero vector if random_dir is parallel to pos
        // Find an arbitrary perpendicular vector if cross product fails
        Vector3f tangent;
        // Check alignment with axes to find a non-parallel vector
        if (std::abs(pos.normalized().dot(Vector3f::UnitX())) < 0.9f) {
            tangent = pos.normalized().cross(Vector3f::UnitX());
        } else {
            tangent = pos.normalized().cross(Vector3f::UnitY());
        }
        vel = tangent;
    }
    vel.normalize();
    vel *= max_speed; // Start at max speed
    constrainToSphere(); // Ensure velocity is perfectly tangent initially
}

inline void Boid::applyForce(const Vector3f& force) {
    // Based on old boids.cpp:Boid::applyForce
    vel += force;
    // Speed limiting and tangent constraint happens in tick()
}

inline void Boid::tick() {
    // Based on old boids.cpp:Boid::tick
    updateState(); // Check for state changes

    // Update position based on velocity
    pos += vel;
    pos.normalize(); // Project back onto unit sphere
    pos *= scene.sphere_radius; // Scale to actual radius

    // Keep velocity tangent to the new position and limit speed
    constrainToSphere();
    limitSpeed();
}

inline void Boid::updateState() {
    state_timer -= static_cast<uint32_t>(scene.deltaTime() * 1000.0f); // Use scene member directly
    heading_change_timer -= static_cast<uint32_t>(scene.deltaTime() * 1000.0f); // Use scene member directly

    if (state == State::FOLLOWING) {
        if (state_timer <= 0) {
            state = State::EXPLORING;
            // scene.random should be accessible
            state_timer = scene.getRandom(MIN_REST_TIME, MAX_REST_TIME);
            vel = Vector3f::Zero(); // Stop when starting to explore
        } else if (heading_change_timer <= 0) {
            // Introduce random heading change while following
            // Use random direction and chaos factor
            Vector3f random_dir(scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f), scene.getRandomFloat(-1.0f, 1.0f));
            if (random_dir.norm() > 1e-6f) {
                Vector3f chaos_force = random_dir.normalized() * chaos_factor * 0.1f; // Reduced chaos effect
                applyForce(chaos_force);
            }
            // Reset heading timer
            heading_change_timer = scene.getRandom(MIN_HEADING_TIME, MAX_HEADING_TIME);
        }
    } else { // State::EXPLORING
        if (state_timer <= 0) {
            state = State::FOLLOWING;
            // Reset timers for following state
            state_timer = scene.getRandom(MIN_FOLLOW_TIME, MAX_FOLLOW_TIME);
            heading_change_timer = scene.getRandom(MIN_HEADING_TIME, MAX_HEADING_TIME);
            // Give a small initial velocity boost
            Vector3f boost_dir = pos.normalized(); // Move away from center initially
            vel = boost_dir * (max_speed * 0.1f);
        }
        // No heading changes during EXPLORING state (just sit still)
    }
}

inline void Boid::limitSpeed() {
    // Based on old boids.cpp:Boid::limitSpeed
    float speed_sq = vel.squaredNorm();
    float max_speed_sq = max_speed * max_speed;
    if (speed_sq > max_speed_sq && speed_sq > 1e-9f) { // Avoid division by zero/nan
        vel *= (max_speed / sqrt(speed_sq));
    }
}

inline void Boid::constrainToSphere() {
    // Based on old boids.cpp:Boid::constrainToSphere
    // Project velocity vector onto the plane tangent to the sphere at the current position
    Vector3f norm_pos = pos.normalized();
    vel -= vel.dot(norm_pos) * norm_pos;
}

inline void Boid::setRandomTimer() {
    // Based on old boids.cpp:Boid::setRandomTimer
    uint32_t now = scene.millis(); // Use scene's time access
    if (state == State::FOLLOWING) {
        state_timer = now + scene.random(MIN_FOLLOW_TIME, MAX_FOLLOW_TIME);
    } else { // EXPLORING
        state_timer = now + scene.random(MIN_REST_TIME, MAX_REST_TIME);
        // Also reset heading change timer when entering EXPLORING state
        heading_change_timer = now + scene.random(MIN_HEADING_TIME, MAX_HEADING_TIME);
    }
}

// --- Implementation placeholders (will go in a .cpp file eventually) ---

inline void BoidsScene::setup() {
    set_name("Boids");
    set_description("Flocking simulation on a sphere");
    set_version("1.0");
    set_author("PixelTheater Team (Refactored)");

    // Estimate radius based on model
    estimateSphereRadius(); // Call the estimation function

    // Define parameters using the base class method
    param("num_boids", "count", 10, 200, DEFAULT_NUM_BOIDS, "clamp", "Number of boids");
    param("visual_range", "range", 0.1f, 2.0f, DEFAULT_VISUAL_RANGE, "clamp", "Boid sight distance (radians)");
    param("protected_range", "range", 0.05f, 1.0f, DEFAULT_PROTECTED_RANGE, "clamp", "Min distance between boids (radians)");
    param("centering_factor", "range", 0.0f, 1.0f, DEFAULT_CENTERING_FACTOR, "clamp", "Flock centering strength");
    param("avoid_factor", "range", 0.0f, 1.0f, DEFAULT_AVOID_FACTOR, "clamp", "Collision avoidance strength");
    param("matching_factor", "range", 0.0f, 1.0f, DEFAULT_MATCHING_FACTOR, "clamp", "Velocity matching strength");
    param("speed_limit", "range", 1.0f, 15.0f, DEFAULT_SPEED_LIMIT, "clamp", "Max boid speed");
    param("fade", "count", 1, 100, DEFAULT_FADE, "clamp", "Trail fade amount");
    param("chaos", "range", 0.0f, 1.0f, DEFAULT_CHAOS, "clamp", "Probability of random movement");
    // Corrected size parameter description and range based on original comments
    param("size", "count", 5, 100, DEFAULT_SIZE, "clamp", "Boid light spread size (% radius?)"); 
    param("intensity", "range", 0.1f, 1.0f, DEFAULT_INTENSITY, "clamp", "LED brightness multiplier");
    // Note: Original had palette parameter, hardcoded here for now

    initBoids(); // Call initialization after params are set
}

// Add implementation for estimateSphereRadius
inline void BoidsScene::estimateSphereRadius() {
    float max_dist_sq = 0.0f;
    size_t count = model().pointCount();
    if (count == 0) { 
        logWarning("Cannot estimate sphere radius: No points in model. Using default: %.1f", sphere_radius);
        return; 
    }
    // Find the point furthest from the origin (0,0,0)
    for (size_t i = 0; i < count; i++) { 
        const auto& p = model().point(i);
        // Assuming point coordinates are floats now
        float dist_sq = p.x() * p.x() + p.y() * p.y() + p.z() * p.z(); 
        max_dist_sq = std::max(max_dist_sq, dist_sq);
    }
    if (max_dist_sq > 1e-6f) { // Check against small epsilon for floating point
        sphere_radius = sqrt(max_dist_sq);
        logInfo("Estimated sphere radius: %.2f", sphere_radius);
    } else {
        logWarning("Could not estimate sphere radius (max_dist_sq=%.2f), using default: %.1f", max_dist_sq, sphere_radius);
        // Keep the default sphere_radius if estimation fails
    }
}

inline void BoidsScene::tick() {
    Scene::tick(); // Call base class tick first

    uint8_t fade_amount = settings["fade"];

    size_t count = ledCount();
    for (size_t i = 0; i < count; ++i) {
        // Use the CRGB method for fading
        leds[i].fadeToBlackBy(fade_amount); 
    }

    BENCHMARK_START("boid_update");
    for (auto& boid : boids) {
        updateBoid(*boid); // Calculate forces
        boid->tick();      // Apply velocity, constrain
    }
    BENCHMARK_END();

    BENCHMARK_START("boid_draw");
    for (const auto& boid : boids) {
        drawBoid(*boid);
    }
    BENCHMARK_END();
}

} // namespace Scenes 