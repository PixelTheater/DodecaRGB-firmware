#include "particle.h"
#include "wandering_particles_scene.h" // Include scene header for access via scene&
#include "PixelTheater.h" // For constants, random, model access etc.

#include <cmath>      // For sin, cos, acos, atan2 etc. if needed
#include <algorithm>  // For std::clamp, std::min, std::fill
// Include Eigen for vector math - REMOVED (Included via PixelTheater.h -> math.h)
// #include <Eigen/Core> 

// Use Eigen types locally
using Vector3f = Eigen::Vector3f;

namespace Scenes {

// --- Particle Implementation ---

Particle::Particle(WanderingParticlesScene& parent_scene, uint16_t unique_id)
    : scene(parent_scene),
      particle_id(unique_id),
      path(MAX_PATH_LENGTH, -1) 
{
    // Call the common initializer with a random starting LED
    initializeParticleState(scene.random(scene.ledCount())); 
}

// Common initialization logic used by reset() and resetAtOppositePole()
void Particle::initializeParticleState(int start_led_number) {
    led_number = start_led_number;
    std::fill(path.begin(), path.end(), -1); 

    if (led_number >= 0 && led_number < (int)scene.ledCount()) {
        path[0] = led_number; 
        const auto& p = scene.model().point(led_number);
        float r = sqrt(p.x()*p.x() + p.y()*p.y() + p.z()*p.z());
        if (r > 1e-6) { 
            c = acos(std::clamp(p.z() / r, -1.0f, 1.0f)); 
            a = atan2(p.y(), p.x()); 
        } else {
            a = 0; c = 0; 
        }
    } else {
        // Invalid start LED, maybe default to 0?
        led_number = 0; 
        if (led_number < (int)scene.ledCount()) { // Check if LED 0 is valid
             path[0] = led_number;
             const auto& p = scene.model().point(led_number);
             float r = sqrt(p.x()*p.x() + p.y()*p.y() + p.z()*p.z());
             if (r > 1e-6) { 
                 c = acos(std::clamp(p.z() / r, -1.0f, 1.0f)); 
                 a = atan2(p.y(), p.x()); 
             } else { a = 0; c = 0; }
        } else {
             a = 0; c = 0; // Still default angles if LED 0 invalid
             led_number = -1; // Mark as invalid
        }
    }

    // Assign color
    uint8_t lev = scene.random(10, 51); 
    color = PixelTheater::CRGB(lev, scene.random(100, 231), lev);

    // Initialize velocity/direction tendency
    float max_accel = 0.02f;
    av = scene.randomFloat(-max_accel, max_accel); 
    cv = scene.randomFloat(-max_accel, max_accel); 
    
    // Reset core state variables
    age = 0;
    ticks_at_pole = 0; 
    hold_time = scene.random(4, 12);
    lifespan = scene.random(200, 701); 
    state = ParticleState::FADING_IN; // Start in fade-in state
}

// Standard reset: pick a random LED and initialize
void Particle::reset() {
    initializeParticleState(scene.random(scene.ledCount()));
}

// Reset at opposite pole: find target LED and initialize
void Particle::resetAtOppositePole(bool stuck_at_north_pole) {
    // scene.logInfo(...); // Logging removed for brevity

    float target_z = stuck_at_north_pole ? -scene.sphere_radius : scene.sphere_radius;
    int best_led = -1;
    float min_dist_sq = 1e18f;

    for (int i = 0; i < (int)scene.ledCount(); ++i) {
        const auto& p = scene.model().point(i);
        float dz = p.z() - target_z;
        float dist_sq = dz * dz; 
        if (dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            best_led = i;
        }
    }

    if (best_led == -1) { 
        initializeParticleState(scene.random(scene.ledCount())); // Fallback to random reset
    } else {
        initializeParticleState(best_led); // Initialize at the found LED
        // Give it a slight push away from the pole it just arrived at
        cv += stuck_at_north_pole ? 0.01f : -0.01f; 
    }
}

// Calculated Cartesian position based on angular coords and scene radius
// Note: These are less relevant now as movement is LED-to-LED, but keep for potential future use or debugging
float Particle::x() const { return scene.sphere_radius * sin(c) * cos(a); }
float Particle::y() const { return scene.sphere_radius * sin(c) * sin(a); }
float Particle::z() const { return scene.sphere_radius * cos(c); }

void Particle::tick() {
    age++;

    // Check lifespan first (ends FADING_OUT or ALIVE)
    if (age > lifespan) {
        reset();
        return; // Exit tick if reset
    }

    // --- State transition logic ---
    if (state == ParticleState::FADING_IN && age > FADE_IN_DURATION) {
        state = ParticleState::ALIVE;
    } else if (state == ParticleState::ALIVE && (lifespan - age) < FADE_OUT_DURATION) {
        state = ParticleState::FADING_OUT;
    }
    // --- End State transition logic ---

    // Check hold time for movement
    if (age > hold_time) {
        // --- NEW MOVEMENT LOGIC --- 
        float gravity_strength = scene.settings["gravity"]; // Read gravity from scene
        findNextLed(gravity_strength); // Pass gravity to the updated function
        // --- END NEW MOVEMENT LOGIC ---
        
        // Update path history
        if (led_number >= 0) { // Only update path if a valid LED was chosen
            // Shift elements down: path[i] = path[i-1]
            for (size_t i = path.size() - 1; i > 0; --i) {
                path[i] = path[i-1];
            }
            path[0] = led_number; // Add new LED to the start
        }
        // Ensure path doesn't exceed max length (already handled by vector size)

        // Reset age for the next hold period
        age = 0;
    }

    // --- Pole Sticking Logic --- 
    float gravity_strength = scene.settings["gravity"];
    const float GRAVITY_THRESHOLD = 0.01f; // Ignore very small gravity values
    const float POLE_ZONE_THRESHOLD = 0.82f; // Relaxed threshold
    const int POLE_STICK_LIMIT = 20; // Max ticks to stay at pole

    if (abs(gravity_strength) > GRAVITY_THRESHOLD && led_number >= 0) {
        const auto& p = scene.model().point(led_number);
        float z_norm = 0.0f;
        if (scene.sphere_radius > 1e-6f) {
             z_norm = p.z() / scene.sphere_radius;
        }

        bool at_north_pole = (gravity_strength < 0 && z_norm > POLE_ZONE_THRESHOLD); // Negative gravity pulls up
        bool at_south_pole = (gravity_strength > 0 && z_norm < -POLE_ZONE_THRESHOLD); // Positive gravity pulls down

        if (at_north_pole || at_south_pole) {
            ticks_at_pole++;
            if (ticks_at_pole > POLE_STICK_LIMIT) {
                 resetAtOppositePole(at_north_pole); // Pass which pole it was stuck at
                 return; // Exit tick after reset
            }
        } else {
            ticks_at_pole = 0; // Reset counter if not at a pole
        }
    } else {
        ticks_at_pole = 0; // Reset counter if gravity is off
    }
    // --- End Pole Sticking Logic --- 

    // Add periodic direction change
    if (scene.random(100) < 2) {
        // Slightly perturb the angular velocities which influence direction choice
        float max_rand_av = 0.005f; // Define small random change magnitude
        float max_rand_cv = 0.005f;
        av += scene.randomFloat(-max_rand_av, max_rand_av);
        cv += scene.randomFloat(-max_rand_cv, max_rand_cv);
        // Consider clamping av/cv if they can grow too large
    }
}

// REWRITTEN findNextLed based on direction, now with randomization
void Particle::findNextLed(float gravity_strength) { 
    // Safety check for current LED validity
    if (led_number < 0 || led_number >= (int)scene.ledCount()) {
         reset(); 
         return;
    }

    const auto& current_point = scene.model().point(led_number);
    Vector3f p_current(current_point.x(), current_point.y(), current_point.z());
    Vector3f preferred_direction;

    // Determine preferred direction (same as before)
    int previous_led = (path.size() > 1 && path[1] != -1) ? path[1] : -1;
    if (previous_led >= 0 && previous_led < (int)scene.ledCount()) {
        const auto& prev_point = scene.model().point(previous_led);
        Vector3f p_prev(prev_point.x(), prev_point.y(), prev_point.z());
        preferred_direction = (p_current - p_prev);
        if (preferred_direction.norm() > 1e-6f) { 
             preferred_direction.normalize();
        } else { 
             preferred_direction = Vector3f::Random().normalized();
        }
    } else {
        preferred_direction = Vector3f::Random().normalized();
    }

    // --- Gravity Influence Calculation (same as before) ---
    Vector3f gravity_vector(0.0f, 0.0f, -gravity_strength); 
    float z_norm = 0.0f;
    if (scene.sphere_radius > 1e-6f) {
        z_norm = std::clamp(p_current.z() / scene.sphere_radius, -1.0f, 1.0f);
    }
    float equator_factor_sq = 1.0f - z_norm * z_norm;
    float equator_factor = (equator_factor_sq > 0.0f) ? sqrt(equator_factor_sq) : 0.0f;
    Vector3f effective_gravity_vector = gravity_vector * equator_factor;
    float gravity_weight = 0.5f; 
    Vector3f target_direction = preferred_direction + effective_gravity_vector * gravity_weight;
    if (target_direction.norm() < 1e-6f) { 
         target_direction = preferred_direction;
         if (target_direction.norm() < 1e-6f) { 
             target_direction = Vector3f::Random();
         }
    }
    target_direction.normalize();
    // --- End Gravity Influence ---

    const auto& neighbors = current_point.getNeighbors(); 
    
    // --- Collect Candidate Neighbors --- 
    std::vector<std::pair<float, int>> candidates; // Store <dot_product, led_index>
    const float DIRECTION_ALIGNMENT_THRESHOLD = 0.3f; // Min dot product to consider (tune this) 

    for (const auto& neighbor : neighbors) {
        if (neighbor.id == 0xFFFF || neighbor.distance <= 1e-6f) { 
            continue;
        }
        int potential_next_led = neighbor.id;
        if (potential_next_led < 0 || potential_next_led >= (int)scene.ledCount()) continue;

        // Path Avoidance (same as before)
        bool in_path = false;
        for(size_t p_idx = 0; p_idx < std::min(size_t(4), path.size()); ++p_idx) { 
            if (path[p_idx] != -1 && path[p_idx] == potential_next_led) {
                in_path = true;
                break;
            }
        }
        if (in_path) continue; 

        // Direction Alignment Check
        const auto& neighbor_point = scene.model().point(potential_next_led);
        Vector3f p_neighbor(neighbor_point.x(), neighbor_point.y(), neighbor_point.z());
        Vector3f neighbor_vector = (p_neighbor - p_current);
        if (neighbor_vector.norm() < 1e-6f) continue; 
        neighbor_vector.normalize(); 

        float dot_product = target_direction.dot(neighbor_vector);

        // Only consider neighbors reasonably aligned with the target direction
        if (dot_product > DIRECTION_ALIGNMENT_THRESHOLD) {
            candidates.push_back({dot_product, potential_next_led});
        }
    }
        
    // --- Choose Next LED --- 
    if (!candidates.empty()) {
        // Option A: Choose randomly from ALL valid candidates
        // led_number = candidates[scene.random(candidates.size())].second;

        // Option B: Choose randomly from the TOP N candidates (e.g., up to 3)
        // Sort candidates by dot product (descending)
        std::sort(candidates.begin(), candidates.end(), std::greater<std::pair<float, int>>());
        size_t num_choices = std::min(candidates.size(), (size_t)3); // Consider top 3
        led_number = candidates[scene.random(num_choices)].second; // Pick one randomly
        
    } else {
        // Fallback: No suitable aligned neighbors found
        std::vector<int> valid_neighbors;
        for (const auto& neighbor : neighbors) {
             if (neighbor.id != 0xFFFF && neighbor.distance > 1e-6f && neighbor.id < (int)scene.ledCount()) {
                 valid_neighbors.push_back(neighbor.id);
             }
        }
        if (!valid_neighbors.empty()) {
            led_number = valid_neighbors[scene.random(valid_neighbors.size())];
        } else {
            reset(); 
        }
    }
}


} // namespace Scenes 