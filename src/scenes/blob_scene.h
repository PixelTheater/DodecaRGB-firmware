#pragma once

#include "PixelTheater.h" 
#include "benchmark.h"
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm> // For std::max, std::min, std::clamp
#include <string> // For status method (if restored)

// using namespace PixelTheater; // Avoid top-level using
// using namespace PixelTheater::Constants; // Avoid top-level using

namespace Scenes {

// Forward declaration
class BlobScene;

// Blob class 
class Blob {
public:
    int sphere_radius = 100;  
    BlobScene& scene; 
    uint16_t blob_id = 0;
    int radius = 0;      
    float a = 0.0f, c = 0.0f;  
    float av = 0.0f, cv = 0.0f;    
    float max_accel = 0.01f;  
    int age = 0;
    int lifespan = 1000;
    PixelTheater::CRGB color = PixelTheater::CRGB::White;
    
    Blob(BlobScene& parent_scene, uint16_t unique_id, int min_r, int max_r, int max_a, float speed);
    
    void estimateSphereRadius();
    void reset();
    int x() const { return sphere_radius * sin(c) * cos(a); }
    int y() const { return sphere_radius * sin(c) * sin(a); }
    int z() const { return sphere_radius * cos(c); }
    void applyForce(float af, float cf);
    void applyForce(float fx, float fy, float fz);
    void tick();
    
private:
    int min_radius;
    int max_radius;
    int max_age;
    float speed_scale;
};

// BlobScene class
class BlobScene : public PixelTheater::Scene { 
public:
    BlobScene() = default; 
    
    static constexpr int DEFAULT_NUM_BLOBS = 15;
    static constexpr int DEFAULT_MIN_RADIUS = 70;
    static constexpr int DEFAULT_MAX_RADIUS = 120;
    static constexpr int DEFAULT_MAX_AGE = 4000;
    static constexpr float DEFAULT_SPEED = 0.2;
    static constexpr uint8_t DEFAULT_FADE = 10;
    
    void setup() override;
    void initBlobs();
    void tick() override;
    void updateBlobs();
    void drawBlobs();
    
    friend class Blob;

private:
    std::vector<std::unique_ptr<Blob>> blobs;
};

// --- Implementations --- 

// Blob Implementation
Blob::Blob(BlobScene& parent_scene, uint16_t unique_id, int min_r, int max_r, int max_a, float speed)
    : scene(parent_scene),
      blob_id(unique_id),
      min_radius(min_r),
      max_radius(max_r),
      max_age(max_a),
      speed_scale(speed) {
    estimateSphereRadius();
    reset();
}

void Blob::estimateSphereRadius() {
    int max_dist_sq = 0;
    size_t count = scene.ledCount(); // Use ledCount() as primary index
    for (size_t i = 0; i < count; i++) { 
        const auto& p = scene.model().point(i); // Still need point geometry
        int dist_sq = p.x() * p.x() + p.y() * p.y() + p.z() * p.z();
        max_dist_sq = std::max(max_dist_sq, dist_sq);
    }
    if (max_dist_sq > 0) {
        sphere_radius = static_cast<int>(sqrt(static_cast<float>(max_dist_sq)));
        // scene.logInfo("Estimated sphere radius"); // Simplify logging - use const char*
        scene.logInfo("Radius estimate done"); 
    } else {
        scene.logWarning("Could not estimate sphere radius");
    }
}

void Blob::reset() {
    age = 0;
    lifespan = scene.random(max_age/2, max_age); // Use two-arg random
    radius = scene.random(min_radius, max_radius);
    max_accel = scene.randomFloat(0.005f, 0.010f) * speed_scale * 5;
    av = scene.randomFloat(-max_accel, max_accel);
    cv = scene.randomFloat(-max_accel, max_accel);
    using PixelTheater::Constants::PT_PI;
    using PixelTheater::Constants::PT_TWO_PI;
    a = scene.randomFloat(0.0f, PT_TWO_PI) - PT_PI;
    c = scene.randomFloat(0.0f, PT_TWO_PI) - PT_PI;
}

void Blob::applyForce(float af, float cf) {
    av += af;
    av = std::clamp(av, -max_accel, max_accel);
    cv += cf;
    cv = std::clamp(cv, -max_accel, max_accel);
}

void Blob::applyForce(float fx, float fy, float fz) {
    float af = atan2(fy, fx);
    float cf = atan2(sqrt(fx*fx + fy*fy), fz);
    applyForce(af, cf);
}

void Blob::tick() {
    using PixelTheater::Constants::PT_PI;
    using PixelTheater::Constants::PT_TWO_PI;
    // Restore original force calculations
    float force_av = av * 1.001f; 
    c = fmod(c + PT_PI, PT_TWO_PI) - PT_PI; 
    float force_cv = 0.00035f * (c - PT_PI/2.0f);
    if (c < -PT_PI/2.0f) {
        force_cv = -0.0003f * (c + PT_PI/2.0f);
    }
    applyForce(force_av, force_cv);
    
    age++;
    // Restore original damping
    av *= 0.99f; 
    cv *= 0.99f; 
    a += av; 
    c += cv;
    
    // Restore original random nudge logic
    if (abs(cv) < 0.001f) {
        float af = scene.randomFloat(-max_accel, max_accel); // Adjust range if needed based on original
        float cf = scene.randomFloat(-max_accel, max_accel);
        applyForce(af/2.0f, cf); // Check original division factors
    }
    
    // Restore original shrink logic
    if (lifespan - age < max_age/20) { 
        radius *= 0.99f;
    }
    // Restore original reset logic
    if (age++ > lifespan) { // Original used age++
        reset();
    }
}


// BlobScene Implementation
void BlobScene::setup() {
    set_name("Blobs");
    set_description("Colorful blobs moving on the surface");
    set_version("1.1");
    set_author("PixelTheater Team");
    
    const int MIN_BLOBS = 1, MAX_BLOBS = 20;
    const int MIN_RADIUS_LOW = 10, MIN_RADIUS_HIGH = 100;
    const int MAX_RADIUS_LOW = 50, MAX_RADIUS_HIGH = 200;
    const int MIN_AGE = 500, MAX_AGE = 10000;
    const int MIN_FADE = 1, MAX_FADE = 20;
        
    param("num_blobs", "count", MIN_BLOBS, MAX_BLOBS, DEFAULT_NUM_BLOBS, "clamp", "Number of blobs");
    param("min_radius", "count", MIN_RADIUS_LOW, MIN_RADIUS_HIGH, DEFAULT_MIN_RADIUS, "clamp", "Min blob radius");
    param("max_radius", "count", MAX_RADIUS_LOW, MAX_RADIUS_HIGH, DEFAULT_MAX_RADIUS, "clamp", "Max blob radius");
    param("max_age", "count", MIN_AGE, MAX_AGE, DEFAULT_MAX_AGE, "clamp", "Max blob lifetime (frames)");
    param("speed", "ratio", DEFAULT_SPEED, "clamp", "Animation speed scale");
    param("fade", "count", MIN_FADE, MAX_FADE, DEFAULT_FADE, "clamp", "Fade amount per frame (1-20)");
    
    logInfo("BlobScene Parameters defined"); 

    BENCHMARK_RESET();
    initBlobs();
    logInfo("BlobScene setup complete");
}

void BlobScene::initBlobs() {
    int num_blobs = settings["num_blobs"];
    int min_radius = settings["min_radius"];
    int max_radius = settings["max_radius"];
    int max_age = settings["max_age"];
    float speed = settings["speed"];
        
    logInfo("Creating blobs...");
    
    blobs.clear();
    blobs.reserve(num_blobs); // Pre-allocate vector space
    for (int i = 0; i < num_blobs; i++) {
        auto blob = std::make_unique<Blob>(*this, i, min_radius, max_radius, max_age, speed);
        PixelTheater::CHSV hsv(random8(), 255, 255);  
        PixelTheater::hsv2rgb_rainbow(hsv, blob->color);
        blobs.push_back(std::move(blob));
    }
    logInfo("Blobs created.");
        
    if (blobs.empty()) { 
        logWarning("No blobs created from params, using fallback");
        for (int i = 0; i < 3; i++) { // Fewer fallback blobs
            auto blob = std::make_unique<Blob>(*this, i, 50, 80, 4000, 1.0f);
            PixelTheater::CHSV hsv(i * 85, 255, 255); // Spread hues
            PixelTheater::hsv2rgb_rainbow(hsv, blob->color);
            blobs.push_back(std::move(blob));
        }
        logWarning("Fallback blobs created");
    }
}

void BlobScene::tick() {
    BENCHMARK_START("scene_total");
    Scene::tick();  
        
    BENCHMARK_START("get_parameters");
    uint8_t fade_amount = static_cast<uint8_t>(settings["fade"]);
    BENCHMARK_END();
        
    BENCHMARK_START("update_blobs");
    updateBlobs();
    BENCHMARK_END();
        
    BENCHMARK_START("draw_blobs");
    drawBlobs();
    BENCHMARK_END();
        
    BENCHMARK_START("fade_leds");
    size_t count = ledCount();
    for(size_t i = 0; i < count; ++i) {
        PixelTheater::fadeToBlackBy(leds[i], fade_amount); // Use leds[] proxy is fine too
    }
    BENCHMARK_END();
        
    BENCHMARK_END();
}

void BlobScene::updateBlobs() {
    static const float forceStrength = 0.000005f;
    for (auto& blob : blobs) {
        blob->tick();
    }
    // Restore original pairwise repulsion logic
    for (size_t i = 0; i < blobs.size(); i++) {
        for (size_t j = i + 1; j < blobs.size(); j++) {
            float min_dist = (blobs[i]->radius + blobs[j]->radius) / 2.0f;
            float min_dist_sq = min_dist * min_dist;
            float dx = blobs[i]->x() - blobs[j]->x();
            float dy = blobs[i]->y() - blobs[j]->y();
            float dz = blobs[i]->z() - blobs[j]->z();
            float dist_sq = dx*dx + dy*dy + dz*dz;
            // Restore original distance check
            if (dist_sq < min_dist_sq && dist_sq > 20.0f) { 
                float dist = sqrt(dist_sq);
                // Avoid division by zero/tiny number (good practice)
                if (dist < 1e-6f) continue; 
                float force = ((min_dist - dist) / min_dist) * forceStrength;
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;
                blobs[i]->applyForce(nx * force, ny * force, nz * force);
                blobs[j]->applyForce(-nx * force, -ny * force, -nz * force);
            }
        }
    }
}

void BlobScene::drawBlobs() {
    size_t count = ledCount();
    // For each LED, check distance to each blob
    for (size_t i = 0; i < count; ++i) {
        const auto& p = model().point(i); // Use model().point()
        PixelTheater::CRGB& current_led = leds[i]; // Use leds[] proxy

        for (auto& blob : blobs) {
            auto rad_sq = blob->radius * blob->radius;
            int dx = p.x() - blob->x();
            int dy = p.y() - blob->y();
            int dz = p.z() - blob->z();
            int dist = dx*dx + dy*dy + dz*dz;

            if (dist < rad_sq) {
                PixelTheater::CRGB c = blob->color;
                if (blob->age < 150) { // Fade in new blobs
                    uint8_t fade = PixelTheater::map(blob->age, 0, 150, 180, 0); // Fade from mostly black
                    PixelTheater::fadeToBlackBy(c, fade);
                }
                // Blend based on distance (closer is brighter/more opaque)
                uint8_t blend = PixelTheater::map(dist, 0, rad_sq, 255, 8); // Stronger blend
                current_led.r = PixelTheater::blend8(current_led.r, c.r, blend);
                current_led.g = PixelTheater::blend8(current_led.g, c.g, blend);
                current_led.b = PixelTheater::blend8(current_led.b, c.b, blend);
            }
        } 
    }
}

} // namespace Scenes 