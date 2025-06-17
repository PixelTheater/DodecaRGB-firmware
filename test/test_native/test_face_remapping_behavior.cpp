#include <doctest/doctest.h>
#include "PixelTheater/model/model.h"

// Include the generated test fixtures
#include "fixtures/models/test_no_remap/model.h"
#include "fixtures/models/test_with_remap/model.h"

// Include the real model to test
#include "models/DodecaRGBv2_1/model.h"

using namespace PixelTheater;
using namespace PixelTheater::Models;

TEST_CASE("Face Remapping Behavior - Geometric vs Logical Access") {
    
    SUBCASE("No Remapping: Geometric position matches logical face") {
        CRGB test_leds[12];  // 4 faces * 3 LEDs each
        Model<PyramidNoRemap> model(test_leds);
        
        // Clear all LEDs
        for (int i = 0; i < 12; i++) {
            test_leds[i] = CRGB::Black;
        }
        
        // Test that model().face(0) accesses geometric position 0 (which is logical face 0)
        auto face_0 = model.face(0);
        CHECK(face_0.id() == 0);  // Should be logical face 0
        CHECK(face_0.led_offset() == 0);  // Should use LEDs 0-2
        CHECK(face_0.led_count() == 3);
        
        // Set color on geometric position 0's first LED  
        face_0.leds()[0] = CRGB::Red;
        
        // Without remapping: geometric position 0 → logical face 0 → LED 0
        CHECK(test_leds[0] == CRGB::Red);
        CHECK(test_leds[3] == CRGB::Black);  // Other faces unaffected
        CHECK(test_leds[6] == CRGB::Black);
        CHECK(test_leds[9] == CRGB::Black);
    }
    
    SUBCASE("With Remapping: Geometric position maps to correct logical face") {
        CRGB test_leds[12];  // 4 faces * 3 LEDs each
        Model<PyramidWithRemap> model(test_leds);
        
        // Clear all LEDs
        for (int i = 0; i < 12; i++) {
            test_leds[i] = CRGB::Black;
        }
        
        // Test: model().face(0) should access geometric position 0
        // In our remapped model: geometric position 0 → logical face 2 → LEDs 6-8
        auto face_at_pos_0 = model.face(0);
        CHECK(face_at_pos_0.id() == 2);  // Should be logical face 2
        CHECK(face_at_pos_0.led_offset() == 6);  // Should use LEDs 6-8
        CHECK(face_at_pos_0.led_count() == 3);
        
        // Set color on geometric position 0's first LED
        face_at_pos_0.leds()[0] = CRGB::Blue;
        
        // With remapping: geometric position 0 → logical face 2 → LED 6
        CHECK(test_leds[0] == CRGB::Black);  // Logical face 0's LEDs unaffected
        CHECK(test_leds[3] == CRGB::Black);  // Logical face 1's LEDs unaffected  
        CHECK(test_leds[6] == CRGB::Blue);   // Logical face 2's LED 0 is set (geometric pos 0)
        CHECK(test_leds[9] == CRGB::Black);  // Logical face 3's LEDs unaffected
        
        // Test another geometric position
        // model().face(2) should access geometric position 2 → logical face 0 → LEDs 0-2
        auto face_at_pos_2 = model.face(2);
        CHECK(face_at_pos_2.id() == 0);  // Should be logical face 0
        CHECK(face_at_pos_2.led_offset() == 0);  // Should use LEDs 0-2
        
        face_at_pos_2.leds()[0] = CRGB::Green;
        
        // geometric position 2 → logical face 0 → LED 0
        CHECK(test_leds[0] == CRGB::Green);  // Now logical face 0's LED is set
        CHECK(test_leds[6] == CRGB::Blue);   // Previous setting still there
    }
    
    SUBCASE("Complete Remapping Verification: All geometric positions") {
        CRGB test_leds[12];
        Model<PyramidWithRemap> model(test_leds);
        
        // Clear all LEDs
        for (int i = 0; i < 12; i++) {
            test_leds[i] = CRGB::Black;
        }
        
        // Expected mapping from our YAML:
        // geometric pos 0 → logical face 2 → LEDs 6-8
        // geometric pos 1 → logical face 3 → LEDs 9-11
        // geometric pos 2 → logical face 0 → LEDs 0-2
        // geometric pos 3 → logical face 1 → LEDs 3-5
        
        CRGB colors[] = {CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow};
        
        for (uint8_t geometric_pos = 0; geometric_pos < 4; geometric_pos++) {
            auto face = model.face(geometric_pos);
            
            // Set unique color on this geometric position's first LED
            face.leds()[0] = colors[geometric_pos];
        }
        
        // Verify physical LED pattern matches expected remapping
        CHECK(test_leds[0] == CRGB::Blue);    // Geometric pos 2 → logical face 0 → LED 0
        CHECK(test_leds[3] == CRGB::Yellow);  // Geometric pos 3 → logical face 1 → LED 3
        CHECK(test_leds[6] == CRGB::Red);     // Geometric pos 0 → logical face 2 → LED 6
        CHECK(test_leds[9] == CRGB::Green);   // Geometric pos 1 → logical face 3 → LED 9
        
        // Other LEDs should remain black
        CHECK(test_leds[1] == CRGB::Black);
        CHECK(test_leds[2] == CRGB::Black);
        CHECK(test_leds[4] == CRGB::Black);
        CHECK(test_leds[5] == CRGB::Black);
        CHECK(test_leds[7] == CRGB::Black);
        CHECK(test_leds[8] == CRGB::Black);
        CHECK(test_leds[10] == CRGB::Black);
        CHECK(test_leds[11] == CRGB::Black);
    }
}

TEST_CASE("Identify Sides Scene - Remapping Behavior") {
    
    SUBCASE("Scene should produce different LED patterns with remapping") {
        // Test with no-remap model
        CRGB no_remap_leds[12];
        {
            Model<PyramidNoRemap> model(no_remap_leds);
            
            // Clear LEDs
            for (int i = 0; i < 12; i++) {
                no_remap_leds[i] = CRGB::Black;
            }
            
            // Simulate the identify_sides scene pattern: light geometric_pos + 1 LEDs per face
            for (size_t geometric_pos = 0; geometric_pos < model.faceCount(); geometric_pos++) {
                auto face = model.face(static_cast<uint8_t>(geometric_pos));
                
                // Light geometric_pos + 1 LEDs (geometric pos 0 = 1 LED, pos 1 = 2 LEDs, etc.)
                size_t leds_to_light = geometric_pos + 1;
                for (size_t led_idx = 0; led_idx < leds_to_light && led_idx < face.led_count(); led_idx++) {
                    face.leds()[led_idx] = CRGB::White;
                }
            }
        }
        
        // Test with remap model - should produce DIFFERENT physical LED pattern
        CRGB remap_leds[12];
        {
            Model<PyramidWithRemap> model(remap_leds);
            
            // Clear LEDs
            for (int i = 0; i < 12; i++) {
                remap_leds[i] = CRGB::Black;
            }
            
            // Same identify_sides pattern - but now accessing by geometric position
            for (size_t geometric_pos = 0; geometric_pos < model.faceCount(); geometric_pos++) {
                auto face = model.face(static_cast<uint8_t>(geometric_pos));
                
                size_t leds_to_light = geometric_pos + 1;
                for (size_t led_idx = 0; led_idx < leds_to_light && led_idx < face.led_count(); led_idx++) {
                    face.leds()[led_idx] = CRGB::White;
                }
            }
        }
        
        // Verify no-remap pattern (geometric pos matches logical face)
        // Geometric pos 0 = 1 LED, pos 1 = 2 LEDs, pos 2 = 3 LEDs, pos 3 = 3 LEDs
        CHECK(no_remap_leds[0] == CRGB::White);   // Face 0, LED 0
        CHECK(no_remap_leds[1] == CRGB::Black);   // Face 0, LED 1 (not lit)
        CHECK(no_remap_leds[2] == CRGB::Black);   // Face 0, LED 2 (not lit)
        
        CHECK(no_remap_leds[3] == CRGB::White);   // Face 1, LED 0
        CHECK(no_remap_leds[4] == CRGB::White);   // Face 1, LED 1
        CHECK(no_remap_leds[5] == CRGB::Black);   // Face 1, LED 2 (not lit)
        
        CHECK(no_remap_leds[6] == CRGB::White);   // Face 2, LED 0
        CHECK(no_remap_leds[7] == CRGB::White);   // Face 2, LED 1
        CHECK(no_remap_leds[8] == CRGB::White);   // Face 2, LED 2
        
        CHECK(no_remap_leds[9] == CRGB::White);   // Face 3, LED 0
        CHECK(no_remap_leds[10] == CRGB::White);  // Face 3, LED 1
        CHECK(no_remap_leds[11] == CRGB::White);  // Face 3, LED 2
        
        // Verify remap pattern (geometric pos maps to different logical faces)
        // Geometric pos 0 → logical face 2 → LEDs 6-8 (1 LED lit)
        // Geometric pos 1 → logical face 3 → LEDs 9-11 (2 LEDs lit)
        // Geometric pos 2 → logical face 0 → LEDs 0-2 (3 LEDs lit)
        // Geometric pos 3 → logical face 1 → LEDs 3-5 (3 LEDs lit) 
        CHECK(remap_leds[0] == CRGB::White);   // Logical face 0, LED 0 (geometric pos 2, 3 LEDs)
        CHECK(remap_leds[1] == CRGB::White);   // Logical face 0, LED 1 (geometric pos 2, 3 LEDs)
        CHECK(remap_leds[2] == CRGB::White);   // Logical face 0, LED 2 (geometric pos 2, 3 LEDs)
        
        CHECK(remap_leds[3] == CRGB::White);   // Logical face 1, LED 0 (geometric pos 3, 3 LEDs)
        CHECK(remap_leds[4] == CRGB::White);   // Logical face 1, LED 1 (geometric pos 3, 3 LEDs)
        CHECK(remap_leds[5] == CRGB::White);   // Logical face 1, LED 2 (geometric pos 3, 3 LEDs)
        
        CHECK(remap_leds[6] == CRGB::White);   // Logical face 2, LED 0 (geometric pos 0, 1 LED)
        CHECK(remap_leds[7] == CRGB::Black);   // Logical face 2, LED 1 (geometric pos 0, not lit)
        CHECK(remap_leds[8] == CRGB::Black);   // Logical face 2, LED 2 (geometric pos 0, not lit)
        
        CHECK(remap_leds[9] == CRGB::White);   // Logical face 3, LED 0 (geometric pos 1, 2 LEDs)
        CHECK(remap_leds[10] == CRGB::White);  // Logical face 3, LED 1 (geometric pos 1, 2 LEDs)
        CHECK(remap_leds[11] == CRGB::Black);  // Logical face 3, LED 2 (geometric pos 1, not lit)
        
        // Verify the patterns are indeed different
        bool patterns_different = false;
        for (int i = 0; i < 12; i++) {
            if (no_remap_leds[i] != remap_leds[i]) {
                patterns_different = true;
                break;
            }
        }
        CHECK(patterns_different);  // Patterns should be different due to remapping
    }
}

TEST_CASE("DodecaRGBv2_1 Real Model - Face Remapping Debug") {
    
    SUBCASE("Debug actual remapping behavior in real model") {
        CRGB test_leds[1620];  // DodecaRGBv2_1 LED count
        Model<DodecaRGBv2_1> model(test_leds);
        
        // Clear all LEDs
        for (int i = 0; i < 1620; i++) {
            test_leds[i] = CRGB::Black;
        }
        
        // Test geometric position 0 (top of model where user sees 3 dots)
        auto face_at_pos_0 = model.face(0);
        
        // Check which logical face is at geometric position 0
        CHECK(face_at_pos_0.id() == 2);  // Should be logical face 2 based on YAML
        CHECK(face_at_pos_0.led_offset() == 270);  // Face 2 offset: 2 * 135 = 270
        CHECK(face_at_pos_0.led_count() == 135);   // Pentagon has 135 LEDs
        
        // Test geometric position 2 
        auto face_at_pos_2 = model.face(2);
        
        // Check which logical face is at geometric position 2
        CHECK(face_at_pos_2.id() == 0);  // Should be logical face 0 based on YAML
        CHECK(face_at_pos_2.led_offset() == 0);    // Face 0 offset: 0 * 135 = 0
        CHECK(face_at_pos_2.led_count() == 135);   // Pentagon has 135 LEDs
        
        // Simulate identify_sides scene behavior
        // Geometric position 0 should light 1 LED (position + 1)
        face_at_pos_0.leds()[0] = CRGB::Red;
        
        // Check that LED 270 is lit (first LED of logical face 2)
        CHECK(test_leds[270] == CRGB::Red);   // Face 2's first LED
        CHECK(test_leds[0] == CRGB::Black);   // Face 0's first LED not lit
        
        // Geometric position 2 should light 3 LEDs (position + 1)
        for (int i = 0; i < 3; i++) {
            face_at_pos_2.leds()[i] = CRGB::Blue;
        }
        
        // Check that LEDs 0, 1, 2 are lit (first 3 LEDs of logical face 0)
        CHECK(test_leds[0] == CRGB::Blue);    // Face 0's LED 0
        CHECK(test_leds[1] == CRGB::Blue);    // Face 0's LED 1  
        CHECK(test_leds[2] == CRGB::Blue);    // Face 0's LED 2
        CHECK(test_leds[270] == CRGB::Red);   // Face 2's LED still red
    }
    
    SUBCASE("Verify expected identify_sides behavior") {
        CRGB test_leds[1620];
        Model<DodecaRGBv2_1> model(test_leds);
        
        // Clear all LEDs
        for (int i = 0; i < 1620; i++) {
            test_leds[i] = CRGB::Black;
        }
        
        // Simulate identify_sides logic for first 4 positions
        for (int geometric_pos = 0; geometric_pos < 4; geometric_pos++) {
            auto face = model.face(geometric_pos);
            int leds_to_light = geometric_pos + 1;
            
            for (int led_idx = 0; led_idx < leds_to_light; led_idx++) {
                face.leds()[led_idx] = CRGB::White;
            }
        }
        
        // Verify results based on remapping:
        // Geometric pos 0 → logical face 2 → LEDs 270+ (1 LED lit)
        // Geometric pos 1 → logical face 1 → LEDs 135+ (2 LEDs lit)  
        // Geometric pos 2 → logical face 0 → LEDs 0+ (3 LEDs lit)
        // Geometric pos 3 → logical face 3 → LEDs 405+ (4 LEDs lit)
        
        // Check geometric position 0 → logical face 2 (1 LED)
        CHECK(test_leds[270] == CRGB::White);  // Face 2, LED 0
        CHECK(test_leds[271] == CRGB::Black);  // Face 2, LED 1 (not lit)
        
        // Check geometric position 1 → logical face 1 (2 LEDs)  
        CHECK(test_leds[135] == CRGB::White);  // Face 1, LED 0
        CHECK(test_leds[136] == CRGB::White);  // Face 1, LED 1
        CHECK(test_leds[137] == CRGB::Black);  // Face 1, LED 2 (not lit)
        
        // Check geometric position 2 → logical face 0 (3 LEDs)
        CHECK(test_leds[0] == CRGB::White);    // Face 0, LED 0
        CHECK(test_leds[1] == CRGB::White);    // Face 0, LED 1
        CHECK(test_leds[2] == CRGB::White);    // Face 0, LED 2
        CHECK(test_leds[3] == CRGB::Black);    // Face 0, LED 3 (not lit)
        
        // Check geometric position 3 → logical face 3 (4 LEDs)
        CHECK(test_leds[405] == CRGB::White);  // Face 3, LED 0 (3 * 135 = 405)
        CHECK(test_leds[406] == CRGB::White);  // Face 3, LED 1
        CHECK(test_leds[407] == CRGB::White);  // Face 3, LED 2
        CHECK(test_leds[408] == CRGB::White);  // Face 3, LED 3
        CHECK(test_leds[409] == CRGB::Black);  // Face 3, LED 4 (not lit)
    }
} 