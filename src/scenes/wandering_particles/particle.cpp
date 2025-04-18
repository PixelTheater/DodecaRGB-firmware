#include "particle.h"
#include "wandering_particles_scene.h" // Include scene header for access via scene&
#include "PixelTheater.h" // For constants, random, model access etc.

#include <cmath>      // For sin, cos, acos, atan2 etc. if needed
#include <algorithm>  // For std::clamp, std::min, std::fill

namespace Scenes {

// --- Particle Implementation ---

Particle::Particle(WanderingParticlesScene& parent_scene, uint16_t unique_id)
    : scene(parent_scene),
      particle_id(unique_id),
      path(MAX_PATH_LENGTH, -1) // Initialize path vector
{
    reset(); // Call reset to initialize state
}

void Particle::reset() {
    led_number = scene.random(scene.ledCount()); // Pick random starting LED
    std::fill(path.begin(), path.end(), -1); // Clear path history
    if (led_number >= 0 && led_number < (int)scene.ledCount()) {
        path[0] = led_number; // Set current LED as start of path
        // Position particle exactly at the starting LED initially
        const auto& p = scene.model().point(led_number);
        float r = sqrt(p.x()*p.x() + p.y()*p.y() + p.z()*p.z());
        if (r > 1e-6) { // Avoid division by zero
            c = acos(p.z() / r); // Polar angle
            a = atan2(p.y(), p.x()); // Azimuthal angle
        } else {
            a = 0; c = 0; // Default to origin/pole if radius is zero
        }
    } else {
        a = 0; c = 0; // Default if initial LED is invalid
        led_number = -1;
    }

    // Assign color (example: greenish with random brightness)
    uint8_t lev = scene.random(10, 51); 
    color = PixelTheater::CRGB(lev, scene.random(100, 231), lev);

    // Initialize velocity (using ranges similar to the monolithic header)
    av = scene.randomFloat(0.02f, 0.06f) / 25.0f;
    cv = scene.randomFloat(0.02f, 0.06f) / 24.0f;
    
    // Reset age and hold time
    age = 0;
    hold_time = scene.random(6, 12);
}

// Calculated Cartesian position based on angular coords and scene radius
float Particle::x() const { return scene.sphere_radius * sin(c) * cos(a); }
float Particle::y() const { return scene.sphere_radius * sin(c) * sin(a); }
float Particle::z() const { return scene.sphere_radius * cos(c); }

void Particle::tick() {
    age++;
    if (age > hold_time) {
        // Update angular position based on velocity
        a += av;
        c += cv;

        // Wrap azimuth angle a to [-PI, PI]
        using PixelTheater::Constants::PT_PI;
        using PixelTheater::Constants::PT_TWO_PI;
        a = fmod(a + PT_PI, PT_TWO_PI) - PT_PI;

        // Clamp polar angle c to [0, PI]
        c = std::clamp(c, 0.0f, PT_PI);

        // Find the closest LED to the new target position
        findNextLed();

        // Update path history
        // Shift elements down: path[i] = path[i-1]
        for (size_t i = path.size() - 1; i > 0; --i) {
            path[i] = path[i-1];
        }
        path[0] = led_number; // Add new LED to the start
        // Ensure path doesn't exceed max length (already handled by vector size)

        // Reset age for the next hold period
        age = 0;
    }
}

// Internal helper to find the next LED based on current particle position
void Particle::findNextLed() {
    // Target position based on current angular coordinates
    float px = x();
    float py = y();
    float pz = z();

    // Safety check for current LED validity
    if (led_number < 0 || led_number >= (int)scene.ledCount()) {
         scene.logWarning("Particle %d: Invalid current LED %d, resetting.", particle_id, led_number);
         reset(); 
         return;
    }

    // Get neighbors of the current LED from the precomputed model data
    const auto& current_point = scene.model().point(led_number);
    const auto& neighbors = current_point.getNeighbors(); 
    
    float closest_dist_sq = 1e18f; // Initialize with a large value (using squared distance)
    int closest_led = -1;
    bool found_suitable_neighbor = false;

    // Iterate through the pre-calculated neighbors
    for (const auto& neighbor : neighbors) {
        // Check if the neighbor entry is valid (ID 0xFFFF is sentinel)
        if (neighbor.id == 0xFFFF || neighbor.distance <= 0.0f) {
            continue; // Skip invalid or padding entries
        }

        int potential_next_led = neighbor.id;

        // Check if this neighbor is in the recent path (avoid backtracking)
        bool in_path = false;
        // Check first few elements of path (e.g., last 3 positions)
        for(size_t p_idx = 1; p_idx < std::min(size_t(3), path.size()); ++p_idx) {
             // Start check from index 1 (previous LED) to avoid immediate back-and-forth
            if (path[p_idx] != -1 && path[p_idx] == potential_next_led) {
                in_path = true;
                break;
            }
        }
        if (in_path) continue; // Skip if recently visited

        // Get the neighbor's point data (check bounds first)
        if (potential_next_led < 0 || potential_next_led >= (int)scene.ledCount()) continue;
        const auto& neighbor_point = scene.model().point(potential_next_led);

        // Calculate squared distance from the particle's *target* position to the neighbor LED
        float dx = neighbor_point.x() - px;
        float dy = neighbor_point.y() - py;
        float dz = neighbor_point.z() - pz;
        float dist_sq = dx*dx + dy*dy + dz*dz;

        // If this neighbor is closer, it becomes the current best candidate
        if (dist_sq < closest_dist_sq) {
            closest_dist_sq = dist_sq;
            closest_led = potential_next_led;
            found_suitable_neighbor = true;
        }
    }
        
    // Update the particle's current LED if a suitable neighbor was found
    if (found_suitable_neighbor) {
        led_number = closest_led;
    }
    // else: stay on the current LED if no suitable non-path neighbor found
    // This can happen if trapped or at a dead end in the neighbor graph.
}


} // namespace Scenes 