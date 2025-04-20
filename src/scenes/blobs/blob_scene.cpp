#include "blob_scene.h"
#include "PixelTheater/SceneKit.h" // SceneKit provides short aliases
#include "benchmark.h"   // Include benchmark helpers if used

// Include standard libraries used by the implementations
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>

namespace Scenes {

// --- BlobScene Implementation --- 

void BlobScene::setup() {
    set_name("Blobs");
    set_description("Colorful blobs moving on the surface");
    set_version("1.1");
    set_author("PixelTheater Team");
    
    // Parameter ranges
    const int MIN_BLOBS = 1, MAX_BLOBS = 20;
    const int MIN_RADIUS_LOW = 10, MIN_RADIUS_HIGH = 100;
    const int MAX_RADIUS_LOW = 50, MAX_RADIUS_HIGH = 200;
    const int MIN_AGE = 500, MAX_AGE = 10000;
    const int MIN_FADE = 1, MAX_FADE = 20;
        
    // Define parameters using static consts for defaults
    param("num_blobs", "count", MIN_BLOBS, MAX_BLOBS, DEFAULT_NUM_BLOBS, "clamp", "Number of blobs");
    param("min_radius", "count", MIN_RADIUS_LOW, MIN_RADIUS_HIGH, DEFAULT_MIN_RADIUS, "clamp", "Min blob radius");
    param("max_radius", "count", MAX_RADIUS_LOW, MAX_RADIUS_HIGH, DEFAULT_MAX_RADIUS, "clamp", "Max blob radius");
    param("max_age", "count", MIN_AGE, MAX_AGE, DEFAULT_MAX_AGE, "clamp", "Max blob lifetime (frames)");
    param("speed", "ratio", DEFAULT_SPEED, "clamp", "Animation speed scale");
    param("fade", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame (1-20)");
    
    logInfo("BlobScene Parameters defined"); 

    BENCHMARK_RESET(); // Reset benchmark counters if used
    initBlobs(); // Initialize blobs after parameters are defined
    logInfo("BlobScene setup complete");
}

void BlobScene::initBlobs() {
    // Get parameter values
    int num_blobs = settings["num_blobs"];
    int min_radius = settings["min_radius"];
    int max_radius = settings["max_radius"];
    int max_age = settings["max_age"];
    float speed = settings["speed"];
        
    logInfo("Creating %d blobs...", num_blobs);
    
    blobs.clear(); // Clear any existing blobs
    blobs.reserve(num_blobs); // Pre-allocate vector space for efficiency

    for (int i = 0; i < num_blobs; i++) {
        // Create a new Blob using unique_ptr for automatic memory management
        auto blob = std::make_unique<Blob>(*this, i, min_radius, max_radius, max_age, speed);
        
        // Assign a unique color to each blob
        CHSV hsv(random8(), 255, 255);
        blob->color = hsv; // Assign CHSV, relies on CRGB(CHSV) constructor in CRGB class
        
        blobs.push_back(std::move(blob)); // Move ownership to the vector
    }
    logInfo("%d Blobs created.", (int)blobs.size());
        
    // Fallback if parameter parsing failed or resulted in zero blobs
    if (blobs.empty() && num_blobs == 0) { 
        logWarning("No blobs created based on parameters, creating fallback blobs.");
        num_blobs = 3; // Create a small number of fallback blobs
        for (int i = 0; i < num_blobs; i++) { 
            auto blob = std::make_unique<Blob>(*this, i, 50, 80, 4000, 1.0f); // Use fixed defaults
            CHSV hsv(i * 85, 255, 255); // Spread hues for fallback blobs
            blob->color = hsv; 
            blobs.push_back(std::move(blob));
        }
        logWarning("%d Fallback blobs created.", (int)blobs.size());
    }
}

void BlobScene::tick() {
    BENCHMARK_START("scene_total"); // Start total scene benchmark timer
    Scene::tick(); // Call base class tick (e.g., increments tick_count)
        
    BENCHMARK_START("get_parameters"); // Benchmark parameter access
    uint8_t fade_amount = static_cast<uint8_t>(settings["fade"]);
    BENCHMARK_END(); // End parameter access benchmark
        
    BENCHMARK_START("update_blobs"); // Benchmark blob physics update
    updateBlobs();
    BENCHMARK_END(); // End blob physics update benchmark
        
    BENCHMARK_START("draw_blobs"); // Benchmark drawing blobs to LEDs
    drawBlobs();
    BENCHMARK_END(); // End drawing benchmark
        
    BENCHMARK_START("fade_leds"); // Benchmark fading all LEDs
    size_t count = ledCount();
    for(size_t i = 0; i < count; ++i) {
        leds[i].fadeToBlackBy(fade_amount); // Use CRGB method via leds proxy
    }
    BENCHMARK_END(); // End LED fading benchmark
        
    BENCHMARK_END(); // End total scene benchmark timer
}

void BlobScene::updateBlobs() {
    // Update each blob's internal state (position, velocity, age, etc.)
    for (auto& blob : blobs) {
        blob->tick();
    }

    // Apply pairwise repulsion between blobs
    static const float forceStrength = 0.000005f; // Strength of repulsion
    for (size_t i = 0; i < blobs.size(); ++i) {
        for (size_t j = i + 1; j < blobs.size(); ++j) { // Avoid self-comparison and duplicate pairs
            // Calculate desired minimum distance based on radii
            float min_dist = (blobs[i]->radius + blobs[j]->radius) / 2.0f; 
            float min_dist_sq = min_dist * min_dist;
            
            // Calculate vector and squared distance between blob centers
            float dx = static_cast<float>(blobs[i]->x() - blobs[j]->x());
            float dy = static_cast<float>(blobs[i]->y() - blobs[j]->y());
            float dz = static_cast<float>(blobs[i]->z() - blobs[j]->z());
            float dist_sq = dx*dx + dy*dy + dz*dz;
            
            // Apply repulsion only if closer than min_dist and not exactly overlapping
            if (dist_sq < min_dist_sq && dist_sq > 20.0f) { // Restore original distance check
                float dist = sqrt(dist_sq);
                // Calculate force magnitude (stronger the closer they are)
                float force = ((min_dist - dist) / min_dist) * forceStrength; 
                
                // Calculate normalized direction vector
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;
                
                // Apply force to both blobs in opposite directions (using Cartesian force application)
                blobs[i]->applyForce(nx * force, ny * force, nz * force);
                blobs[j]->applyForce(-nx * force, -ny * force, -nz * force);
            }
        }
    }
}

void BlobScene::drawBlobs() {
    size_t count = ledCount();
    // For each LED, check its proximity to each blob and blend colors
    for (size_t i = 0; i < count; ++i) {
        const auto& p = model().point(i); // Get the 3D position of the current LED
        CRGB& current_led = leds[i]; // Get reference to the LED's color object

        for (auto& blob : blobs) {
            // Calculate squared distance from LED to blob center
            auto rad_sq = blob->radius * blob->radius;
            int dx = p.x() - blob->x();
            int dy = p.y() - blob->y();
            int dz = p.z() - blob->z();
            int dist_sq = dx*dx + dy*dy + dz*dz;

            // If the LED is within the blob's radius...
            if (dist_sq < rad_sq) {
                CRGB blob_draw_color = blob->color;
                
                // Fade in new blobs over the first 150 frames
                if (blob->age < 150) { 
                    // Map age (0-150) to fade amount (180 -> 0)
                    uint8_t fade_in_amount = map(blob->age, 0, 150, 180, 0); 
                    blob_draw_color.fadeToBlackBy(fade_in_amount); // Apply fade
                }
                
                // Blend the blob's color onto the LED
                // Closer distance = stronger blend (more opaque)
                // Map distance squared (0-rad_sq) to blend amount (255 -> 8)
                uint8_t blend_amount = map(dist_sq, 0, rad_sq, 255, 8); 
                
                // Use blend8 for efficient 8-bit per channel blending
                current_led.r = blend8(current_led.r, blob_draw_color.r, blend_amount);
                current_led.g = blend8(current_led.g, blob_draw_color.g, blend_amount);
                current_led.b = blend8(current_led.b, blob_draw_color.b, blend_amount);
            }
        } 
    }
}

} // namespace Scenes 