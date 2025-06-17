#include "identify_sides_scene.h"
#include "PixelTheater/SceneKit.h"
#include "PixelTheater/model/model.h"
#include "models/DodecaRGBv2_1/model.h"
#include <cmath>
#include <algorithm>

namespace Scenes {

void IdentifySidesScene::setup() {
    // --- Define Metadata ---
    set_name("Identify Sides");
    set_author("DodecaRGB");
    set_description("Identifies each face with unique colors and patterns for alignment and configuration.");
    set_version("1.0");

    // --- Define Parameters ---
    param("Speed", "ratio", 0.0f, 2.0f, IdentifySidesScene::DEFAULT_SPEED, "clamp", "Animation speed (0=Static, 2=Fast)");
    param("Brightness", "ratio", 0.1f, 1.0f, IdentifySidesScene::DEFAULT_BRIGHTNESS, "clamp", "Overall brightness");

    logInfo("IdentifySidesScene setup complete");
}

void IdentifySidesScene::tick() {
    float speed = settings["Speed"];
    float brightness = settings["Brightness"];
    
    // Get brightness factor (0-255)
    uint8_t brightness_factor = static_cast<uint8_t>(brightness * 255.0f);
    
    // Clear all LEDs first
    for (size_t i = 0; i < ledCount(); i++) {
        leds[i] = CRGB::Black;
    }
    
    // Create face colors array for easy lookup 
    CRGB face_colors[12];  // DodecaRGBv2 has 12 faces
    for (size_t face_id = 0; face_id < model().faceCount(); face_id++) {
        // Create unique color for each face using HSV for even distribution
        uint8_t hue = static_cast<uint8_t>((face_id * 255) / model().faceCount());
        face_colors[face_id] = CHSV(hue, 255, 150);  // Full saturation, medium brightness
    }
    
    // Color cycling animation: slowly loop through all colors
    // Each color pulses for 3 seconds at 60 BPM
    float time_seconds = millis() / 1000.0f;
    float color_duration = 3.0f;  // 3 seconds per color
    uint8_t total_faces = model().faceCount();
    uint8_t current_pulsing_color = static_cast<uint8_t>(fmod(time_seconds / color_duration, total_faces));
    
    // 60 BPM = 1 beat per second
    float pulse_freq = 1.0f * speed;  // Use speed parameter to control pulse rate
    float pulse_phase = time_seconds * pulse_freq * 2.0f * 3.14159f;  // Convert to radians
    float pulse_factor = 0.3f + 0.7f * (0.5f + 0.5f * sinf(pulse_phase));  // Pulse between 30% and 100%
    

    
    // Light ALL faces with their colors, applying pulse to the current color
    for (size_t geometric_pos = 0; geometric_pos < total_faces; geometric_pos++) {
        auto face = model().face(geometric_pos);  // Access by geometric position
        
        // Get the base color for this geometric position
        CRGB face_color = face_colors[geometric_pos];
        face_color.nscale8(brightness_factor);
        
        // Apply pulsing if this is the current pulsing geometric position
        if (geometric_pos == current_pulsing_color) {
            face_color.nscale8(static_cast<uint8_t>(pulse_factor * 255.0f));
        }
        
        // Light the first N LEDs where N = geometric_pos + 1 for identification
        // This ensures geometric position 0 shows 1 dot, position 1 shows 2 dots, etc.
        size_t leds_to_light = std::min(static_cast<size_t>(geometric_pos + 1), static_cast<size_t>(face.led_count()));
        for (size_t led_idx = 0; led_idx < leds_to_light; led_idx++) {
            face.leds[led_idx] = face_color;
        }
    }
    
    // Light ALL edges using adjacency information from the model
    for (size_t geometric_pos = 0; geometric_pos < total_faces; geometric_pos++) {
        auto face = model().face(geometric_pos);
        
        // Light edges for this face using proper edge groups from YAML
        uint8_t num_edges = model().face_edge_count(geometric_pos);
        for (int edge_idx = 0; edge_idx < num_edges; edge_idx++) {
            // Get the adjacent face for this edge using face-centric API
            int8_t adjacent_geometric_pos = model().face_at_edge(geometric_pos, edge_idx);
                
            if (adjacent_geometric_pos >= 0 && adjacent_geometric_pos < static_cast<int8_t>(total_faces)) {
                // Use the adjacent face's color 
                CRGB edge_color = face_colors[adjacent_geometric_pos];
                edge_color.nscale8(brightness_factor);
                
                // Apply pulsing if this edge color matches the current pulsing face
                if (static_cast<uint8_t>(adjacent_geometric_pos) == current_pulsing_color) {
                    edge_color.nscale8(static_cast<uint8_t>(pulse_factor * 255.0f));
                }
                
                // Light edge LEDs using model-defined groups (clean interface API)
                // Get the edge group name (e.g., "edge0", "edge1", etc.)
                char edge_group_name[16];
                snprintf(edge_group_name, sizeof(edge_group_name), "edge%d", edge_idx);
                
                // Use the clean IModel interface - no casting required!
                auto edge_group = model().face_group(geometric_pos, edge_group_name);
                
                // Light all LEDs in this edge group using model definition
                if (edge_group && edge_group->size() > 0) {
                    for (size_t led_idx = 0; led_idx < edge_group->size(); led_idx++) {
                        (*edge_group)[led_idx] = edge_color;
                    }
                } else {
                    // Graceful fallback: Log warning but continue
                    // This happens if the edge group isn't defined in the model YAML
                    logWarning("Edge group '%s' not found for geometric pos %d - skipping edge lighting", edge_group_name, geometric_pos);
                }
            }
        }
    }
}

std::string IdentifySidesScene::status() const {
    float speed = settings["Speed"];
    float brightness = settings["Brightness"];
    
    // Calculate which face is currently pulsing
    float time_seconds = millis() / 1000.0f;
    float color_duration = 3.0f;
    uint8_t total_faces = model().faceCount();
    uint8_t current_pulsing_face = static_cast<uint8_t>(fmod(time_seconds / color_duration, total_faces));
    
    char buffer[128];
    snprintf(buffer, sizeof(buffer), 
             "Pulsing Face: %d/%zu | Pulse: %.1f BPM | Brightness: %.2f",
             current_pulsing_face, model().faceCount(), speed * 60.0f, brightness);
    return std::string(buffer);
}

} // namespace Scenes 