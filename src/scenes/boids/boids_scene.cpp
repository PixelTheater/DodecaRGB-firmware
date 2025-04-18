#include "boids_scene.h"
#include <memory> // For std::make_unique
#include <cmath> // For std::clamp, std::sqrt, std::acos
#include <cstdio> // For snprintf

// Use shorter Eigen types (already in header, but good practice here too?)
using Vector3f = Eigen::Vector3f;

namespace Scenes {

// --- BoidsScene Method Implementations ---

void BoidsScene::setup() {
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
    param("intensity", "range", 0.1f, 1.0f, DEFAULT_INTENSITY, "clamp", "LED brightness multiplier");

    initBoids(); // Call initialization after params are set
}

void BoidsScene::estimateSphereRadius() {
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

void BoidsScene::initBoids() {
    logInfo("BoidsScene::initBoids() called");
    boids.clear(); // Clear existing boids if any
    int num_boids_setting = settings["num_boids"]; 

    char log_buffer[128]; 
    snprintf(log_buffer, sizeof(log_buffer), "  Retrieved 'num_boids' setting: %d", (int)num_boids_setting);
    logInfo(log_buffer); 

    float speed_limit_setting = settings["speed_limit"];
    float chaos_setting = settings["chaos"];

    bool defaulted = false;
    if (num_boids_setting <= 0 || num_boids_setting > 1000) { 
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
        uint8_t palette_index = i * 255 / num_boids_setting;
        boid->color = PixelTheater::colorFromPalette(PixelTheater::Palettes::OceanColors, palette_index);
        boids.push_back(std::move(boid));
    }

    last_num_boids = num_boids_setting;
    last_speed_limit = speed_limit_setting;
    last_chaos_factor = chaos_setting;

    snprintf(log_buffer, sizeof(log_buffer), "BoidsScene::initBoids() complete, created %d boids", (int)num_boids_setting);
    logInfo(log_buffer);
}

void BoidsScene::tick() {
    Scene::tick(); // Call base class tick first

    // --- Check for parameter changes --- 
    int current_num_boids = settings["num_boids"];
    float current_speed_limit = settings["speed_limit"];
    float current_chaos_factor = settings["chaos"];

    if (current_num_boids != last_num_boids) {
        logInfo("num_boids changed (%d -> %d), re-initializing.", last_num_boids, current_num_boids);
        initBoids(); 
    } else {
        if (current_speed_limit != last_speed_limit) {
            logInfo("speed_limit changed (%.2f -> %.2f), updating boids.", last_speed_limit, current_speed_limit);
            for (auto& boid : boids) {
                boid->max_speed = current_speed_limit;
            }
            last_speed_limit = current_speed_limit;
        }
        if (current_chaos_factor != last_chaos_factor) {
             logInfo("chaos_factor changed (%.2f -> %.2f), updating boids.", last_chaos_factor, current_chaos_factor);
            for (auto& boid : boids) {
                boid->chaos_factor = current_chaos_factor;
            }
            last_chaos_factor = current_chaos_factor;
        }
    }
    // --- End parameter change check ---

    uint8_t fade_amount = settings["fade"];

    size_t count = ledCount();
    for (size_t i = 0; i < count; ++i) {
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

void BoidsScene::updateBoid(Boid& boid) {
    float visual_range_rad = settings["visual_range"];
    float protected_range_rad = settings["protected_range"];
    float centering_factor = settings["centering_factor"];
    float avoid_factor = settings["avoid_factor"];
    float matching_factor = settings["matching_factor"];

    Vector3f total_separation_force = Vector3f::Zero(); 
    Vector3f alignment_force = Vector3f::Zero(); 
    Vector3f cohesion_force = Vector3f::Zero();  

    Vector3f center_of_mass = Vector3f::Zero();
    Vector3f average_velocity = Vector3f::Zero();
    int visual_neighbors = 0;

    for (const auto& other_boid_ptr : boids) {
        const Boid& other_boid = *other_boid_ptr;
        if (&boid == &other_boid) continue; 

        float dist_rad = sphericalDistance(boid, other_boid);

        if (dist_rad < visual_range_rad) {
            visual_neighbors++;
            center_of_mass += other_boid.pos;
            average_velocity += other_boid.vel;

            if (dist_rad < protected_range_rad && dist_rad > 1e-6f) {
                Vector3f away_vec = boid.pos - other_boid.pos;
                total_separation_force += (away_vec.normalized() / dist_rad) * avoid_factor;
            }
        }
    }

    if (visual_neighbors > 0) {
        average_velocity /= visual_neighbors;
        alignment_force = (average_velocity - boid.vel) * matching_factor;

        center_of_mass /= visual_neighbors;
        cohesion_force = (center_of_mass - boid.pos) * centering_factor;
    }

    Vector3f total_force = total_separation_force + alignment_force + cohesion_force;
    boid.applyForce(total_force);
}

void BoidsScene::drawBoid(const Boid& boid) {
    float min_dist_sq = 1e18f; 
    int closest_led_index = -1;
    size_t num_leds = this->ledCount(); 

    char log_buffer[128]; 

    if (num_leds == 0) {
        logError("BoidsScene::drawBoid: Cannot draw, ledCount() is zero.");
        return; 
    }

    for (size_t i = 0; i < num_leds; ++i) {
        const auto& point = this->model().point(i); 
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
        float intensity_setting = settings["intensity"]; 
        uint8_t blend_amount = static_cast<uint8_t>(intensity_setting * 255.0f); 
        PixelTheater::nblend(leds[closest_led_index], boid.color, blend_amount); 
    } else {
        if (closest_led_index >= static_cast<int>(num_leds)) { 
            snprintf(log_buffer, sizeof(log_buffer), "BoidsScene::drawBoid: Closest LED index %d out of bounds (%zu)", closest_led_index, num_leds);
            logError(log_buffer);
        } else { 
            snprintf(log_buffer, sizeof(log_buffer), "BoidsScene::drawBoid: Could not find closest LED for boid %u", boid.boid_id);
            logError(log_buffer);
        }
    }
}

float BoidsScene::sphericalDistance(const Boid& b1, const Boid& b2) const {
    Vector3f dir1 = b1.pos.normalized();
    Vector3f dir2 = b2.pos.normalized();
    float dot = std::clamp(dir1.dot(dir2), -1.0f, 1.0f);
    return acos(dot); 
}

std::string BoidsScene::status() const {
    // Provide a basic status, maybe add more detail later
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Boids: %zu | Chaos: %.2f | SpeedL: %.1f", 
            boids.size(), 
            static_cast<float>(settings["chaos"]), // Get current chaos param
            static_cast<float>(settings["speed_limit"])
            );
    return std::string(buffer);
}

// --- Boid Class Implementation (remains inline in header for now) ---
// ... (Boid::Boid, Boid::reset, etc. implementations were already in the header)

} // namespace Scenes 