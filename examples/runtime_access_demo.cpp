/**
 * @file runtime_access_demo.cpp
 * @brief Demonstrates the new runtime access methods for LED groups, edges, and metadata
 * 
 * This example shows how to use:
 * - model.group("ring0") - Access LED groups by name
 * - model.edges(n) - Access edge geometry
 * - model.face_at_edge(n) - Get connected faces
 * - model.hardware() - Access hardware metadata
 */

#include "PixelTheater/model/model.h"
#include "src/models/DodecaRGBv2_1/model.h"  // Generated model definition
#include "PixelTheater/core/crgb.h"

using namespace PixelTheater;

void demonstrateRuntimeAccess() {
    // Create LED array and model instance
    CRGB leds[Models::DodecaRGBv2_1::LED_COUNT];
    Model<Models::DodecaRGBv2_1> model(leds);
    
    // ===========================================
    // LED GROUP ACCESS DEMONSTRATION
    // ===========================================
    
    // Access LED groups by name
    auto center_group = model.group("center");
    auto ring0_group = model.group("ring0");
    auto edge0_group = model.group("edge0");
    
    // Set all center LEDs to white across all faces
    for (auto& led : center_group) {
        led = CRGB::White;
    }
    
    // Set ring0 LEDs to blue on all faces
    for (auto& led : ring0_group) {
        led = CRGB::Blue;
    }
    
    // Access LED group for a specific face
    auto face_5_center = model.group("center", 5);
    auto face_5_ring1 = model.group("ring1", 5);
    
    // Set face 5's center to red
    face_5_center[0] = CRGB::Red;
    
    // Animate face 5's ring1 with a rainbow
    for (size_t i = 0; i < face_5_ring1.size(); i++) {
        face_5_ring1[i] = CHSV(i * 25, 255, 255);  // Rainbow colors
    }
    
    // ===========================================
    // EDGE ACCESS DEMONSTRATION
    // ===========================================
    
    // Access edge geometry and relationships
    for (size_t i = 0; i < model.edge_count(); i++) {
        auto edge = model.edges(i);
        
        if (edge.has_connection()) {
            // This edge connects two faces
            int8_t connected_face = model.face_at_edge(i);
            
            // Could use this for edge-based lighting effects
            // For example, highlight edges that connect faces
        }
    }
    
    // Get all edges for face 0
    auto face_0_edges = model.face_edges(0);
    for (const auto& edge : face_0_edges) {
        // Process each edge of face 0
        // Could use edge.start_vertex and edge.end_vertex for geometry calculations
    }
    
    // ===========================================
    // HARDWARE METADATA ACCESS
    // ===========================================
    
    // Access hardware configuration
    auto hw = model.hardware();
    
    // Get LED specifications
    const char* led_type = hw.led_type();           // "WS2812B"
    const char* color_order = hw.color_order();     // "GRB"
    float led_diameter = hw.led_diameter_mm();       // 1.6f
    float led_spacing = hw.led_spacing_mm();         // 4.5f
    
    // Get power specifications
    uint16_t max_current = hw.max_current_per_led_ma();  // 20
    uint16_t avg_current = hw.avg_current_per_led_ma();  // 10
    
    // Use this information for power calculations
    uint16_t total_max_current = max_current * model.led_count();
    uint16_t total_avg_current = avg_current * model.led_count();
    
    // ===========================================
    // MODEL SIZE INFORMATION
    // ===========================================
    
    // Get model dimensions
    size_t total_leds = model.led_count();      // 1620
    size_t total_faces = model.face_count();    // 12
    size_t total_edges = model.edge_count();    // 60 (12 faces × 5 edges)
    size_t total_groups = model.group_count();  // 11 (center, ring0-4, edge0-4)
}

/**
 * @brief Example animation using LED groups
 */
void animateWithGroups(Model<Models::DodecaRGBv2_1>& model, uint32_t time_ms) {
    // Breathing effect on all center LEDs
    uint8_t brightness = sin8(time_ms / 10);
    auto centers = model.group("center");
    for (auto& led : centers) {
        led = CRGB::White;
        led.nscale8(brightness);
    }
    
    // Rotating rainbow on ring0
    auto ring0 = model.group("ring0");
    for (size_t i = 0; i < ring0.size(); i++) {
        uint8_t hue = (time_ms / 20 + i * 51) & 0xFF;  // 51 ≈ 255/5 for good spread
        ring0[i] = CHSV(hue, 255, 255);
    }
    
    // Face-specific effects
    for (uint8_t face_id = 0; face_id < model.face_count(); face_id++) {
        // Each face gets a different color for its outer ring
        auto face_ring4 = model.group("ring4", face_id);
        uint8_t face_hue = (face_id * 21 + time_ms / 100) & 0xFF;  // 21 ≈ 255/12
        
        for (auto& led : face_ring4) {
            led = CHSV(face_hue, 255, 128);
        }
    }
}

/**
 * @brief Example using edge relationships for inter-face effects
 */
void animateEdgeConnections(Model<Models::DodecaRGBv2_1>& model) {
    // Light up edges that connect faces with complementary colors
    for (size_t i = 0; i < model.edge_count(); i++) {
        auto edge = model.edges(i);
        
        if (edge.has_connection()) {
            int8_t connected_face = model.face_at_edge(i);
            
            // Create complementary colors between connected faces
            uint8_t face1_hue = edge.face_id * 21;  // 21 ≈ 255/12
            uint8_t face2_hue = connected_face * 21;
            
            // Light up edge LEDs for both faces
            auto face1_edge = model.group("edge0", edge.face_id);
            auto face2_edge = model.group("edge0", connected_face);
            
            for (auto& led : face1_edge) {
                led = CHSV(face1_hue, 255, 255);
            }
            
            for (auto& led : face2_edge) {
                led = CHSV(face2_hue, 255, 255);
            }
        }
    }
}

int main() {
    // Create model
    CRGB leds[Models::DodecaRGBv2_1::LED_COUNT];
    Model<Models::DodecaRGBv2_1> model(leds);
    
    // Demonstrate basic runtime access
    demonstrateRuntimeAccess();
    
    // Example animations
    uint32_t time_ms = 0;
    
    while (true) {
        // Clear all LEDs
        for (auto& led : model.leds) {
            led = CRGB::Black;
        }
        
        // Run animations
        animateWithGroups(model, time_ms);
        animateEdgeConnections(model);
        
        // In a real application, you would call FastLED.show() here
        // FastLED.show();
        
        time_ms += 16;  // ~60 FPS
        
        // In a real application, add delay here
        // delay(16);
    }
    
    return 0;
} 