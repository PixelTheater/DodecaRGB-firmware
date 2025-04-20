#include "sparkles.h"
#include <cmath> // For fmodf
// #include <fmt/core.h> // Remove dependency - not available in Teensy build
#include <string> // Include for std::string and std::to_string
#include "PixelTheater/SceneKit.h"
#include "PixelTheater/color/measurement.h"
#include <algorithm> // For std::clamp
#include <cmath> // For std::abs
// Assuming PixelTheater.h includes necessary palette/color utils
// #include "PixelTheater/palettes.h" 
// #include "PixelTheater/color/palette_utils.h"

// Bring namespaces into scope for convenience
using namespace Scenes;

void Sparkles::setup() {
    // Define Metadata
    set_name("Sparkles (New Spec)");
    set_description("Oscillating palettes blended with chaotic influence.");
    set_author("AI Assistant (New Spec)");

    // --- Define New Parameters ---
    // Speed for blend oscillation (lower = slower cycle)
    param("blend_cycle_speed", "range", 0.05f, 2.0f, 0.2f, "", "Speed of blend oscillation (cycle time ~10s/value)"); 
    // Speed for palette index oscillation (lower = slower cycle)
    param("color_change_speed", "range", 0.05f, 2.0f, 0.2f, "", "Speed of palette index oscillation (cycle time ~10s/value)"); 
    // Amplitude of palette index oscillation
    param("color_range", "range", 16.0f, 128.0f, 64.0f, "", "Range of palette index oscillation around 128");
    // Density: Controls update frequency and inverse fade
    param("density", "ratio", 0.5f, "clamp", "Sparkle density (more attempts, less fade)"); 
    // Chaos: Scales random wandering influence
    param("chaos", "ratio", 0.2f, "clamp", "Amount of random influence on speeds");

    // --- Initialize State Variables ---
    // Palettes
    palette1 = PixelTheater::Palettes::CloudColors;
    palette2 = PixelTheater::Palettes::HeatColors;

    // Palette Index State (start at midpoint, no speed)
    pos1 = 128.0f;
    pos2 = 128.0f;
    speed1 = 0.0f;
    speed2 = 0.0f;
    index1 = 128;
    index2 = 128;

    // Color Blend State (start at midpoint, no speed)
    blend_pos = 128.0f;
    blend_speed = 0.0f;
    color_blend = 128;

    // Chaos State (start neutral)
    chaos_offset = 0.0f;
    chaos_target = 0.0f;
    chaos_timer = millis(); // Start timer now
}

void Sparkles::tick() {
    Scene::tick(); // Base tick increments tick_count() and calculates deltaTime()
    float dt = deltaTime(); // Get time delta for physics updates
    uint32_t now = millis();

    // --- Get Parameters ---
    float blend_speed_param = settings["blend_cycle_speed"]; 
    float color_speed_param = settings["color_change_speed"];
    float color_range_param = settings["color_range"];
    float density_param = settings["density"];
    float chaos_param = settings["chaos"];

    // --- Update Chaos Offset ---
    // Pick a new target periodically (e.g., every 2-5 seconds)
    if (now > chaos_timer) {
        chaos_target = randomFloat(-1.0f, 1.0f); // New target between -1 and 1
        chaos_timer = now + random(2000, 5000); // Set next update time
    }
    // Smoothly move chaos_offset towards chaos_target
    // Adjust speed based on distance, simple lerp for now
    float chaos_delta = chaos_target - chaos_offset;
    chaos_offset += chaos_delta * 0.5f * dt; // Adjust multiplier for responsiveness
    // Apply the chaos parameter scaling
    float current_chaos = chaos_offset * chaos_param;

    // --- Update Color Blend Oscillation ---
    // Spring force towards midpoint (128), scaled by speed param
    float blend_force = (128.0f - blend_pos) * 0.1f * blend_speed_param; // Adjust spring constant (0.1f)
    // Add chaos influence
    blend_force += current_chaos * 20.0f; // Adjust chaos scaling factor (20.0f)
    // Apply force to speed
    blend_speed += blend_force * dt;
    // Apply damping to speed
    blend_speed *= (1.0f - 0.9f * dt); // Adjust damping factor (0.9f)
    // Update position
    blend_pos += blend_speed * dt;
    // Clamp position to valid range
    blend_pos = std::clamp(blend_pos, 0.0f, 255.0f);
    // Update the uint8_t blend value
    color_blend = static_cast<uint8_t>(blend_pos);

    // --- Update Palette Index Oscillations ---
    // Spring force for pos1
    float force1 = (128.0f - pos1) * 0.1f * color_speed_param;
    force1 += current_chaos * 20.0f; // Add chaos (same direction for now)
    speed1 += force1 * dt;
    speed1 *= (1.0f - 0.9f * dt); // Damping
    pos1 += speed1 * dt;
    // Clamp position (e.g., prevent excessive overshoot if needed, though range mapping handles limits)
    // pos1 = std::clamp(pos1, 0.0f, 255.0f); // Optional clamping

    // Spring force for pos2 (use inverse chaos for variety)
    float force2 = (128.0f - pos2) * 0.1f * color_speed_param;
    force2 -= current_chaos * 20.0f; // Subtract chaos 
    speed2 += force2 * dt;
    speed2 *= (1.0f - 0.9f * dt); // Damping
    pos2 += speed2 * dt;
    // pos2 = std::clamp(pos2, 0.0f, 255.0f); // Optional clamping

    // Calculate actual indices based on position and range
    float scaled_range = color_range_param / 2.0f;
    index1 = static_cast<uint8_t>(std::clamp(pos1, 128.0f - scaled_range, 128.0f + scaled_range));
    index2 = static_cast<uint8_t>(std::clamp(pos2, 128.0f - scaled_range, 128.0f + scaled_range));

    // --- Calculate Momentary Colors ---
    CRGB color1 = colorFromPalette(palette1, index1, 255, PixelTheater::LINEARBLEND);
    CRGB color2 = colorFromPalette(palette2, index2, 255, PixelTheater::LINEARBLEND);

    // --- Apply Pixel Updates ---
    int num_leds = ledCount();
    if (num_leds == 0) return; // No LEDs to update

    // Calculate number of updates based on density (e.g., density * 2 * num_leds attempts per frame)
    int num_updates = static_cast<int>(density_param * 2.0f * num_leds);
    num_updates = std::max(10, num_updates); // Ensure a minimum number of updates
    
    uint8_t blend_amount = 5; // Fixed small blend amount for nblend

    for (int i = 0; i < num_updates; ++i) {
        int led_idx = random(num_leds); // Pick a random LED
        // Choose color based on color_blend probability
        if (random(256) < color_blend) { // Higher blend value = higher chance of color1
            nblend(leds[led_idx], color1, blend_amount);
        } else {
            nblend(leds[led_idx], color2, blend_amount);
        }
    }

    // --- Fade All LEDs ---
    // Fade amount inversely proportional to density (higher density = less fade)
    uint8_t fade_amount = static_cast<uint8_t>(map(density_param, 0.0f, 1.0f, 25.0f, 2.0f));
    fade_amount = std::clamp(fade_amount, (uint8_t)1, (uint8_t)50); // Clamp fade amount

    for (int i = 0; i < num_leds; ++i) {
        leds[i].fadeToBlackBy(fade_amount);
    }
}

std::string Sparkles::status() const {
    // Use snprintf for safe formatting (avoid fmt dependency)
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Blend:%3d | Idx1:%3d Pos1:%.1f | Idx2:%3d Pos2:%.1f | Chaos:%.2f",
             color_blend,
             index1, pos1,
             index2, pos2,
             chaos_offset // Show the raw offset, not scaled by param
             );
    return std::string(buffer);
} 