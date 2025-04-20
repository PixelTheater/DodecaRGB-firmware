#include "blob.h"
#include "blob_scene.h" // Include the scene header to access its members via the 'scene' reference
#include "PixelTheater.h" // For Scene utilities like random*, log*, model(), constants, map()

#include <cmath> // For sin, cos, sqrt, atan2, fmod, abs
#include <algorithm> // For std::clamp, std::max

namespace Scenes {

// --- Blob Implementation --- 

Blob::Blob(BlobScene& parent_scene, uint16_t unique_id, int min_r, int max_r, int max_a, float speed)
    : scene(parent_scene),
      blob_id(unique_id),
      min_radius(min_r),
      max_radius(max_r),
      max_age(max_a),
      speed_scale(speed),
      color(PixelTheater::CRGB::White) // Initialize color here
{ 
    // Note: color is set properly in BlobScene::initBlobs after construction
    reset();
}

void Blob::reset() {
    age = 0;
    lifespan = scene.random(max_age/2, max_age); // Use scene reference
    radius = scene.random(min_radius, max_radius); // Use scene reference
    max_accel = scene.randomFloat(0.005f, 0.010f) * speed_scale * 5; // Use scene reference
    av = scene.randomFloat(-max_accel, max_accel); // Use scene reference
    cv = scene.randomFloat(-max_accel, max_accel); // Use scene reference

    // Bring constants into scope locally if preferred
    using PixelTheater::Constants::PT_PI;
    using PixelTheater::Constants::PT_TWO_PI;
    a = scene.randomFloat(0.0f, PT_TWO_PI) - PT_PI; // Use scene reference
    c = scene.randomFloat(0.0f, PT_TWO_PI) - PT_PI; // Use scene reference
}

int Blob::x() const { 
    // Ensure sphere_radius is positive before calculations
    float radius = scene.model().getSphereRadius();
    return (radius > 0) ? static_cast<int>(radius * sin(c) * cos(a)) : 0;
}
int Blob::y() const { 
    float radius = scene.model().getSphereRadius();
    return (radius > 0) ? static_cast<int>(radius * sin(c) * sin(a)) : 0;
}
int Blob::z() const { 
    float radius = scene.model().getSphereRadius();
    return (radius > 0) ? static_cast<int>(radius * cos(c)) : 0;
}

void Blob::applyForce(float af, float cf) {
    av += af;
    av = std::clamp(av, -max_accel, max_accel);
    cv += cf;
    cv = std::clamp(cv, -max_accel, max_accel);
}

void Blob::applyForce(float fx, float fy, float fz) {
    // Convert Cartesian force direction to angular force
    // Note: This conversion might not be physically accurate for force magnitude scaling
    // but approximates the direction of force application in the angular space.
    /* float current_x = static_cast<float>(x());
    float current_y = static_cast<float>(y());
    float current_z = static_cast<float>(z());
    float dist_sq = current_x * current_x + current_y * current_y + current_z * current_z;

    if (dist_sq < 1e-6f) return; // Avoid division by zero if at origin

    // Calculate approximate angular change based on Cartesian force direction
    // This is a simplification; true spherical coordinate forces are more complex.
    float af_target = atan2(fy, fx); // Target azimuth
    // Calculate target polar angle (inclination) carefully handling domain for acos
    float cos_angle_arg = fz / sqrt(dist_sq);
    float cf_target = acos(std::clamp(cos_angle_arg, -1.0f, 1.0f)); 

    // Calculate the difference in angles (shortest path)
    float delta_a = af_target - a;
    // Normalize delta_a to [-PI, PI]
    while (delta_a > PixelTheater::Constants::PT_PI) delta_a -= PixelTheater::Constants::PT_TWO_PI;
    while (delta_a <= -PixelTheater::Constants::PT_PI) delta_a += PixelTheater::Constants::PT_TWO_PI;
    
    float delta_c = cf_target - c; // Polar angle difference is direct

    // Apply a fraction of the angular difference as force (adjust multiplier as needed)
    float force_magnitude_scaler = 0.001f; // Example scaling factor
    applyForce(delta_a * force_magnitude_scaler, delta_c * force_magnitude_scaler); */
    
    // --- Restore Original Logic --- 
    float af = atan2(fy, fx);
    float dist_xy = sqrt(fx*fx + fy*fy);
    float cf = atan2(dist_xy, fz);
    applyForce(af, cf);
    // --- End Restore --- 
}

void Blob::tick() {
    // Bring constants into scope locally if preferred
    using PixelTheater::Constants::PT_PI;
    using PixelTheater::Constants::PT_TWO_PI;

    // Apply inherent forces/damping (restored original logic)
    // float force_av = 0.0f; // Start with zero base angular force for azimuth
    float force_av = av * 1.001f; // Restore original
    // Keep c in [-PI, PI] before calculations
    c = fmod(c + PT_PI, PT_TWO_PI) - PT_PI; 
    // float force_cv = 0.0f;
    float force_cv = 0.00035f * (c - PT_PI/2.0f); // Restore original pole force logic
    if (c < -PT_PI/2.0f) {
        force_cv = -0.0003f * (c + PT_PI/2.0f);
    }
    // Pole repulsion/attraction (simplified from original example's structure)
    // if (c != 0.0f) { // Avoid applying force exactly at the equator
    //     force_cv = -c * 0.0003f; // Simple force proportional to distance from equator
    // }

    // Apply calculated inherent forces
    applyForce(force_av, force_cv);
    
    // Update age
    age++; // Increment age once here

    // Apply damping (restored original damping)
    av *= 0.99f; 
    cv *= 0.99f; 

    // Update position
    a += av;
    c += cv;
    // Keep c within bounds [-PI, PI] - important for stability and trig functions
    // c = std::clamp(c, -PT_PI, PT_PI); // Remove added clamp
    // Wrap a around [-PI, PI]
    // a = fmod(a + PT_PI, PT_TWO_PI) - PT_PI; // Remove added wrap

    
    // Random nudge if velocity is very low (restored original logic)
    // if (abs(cv) < 0.001f && abs(av) < 0.001f) { // Check both angular velocities
    if (abs(cv) < 0.001f) { // Restore original condition (only check cv)
        // float nudge_a = scene.randomFloat(-max_accel, max_accel) * 0.5f; // Use scene reference
        // float nudge_c = scene.randomFloat(-max_accel, max_accel); // Use scene reference
        float af = scene.randomFloat(-max_accel, max_accel); // Restore original nudge calculation
        float cf = scene.randomFloat(-max_accel, max_accel);
        applyForce(af/2.0f, cf); // Restore original application
    }
    
    // Shrink near end of life (restored original shrink logic)
    if ((lifespan - age) < (max_age / 20)) { 
        // radius = static_cast<int>(radius * 0.99f); // Apply shrink factor
        radius *= 0.99f; // Restore original (no cast needed for int*float)
        // radius = std::max(1, radius); // Remove added ensure > 0
    }

    // Reset if lifespan exceeded (restored original reset logic)
    // if (age > lifespan) { 
    if (age++ > lifespan) { // Restore original double increment
        reset();
    }
}

} // namespace Scenes 