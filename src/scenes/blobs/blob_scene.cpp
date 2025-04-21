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
    set_version("2.1");
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
            auto blob = std::make_unique<Blob>(*this, i, 50, 80, 4000, 1.0f);
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
    static const float forceStrength = 0.000002f; // Strength of repulsion
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
            if (dist_sq < min_dist_sq && dist_sq > 30.0f) { // Restore original distance check
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
                
                // --- Eased Fade-In --- 
                if (blob->age < FADE_IN_DURATION) { 
                    float t = static_cast<float>(blob->age) / static_cast<float>(FADE_IN_DURATION);
                    // Use a specific easing function directly
                    float progress = PixelTheater::Easing::outSineF(t);
                    uint8_t brightness = static_cast<uint8_t>(progress * 255.0f);
                    blob_draw_color.nscale8(brightness); // Apply brightness instead of fade
                }
                // --- End Eased Fade-In ---
                
                // --- Eased Blend Falloff ---
                // Map distance squared (0-rad_sq) to blend amount (100 -> 4)
                uint8_t blend_amount = 4; // Default to minimum blend
                if (rad_sq > 0) { // Avoid division by zero
                    float t = static_cast<float>(dist_sq) / static_cast<float>(rad_sq);
                    // Invert t for falloff (1=center, 0=edge), then ease
                    float inverted_t = 1.0f - t;
                    // Use an ease-out function for softer edges
                    float eased_falloff = PixelTheater::Easing::outSineF(inverted_t);
                    
                    // Map eased falloff [0, 1] back to blend range [4, 100]
                    blend_amount = static_cast<uint8_t>(4.0f + eased_falloff * (100.0f - 4.0f));
                    blend_amount = std::max((uint8_t)4, std::min((uint8_t)100, blend_amount)); // Clamp just in case
                }
                // --- End Eased Blend Falloff ---
                
                // Use nblend for efficient blending of the whole color
                nblend(current_led, blob_draw_color, blend_amount);
            }
        } 
    }
}

} // namespace Scenes 