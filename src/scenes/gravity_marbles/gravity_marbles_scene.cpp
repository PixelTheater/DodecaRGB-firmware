#include "gravity_marbles_scene.h"
#include <cmath>
#include <algorithm>

namespace Scenes {

// Constants
constexpr float MIN_VELOCITY_THRESHOLD = 0.01f;
constexpr int COLLISION_ITERATIONS = 2; // Iterate collision resolver multiple times for stability

void GravityMarblesScene::setup() {
    set_name("Gravity Marbles");
    set_description("Marbles rolling around under the influence of gravity.");
    set_author("PixelTheater");
    set_version("1.0");

    param("population", "count", 1, 100, 10, "", "Number of marbles");
    param("marble_size", "ratio", 0.01f, 0.6f, 0.35f, "", "Size of each marble");
    param("gravity_strength", "range", 0.0f, 100.0f, 65.0f, "", "Strength of gravity");
    param("damping", "ratio", 0.01f, 1.0f, 0.1f, "", "How quickly marbles lose velocity");
    param("elasticity", "ratio", 0.1f, 1.0f, 0.50f, "", "Bounciness of collisions");
    param("trails", "ratio", 0.1f, 1.0f, 0.7f, "", "Length of marble trails (fade amount)");
    param("collisions", "switch", true, "", "Enable/disable collisions");
    param("inertia", "range", 0.0f, 200.0f, 75.0f, "", "Inertial force from movement");

    // Input parameters for IMU data. These will be set externally from main.cpp
    // They define the direction of the gravity vector.
    param("gravity_x", "signed_ratio", 0.0f, "", "Gravity vector X component");
    param("gravity_y", "signed_ratio", 0.0f, "", "Gravity vector Y component");
    param("gravity_z", "signed_ratio", -1.0f, "", "Gravity vector Z component");
    // Parameters for linear acceleration
    param("lin_accel_x", "range", -100.0f, 100.0f, 0.0f, "", "Linear Accel X");
    param("lin_accel_y", "range", -100.0f, 100.0f, 0.0f, "", "Linear Accel Y");
    param("lin_accel_z", "range", -100.0f, 100.0f, 0.0f, "", "Linear Accel Z");

    initializeMarbles();
}

void GravityMarblesScene::tick() {
    Scene::tick();
    const float dt = deltaTime();

    // Check if population changed and re-initialize if needed
    int new_population = settings["population"];
    if (new_population != population_) {
        population_ = new_population;
        initializeMarbles();
    }
    
    // Cache other parameters for use in this frame
    marble_size_ = settings["marble_size"];
    gravity_strength_ = settings["gravity_strength"];
    damping_ = settings["damping"];
    elasticity_ = settings["elasticity"];
    trail_amount_ = 255 * (1.0f - static_cast<float>(settings["trails"]));
    inertia_ = settings["inertia"];

    // Update gravity direction from parameters
    gravity_.x() = settings["gravity_x"];
    gravity_.y() = settings["gravity_y"];
    gravity_.z() = settings["gravity_z"];
    if (gravity_.squaredNorm() > 1e-6) {
        gravity_.normalize();
    }
    // Update linear acceleration from parameters
    lin_accel_.x() = settings["lin_accel_x"];
    lin_accel_.y() = settings["lin_accel_y"];
    lin_accel_.z() = settings["lin_accel_z"];

    // Apply fade to the whole display to create trails
    for (size_t i = 0; i < ledCount(); ++i) {
        leds[i].fadeToBlackBy(trail_amount_);
    }

    // Run the simulation steps
    updatePhysics(dt);
    if (settings["collisions"]) {
        for (int i=0; i < COLLISION_ITERATIONS; ++i) {
            handleCollisions();
        }
    }
    // renderGravityIndicator();
    renderMarbles();
}

void GravityMarblesScene::initializeMarbles() {
    marbles_.clear();
    population_ = settings["population"];
    marbles_.resize(population_);

    float sphere_radius = model().getSphereRadius();
    if (sphere_radius < 1e-3) sphere_radius = 100.0f; // Fallback

    const auto& palette = PixelTheater::Palettes::PartyColors;

    for (int i = 0; i < population_; ++i) {
        auto& m = marbles_[i];
        m.id = next_id_++;
        
        // Generate a random point on the sphere
        Eigen::Vector3f p(randomFloat(-1, 1), randomFloat(-1, 1), randomFloat(-1, 1));
        m.position = p.normalized() * sphere_radius;
        
        m.velocity = Eigen::Vector3f::Zero();
        m.radius = static_cast<float>(settings["marble_size"]) * sphere_radius;
        m.color = PixelTheater::colorFromPalette(palette, random8());
    }
}

void GravityMarblesScene::updatePhysics(float dt) {
    if (dt <= 0) return;
    
    const float sphere_radius = model().getSphereRadius();
    const Eigen::Vector3f gravity_force = gravity_ * gravity_strength_;
    // The force from device acceleration should be opposite to the acceleration
    const Eigen::Vector3f inertial_force = -lin_accel_ * inertia_;
    const Eigen::Vector3f total_force = gravity_force + inertial_force;

    for (auto& m : marbles_) {
        // Project gravity onto the sphere's tangent plane at the marble's position
        Eigen::Vector3f pos_normal = m.position.normalized();
        Eigen::Vector3f tangent_force = total_force - pos_normal * pos_normal.dot(total_force);

        // Apply forces
        m.velocity += tangent_force * dt;

        // Apply damping
        m.velocity *= (1.0f - damping_ * dt);

        // Update position
        m.position += m.velocity * dt;
        m.position = m.position.normalized() * sphere_radius;

        // Re-project velocity onto the new tangent plane to keep it tangential
        pos_normal = m.position.normalized();
        m.velocity = m.velocity - pos_normal * pos_normal.dot(m.velocity);
        
        // Clamp to prevent runaway speeds (optional, but good for stability)
        float max_speed = sphere_radius * 2.0f; // Heuristic max speed
        if (m.velocity.squaredNorm() > max_speed * max_speed) {
            m.velocity = m.velocity.normalized() * max_speed;
        }
    }
}

void GravityMarblesScene::handleCollisions() {
    const float sphere_radius = model().getSphereRadius();
    float marble_radius_world = static_cast<float>(settings["marble_size"]) * sphere_radius;

    for (auto& m : marbles_) {
        m.radius = marble_radius_world;
    }

    for (size_t i = 0; i < marbles_.size(); ++i) {
        for (size_t j = i + 1; j < marbles_.size(); ++j) {
            auto& m1 = marbles_[i];
            auto& m2 = marbles_[j];

            Eigen::Vector3f axis = m1.position - m2.position;
            float dist_sq = axis.squaredNorm();
            float min_dist = (m1.radius + m2.radius)/1.5;

            if (dist_sq < min_dist * min_dist && dist_sq > 1e-6) {
                float dist = sqrt(dist_sq);
                Eigen::Vector3f normal = axis / dist;
                float overlap = min_dist - dist;

                // 1. Resolve overlap by pushing marbles apart
                m1.position += normal * overlap * 0.5f;
                m2.position -= normal * overlap * 0.5f;

                // 2. Elastic collision response (momentum exchange)
                Eigen::Vector3f v1_normal = m1.velocity.dot(normal) * normal;
                Eigen::Vector3f v2_normal = m2.velocity.dot(normal) * normal;
                Eigen::Vector3f v1_tangent = m1.velocity - v1_normal;
                Eigen::Vector3f v2_tangent = m2.velocity - v2_normal;

                m1.velocity = v1_tangent + (v1_normal * -1.0f + v2_normal * 2.0f) * 0.5f * elasticity_;
                m2.velocity = v2_tangent + (v2_normal * -1.0f + v1_normal * 2.0f) * 0.5f * elasticity_;
            }
        }
    }
}

void GravityMarblesScene::renderGravityIndicator() {
    if (gravity_.squaredNorm() < 1e-6f) return;

    // The gravity vector points "down". We want a dot at the point of strongest influence.
    Eigen::Vector3f gravity_dir = gravity_.normalized();

    // Render a small, white circle at this position.
    float indicator_angular_size = 0.05f; // small dot in radians
    const float cos_render_angle = std::cos(indicator_angular_size);
    CRGB indicator_color = CRGB::White;

    for (size_t i = 0; i < ledCount(); ++i) {
        const auto& p = model().point(i);
        Eigen::Vector3f led_dir(p.x(), p.y(), p.z());
        led_dir.normalize();

        // Check if the LED is within the cone of the indicator spot
        float dot = led_dir.dot(gravity_dir);
        if (dot > cos_render_angle) {
            // Use a quadratic falloff for a soft-edged dot
            float falloff = (dot - cos_render_angle) / (1.0f - cos_render_angle);
            uint8_t blend_amount = static_cast<uint8_t>(falloff * falloff * 255);
            nblend(leds[i], indicator_color, blend_amount);
        }
    }
}

void GravityMarblesScene::renderMarbles() {
    size_t num_leds = ledCount();
    if (num_leds == 0) return;

    const float sphere_radius = model().getSphereRadius();
    float angular_size = settings["marble_size"]; // Use ratio directly as angular size
    const float cos_render_angle = std::cos(angular_size);

    for (const auto& m : marbles_) {
        if (m.position.squaredNorm() < 1e-6f) continue;

        Eigen::Vector3f marble_dir = m.position.normalized();

        for (size_t i = 0; i < num_leds; ++i) {
            const auto& p = model().point(i);
            Eigen::Vector3f led_dir(p.x(), p.y(), p.z());
            led_dir.normalize();

            float dot = marble_dir.dot(led_dir);
            if (dot > cos_render_angle) {
                // Calculate falloff for a soft edge
                float falloff = (dot - cos_render_angle) / (1.0f - cos_render_angle);
                falloff = std::max(0.0f, std::min(1.0f, falloff * falloff));
                
                uint8_t blend_amount = static_cast<uint8_t>(falloff * 200);
                PixelTheater::nblend(leds[i], m.color, blend_amount);
            }
        }
    }
}

std::string GravityMarblesScene::status() const {
    if (marbles_.empty()) {
        return "No marbles.";
    }
    const auto& m = marbles_[0];
    char buffer[150];
    snprintf(buffer, sizeof(buffer), "Pop:%d | M[0] Vel:%.2f",
             population_, m.velocity.norm());
    return std::string(buffer);
}

} // namespace Scenes 