#include "identify_sides_scene.h"
#include "PixelTheater/SceneKit.h"
#include <cmath>
#include <algorithm>
#include <vector>

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

    // --- Edge Detection Diagnostics ---
    logInfo("=== EDGE DETECTION DIAGNOSTICS ===");
    logInfo("Model faces: %zu", model().faceCount());
    
    // Test edge detection for more faces to see connectivity pattern
    for (size_t face_idx = 0; face_idx < std::min(static_cast<size_t>(6), model().faceCount()); face_idx++) {
        const auto& face = model().face(face_idx);
        uint8_t num_edges = model().face_edge_count(face_idx);
        
        logInfo("Face %zu: %d edges, %d vertices, %d LEDs", 
                face_idx, num_edges, face.vertices.size(), face.led_count());
        
        // Check ALL edges for this face to see if any have connectivity
        int connected_edges = 0;
        for (uint8_t edge_idx = 0; edge_idx < num_edges; edge_idx++) {
            int8_t adjacent_face = model().face_at_edge(face_idx, edge_idx);
            if (adjacent_face >= 0) connected_edges++;
            if (edge_idx < 2) { // Only log first 2 to avoid spam
                logInfo("  Edge %d -> Face %d", edge_idx, adjacent_face);
                
                if (face.vertices.size() > edge_idx) {
                    const auto& v0 = face.vertices[edge_idx];
                    const auto& v1 = face.vertices[(edge_idx + 1) % face.vertices.size()];
                    
                    float edge_center_x = (v0.x + v1.x) / 2.0f;
                    float edge_center_y = (v0.y + v1.y) / 2.0f;
                    float edge_center_z = (v0.z + v1.z) / 2.0f;
                    
                    logInfo("    Edge center: (%.2f, %.2f, %.2f)", edge_center_x, edge_center_y, edge_center_z);
                    
                    // Find closest LED to this edge
                    float min_distance = 1000000.0f;
                    uint16_t closest_led = 0;
                    
                    for (uint16_t face_led_idx = 0; face_led_idx < face.led_count(); face_led_idx++) {
                        uint16_t global_led_idx = face.led_offset() + face_led_idx;
                        const auto& led_point = model().point(global_led_idx);
                        
                        float dx = led_point.x() - edge_center_x;
                        float dy = led_point.y() - edge_center_y;
                        float dz = led_point.z() - edge_center_z;
                        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
                        
                        if (distance < min_distance) {
                            min_distance = distance;
                            closest_led = global_led_idx;
                        }
                    }
                    
                    logInfo("    Closest LED: %d (distance: %.2f)", closest_led, min_distance);
                }
            }
        }
        logInfo("  Face %zu: %d/%d edges have connectivity", face_idx, connected_edges, num_edges);
    }
    
    logInfo("CONNECTIVITY SUMMARY: If all faces show 0 connected edges, the model edge data is missing!");
    logInfo("=== END DIAGNOSTICS ===");
    
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
    
    // Create highly distinct face colors for clear visual identification
    CRGB face_colors[12];  // DodecaRGBv2 has 12 faces
    
    // Hand-picked palette with maximum visual distinction
    static const CRGB distinct_colors[12] = {
        CRGB::Red,           // 0: Bright red
        CRGB::Lime,          // 1: Bright green  
        CRGB::Blue,          // 2: Bright blue
        CRGB::Yellow,        // 3: Bright yellow
        CRGB::Magenta,       // 4: Bright magenta
        CRGB::Cyan,          // 5: Bright cyan
        CRGB::Orange,        // 6: Orange
        CRGB::Purple,        // 7: Purple
        CRGB::White,         // 8: White
        CRGB::Pink,          // 9: Pink
        CRGB(0, 128, 0),     // 10: Dark green
        CRGB(139, 69, 19)    // 11: Saddle brown
    };
    
    for (size_t face_id = 0; face_id < model().faceCount(); face_id++) {
        face_colors[face_id] = distinct_colors[face_id % 12];
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
    
    // Light edges using simplified geometric edge detection (separate pass to avoid overwrites)
    static uint32_t last_edge_debug = 0;
    uint32_t current_time = millis();
    bool debug_edges = (current_time - last_edge_debug > 5000); // Debug every 5 seconds
    
    int total_edge_leds_lit = 0;
    
    for (size_t geometric_pos = 0; geometric_pos < total_faces; geometric_pos++) {
        const auto& face = model().face(geometric_pos);
        
        // Get edge information using IModel interface methods
        uint8_t num_edges = model().face_edge_count(geometric_pos);
        
        for (uint8_t edge_idx = 0; edge_idx < num_edges; edge_idx++) {
            // Get the adjacent face for this edge
            int8_t adjacent_geometric_pos = model().face_at_edge(geometric_pos, edge_idx);
            
            // FIXED: Smart edge coloring without requiring connectivity data
            CRGB edge_color;
            uint8_t edge_face_id = 0;  // Which face color to use for this edge
            
            if (adjacent_geometric_pos >= 0 && adjacent_geometric_pos < static_cast<int8_t>(total_faces)) {
                // Perfect case: use actual connectivity data
                edge_color = face_colors[adjacent_geometric_pos];
                edge_face_id = adjacent_geometric_pos;
                edge_color.nscale8(brightness_factor);
                
                // Apply pulsing if this edge color matches the current pulsing face
                if (static_cast<uint8_t>(adjacent_geometric_pos) == current_pulsing_color) {
                    edge_color.nscale8(static_cast<uint8_t>(pulse_factor * 255.0f));
                }
            } else {
                // Fallback: Create meaningful edge colors using geometric logic
                // Use a different face color for each edge to create visible variety
                edge_face_id = (geometric_pos + edge_idx + 1) % total_faces;
                edge_color = face_colors[edge_face_id];
                edge_color.nscale8(brightness_factor);
                
                // Apply pulsing if this calculated edge color matches current pulsing face
                if (edge_face_id == current_pulsing_color) {
                    edge_color.nscale8(static_cast<uint8_t>(pulse_factor * 255.0f));
                }
            }
                
            // Always light edges now - we have smart fallback coloring
            if (true) {
                
                // 🔥 USE PRECISE EDGES DATA: Perfect edge centers, no rotation issues!
                bool edge_found = false;
                float edge_center_x = 0.0f, edge_center_y = 0.0f, edge_center_z = 0.0f;
                
                // Calculate edge center directly from face vertices
                // This is the most reliable approach since vertices are always available
                const auto& face = model().face(geometric_pos);
                if (face.vertices.size() > edge_idx && face.vertices.size() > 0) {
                    const auto& v0 = face.vertices[edge_idx];
                    const auto& v1 = face.vertices[(edge_idx + 1) % face.vertices.size()];
                    
                    edge_center_x = (v0.x + v1.x) / 2.0f;
                    edge_center_y = (v0.y + v1.y) / 2.0f;
                    edge_center_z = (v0.z + v1.z) / 2.0f;
                    edge_found = true;
                    
                    if (debug_edges && geometric_pos == 0 && edge_idx < 2) {
                        logInfo("  USING face vertices: (%.2f, %.2f, %.2f)", 
                                edge_center_x, edge_center_y, edge_center_z);
                    }
                }
                

                
                if (edge_found) {
                    // Find LEDs on this face closest to the edge center
                    struct LedDistance {
                        float distance;
                        uint16_t led_index;
                    };
                    std::vector<LedDistance> led_distances;
                    led_distances.reserve(face.led_count());
                    
                    // Check each LED on this face
                    for (uint16_t face_led_idx = 0; face_led_idx < face.led_count(); face_led_idx++) {
                        uint16_t global_led_idx = face.led_offset() + face_led_idx;
                        const auto& led_point = model().point(global_led_idx);
                        
                        // Calculate distance from LED to edge center
                        float dx = led_point.x() - edge_center_x;
                        float dy = led_point.y() - edge_center_y;
                        float dz = led_point.z() - edge_center_z;
                        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
                        
                        led_distances.push_back({distance, global_led_idx});
                    }
                    
                    // Sort by distance to edge center (closest first)
                    std::sort(led_distances.begin(), led_distances.end(),
                             [](const LedDistance& a, const LedDistance& b) {
                                 return a.distance < b.distance;
                             });
                    
                    // Light edge LEDs based on the color source face ID for visual identification
                    uint8_t num_leds_to_light;
                    if (adjacent_geometric_pos >= 0) {
                        // Use actual connectivity: N = adjacent_face_id + 1
                        num_leds_to_light = static_cast<uint8_t>(adjacent_geometric_pos + 1);
                    } else {
                        // Use calculated face color: N = edge_face_id + 1 (for visual identification)
                        num_leds_to_light = static_cast<uint8_t>(edge_face_id + 1);
                    }
                    size_t leds_to_use = std::min(static_cast<size_t>(num_leds_to_light), led_distances.size());
                    
                    if (debug_edges && geometric_pos == 0 && edge_idx == 0) {
                        const char* color_source = (adjacent_geometric_pos >= 0) ? "connected" : "calculated";
                        logInfo("Edge debug: Face %zu Edge %d -> Face %d (%s), using color from face %d, lighting %zu LEDs", 
                                geometric_pos, edge_idx, adjacent_geometric_pos, color_source, edge_face_id, leds_to_use);
                        if (leds_to_use > 0) {
                            logInfo("  LED %d (dist %.2f), color R:%d G:%d B:%d", 
                                    led_distances[0].led_index, led_distances[0].distance,
                                    edge_color.r, edge_color.g, edge_color.b);
                            logInfo("  Edge center: (%.2f, %.2f, %.2f)", edge_center_x, edge_center_y, edge_center_z);
                            
                            // Additional diagnostic: check LED spread
                            if (led_distances.size() >= 5) {
                                logInfo("  LED distances (first 5): %.2f, %.2f, %.2f, %.2f, %.2f", 
                                        led_distances[0].distance, led_distances[1].distance, 
                                        led_distances[2].distance, led_distances[3].distance, 
                                        led_distances[4].distance);
                            }
                        }
                    }
                    
                    for (size_t i = 0; i < leds_to_use; i++) {
                        uint16_t led_idx = led_distances[i].led_index;
                        leds[led_idx] = edge_color;
                        total_edge_leds_lit++;
                    }
                }
            }
        }
    }
    
    if (debug_edges) {
        logInfo("Total edge LEDs lit this frame: %d", total_edge_leds_lit);
        last_edge_debug = current_time;
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