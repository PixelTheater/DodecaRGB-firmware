#include <doctest/doctest.h>
#include <FastLED.h>
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/model/point.h"
#include "../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;
// Don't use 'using namespace' for FastLED to avoid collisions

// Helper function to convert FastLED CRGB to PixelTheater CRGB
inline PixelTheater::CRGB toPixelTheater(const ::CRGB& fastled_color) {
    return PixelTheater::CRGB(fastled_color.r, fastled_color.g, fastled_color.b);
}

TEST_CASE("LED Model Hardware") {
    Serial.println("\n=== Testing LED Model Hardware ===");

    SUBCASE("Model Construction") {
        Serial.println("Testing model construction...");
        
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);
        
        CHECK(model.led_count() == BasicPentagonModel::LED_COUNT);
        CHECK(model.faces.size() == BasicPentagonModel::FACE_COUNT);
        
        // Verify all LEDs start black
        bool all_black = true;
        for(size_t i = 0; i < model.led_count(); i++) {
            const auto& led = model.leds[i];
            if(led.r != 0 || led.g != 0 || led.b != 0) {
                all_black = false;
                Serial.printf("LED %d not black: R=%d, G=%d, B=%d\n",
                            i, led.r, led.g, led.b);
                break;
            }
        }
        CHECK(all_black);
        Serial.println("Initial LED state verified");
        
        // Verify point data
        CHECK(model.points[0].face_id() == def.POINTS[0].face_id);
        CHECK(model.points[0].x() == def.POINTS[0].x);
        CHECK(model.points[0].y() == def.POINTS[0].y);
        CHECK(model.points[0].z() == def.POINTS[0].z);
        Serial.println("Point data verified");
    }
    
    SUBCASE("Face Operations") {
        Serial.println("Testing face operations...");
        
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);
        
        // Test first face
        auto& face0 = model.faces[0];
        CHECK(face0.id() == 0);
        CHECK(face0.led_count() == 5);  // Pentagon has 5 LEDs
        CHECK(face0.led_offset() == 0);
        
        // Write through face LED access using FastLED colors
        face0.leds[0] = PixelTheater::CRGB(255, 0, 0);    // Red (center)
        face0.leds[2] = PixelTheater::CRGB(0, 255, 0);    // Green (ring)
        face0.leds[4] = PixelTheater::CRGB(0, 0, 255);    // Blue (edge)
        FastLED.show();
        
        // Verify through global access
        CHECK(model.leds[0].r == 255);
        CHECK(model.leds[0].g == 0);
        CHECK(model.leds[0].b == 0);
        
        CHECK(model.leds[2].r == 0);
        CHECK(model.leds[2].g == 255);
        CHECK(model.leds[2].b == 0);
        
        CHECK(model.leds[4].r == 0);
        CHECK(model.leds[4].g == 0);
        CHECK(model.leds[4].b == 255);
        
        Serial.println("Face LED access verified");
        
        // Test face fill operation
        fill_solid(face0.leds.data(), face0.led_count(), PixelTheater::CRGB(255, 255, 255));  // White
        FastLED.show();
        
        for(size_t i = 0; i < face0.led_count(); i++) {
            CHECK(face0.leds[i].r == 255);
            CHECK(face0.leds[i].g == 255);
            CHECK(face0.leds[i].b == 255);
        }
        Serial.println("Face fill operation verified");
    }
    
    SUBCASE("LED Groups") {
        Serial.println("Testing LED groups...");
        
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);
        auto& face = model.faces[0];
        
        // Test center group
        face.leds[0] = PixelTheater::CRGB(255, 0, 0);  // Red center
        FastLED.show();
        CHECK(model.leds[0].r == 255);
        CHECK(model.leds[0].g == 0);
        CHECK(model.leds[0].b == 0);
        
        // Test ring group
        for(int i = 1; i <= 4; i++) {
            face.leds[i] = PixelTheater::CRGB(0, 0, 255);  // Blue ring
        }
        FastLED.show();
        
        // Verify ring LEDs
        for(int i = 1; i <= 4; i++) {
            CHECK(model.leds[i].r == 0);
            CHECK(model.leds[i].g == 0);
            CHECK(model.leds[i].b == 255);
        }
        
        Serial.println("LED groups verified");
    }
    
    SUBCASE("Multi-face Patterns") {
        Serial.println("Testing patterns across faces...");
        
        BasicPentagonModel def;
        Model<BasicPentagonModel> model(def);
        
        // Set each face to a different color
        fill_solid(model.faces[0].leds, model.faces[0].led_count(), PixelTheater::CRGB(255, 0, 0));    // Red
        fill_solid(model.faces[1].leds, model.faces[1].led_count(), PixelTheater::CRGB(0, 255, 0));    // Green
        fill_solid(model.faces[2].leds, model.faces[2].led_count(), PixelTheater::CRGB(0, 0, 255));    // Blue
        FastLED.show();
        
        // Verify face colors
        for(size_t i = 0; i < model.faces[0].led_count(); i++) {
            // Face 0 - Red
            CHECK(model.faces[0].leds[i].r == 255);
            CHECK(model.faces[0].leds[i].g == 0);
            CHECK(model.faces[0].leds[i].b == 0);
            
            // Face 1 - Green
            CHECK(model.faces[1].leds[i].r == 0);
            CHECK(model.faces[1].leds[i].g == 255);
            CHECK(model.faces[1].leds[i].b == 0);
            
            // Face 2 - Blue
            CHECK(model.faces[2].leds[i].r == 0);
            CHECK(model.faces[2].leds[i].g == 0);
            CHECK(model.faces[2].leds[i].b == 255);
        }
        
        // Clear all LEDs
        for(auto& face : model.faces) {
            fill_solid(face.leds, face.led_count(), PixelTheater::CRGB(0, 0, 0));  // Black
        }
        FastLED.show();
        
        Serial.println("Multi-face patterns verified");
    }
    
    Serial.println("LED model tests complete!");
} 