#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/stage.h"
#include "PixelTheater/core/crgb.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model Platform Integration") {
    TEST_CASE("Model Setup") {
        Stage stage;
        stage.setPlatform(std::make_unique<NativePlatform>(100));
        
        SUBCASE("Model Initialization") {
            auto model = std::make_unique<BasicPentagonModel>();
            stage.setModel(std::move(model));
            
            CHECK(stage.model() != nullptr);
            CHECK(stage.model()->numLeds() <= stage.numLeds());
        }
        
        SUBCASE("LED Array Sharing") {
            auto model = std::make_unique<BasicPentagonModel>();
            stage.setModel(std::move(model));
            
            // Write through model
            stage.model()->leds[0] = CRGB::Red;
            
            // Verify through platform
            auto* platform_leds = stage.leds();
            CHECK(platform_leds[0].r == 255);
            CHECK(platform_leds[0].g == 0);
            CHECK(platform_leds[0].b == 0);
        }
    }
    
    TEST_CASE("Face Operations") {
        Stage stage;
        stage.setPlatform(std::make_unique<NativePlatform>(100));
        auto model = std::make_unique<BasicPentagonModel>();
        stage.setModel(std::move(model));
        
        SUBCASE("Face LED Access") {
            // Write through face
            auto& face = stage.model()->faces[0];
            face.leds[0] = CRGB::Green;
            
            // Verify through platform
            auto* platform_leds = stage.leds();
            CHECK(platform_leds[face.led_offset()].g == 255);
        }
        
        SUBCASE("Face Boundaries") {
            auto& face0 = stage.model()->faces[0];
            auto& face1 = stage.model()->faces[1];
            
            // Set different colors on adjacent faces
            face0.leds[face0.led_count() - 1] = CRGB::Red;
            face1.leds[0] = CRGB::Blue;
            
            // Verify colors are distinct
            auto* platform_leds = stage.leds();
            CHECK(platform_leds[face0.led_offset() + face0.led_count() - 1].r == 255);
            CHECK(platform_leds[face1.led_offset()].b == 255);
        }
    }
    
    TEST_CASE("Platform Operations") {
        Stage stage;
        stage.setPlatform(std::make_unique<NativePlatform>(100));
        auto model = std::make_unique<BasicPentagonModel>();
        stage.setModel(std::move(model));
        
        SUBCASE("Clear Operation") {
            // Write some colors
            stage.model()->faces[0].leds[0] = CRGB::Red;
            stage.model()->faces[1].leds[0] = CRGB::Green;
            
            // Clear through stage
            stage.clear();
            
            // Verify all LEDs are black
            auto* platform_leds = stage.leds();
            for(size_t i = 0; i < stage.numLeds(); i++) {
                CHECK(platform_leds[i].r == 0);
                CHECK(platform_leds[i].g == 0);
                CHECK(platform_leds[i].b == 0);
            }
        }
    }
} 