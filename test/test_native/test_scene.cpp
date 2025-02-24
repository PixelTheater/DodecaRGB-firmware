#include <doctest/doctest.h>
#include "PixelTheater/scene.h"
#include "../helpers/stage_test_fixture.h"
#include "../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Testing;
using namespace PixelTheater::Fixtures;

// Test fixture scene
template<typename ModelDef>
class TestScene : public Scene<ModelDef> {
public:
    using Scene<ModelDef>::Scene;
    void setup() override { setup_called = true; }
    bool setup_called{false};
};

// LED test scene
template<typename ModelDef>
class LEDTestScene : public Scene<ModelDef> {
public:
    using Scene<ModelDef>::Scene;
    
    void setup() override {}
    
    void tick() override {
        Scene<ModelDef>::tick();  // Call base to increment counter
        
        // Get LED array from stage
        auto& leds = this->stage.leds;
        leds[0] = CRGB::Red;
        auto last_idx = this->stage.model.led_count() - 1;
        leds[last_idx] = CRGB::Blue;
    }
};

// Access pattern test scene
template<typename ModelDef>
class AccessTestScene : public Scene<ModelDef> {
public:
    using Scene<ModelDef>::Scene;
    
    void setup() override {}
    
    void tick() override {
        Scene<ModelDef>::tick();  // Call base to increment counter
        
        // Set different LEDs than the test case
        this->stage.leds[5] = CRGB::Purple;  // Different index than test checks
        this->stage.model.faces[1].leds[3] = CRGB::Yellow;  // Different face
        
        // Range-based iteration with free function
        for(auto& led : this->stage.leds) {
            fadeToBlackBy(led, 128);
        }
    }
};

TEST_SUITE("Scene") {
    TEST_CASE_FIXTURE(StageTestFixture<BasicPentagonModel>, "Scene Lifecycle") {
        auto* scene = stage->addScene<TestScene<BasicPentagonModel>>(*stage);
        stage->setScene(scene);
        
        SUBCASE("Setup") {
            scene->setup();
            CHECK(scene->setup_called);
        }

        SUBCASE("Tick Counter") {
            CHECK(scene->tick_count() == 0);
            stage->update();
            CHECK(scene->tick_count() == 1);
        }
    }

    TEST_CASE_FIXTURE(StageTestFixture<BasicPentagonModel>, "LED Access") {
        auto* scene = stage->addScene<LEDTestScene<BasicPentagonModel>>(*stage);
        stage->setScene(scene);
        
        // Initial state should be black
        CHECK(stage->leds[0] == CRGB::Black);
        
        // Run one update cycle
        stage->update();
        
        // Verify LEDs were updated through stage
        CHECK(stage->leds[0] == CRGB::Red);
        CHECK(stage->leds[BasicPentagonModel::LED_COUNT - 1] == CRGB::Blue);
    }

    TEST_CASE_FIXTURE(StageTestFixture<BasicPentagonModel>, "Stage Access Patterns") {
        SUBCASE("LED Array Access") {
            // Direct indexing
            stage->leds[0] = CRGB::Blue;
            CHECK(stage->leds[0] == CRGB::Blue);
            
            // Bounds checking
            stage->leds[9999] = CRGB::Red;  // Should clamp to last LED
            CHECK(stage->leds[BasicPentagonModel::LED_COUNT - 1] == CRGB::Red);
            
            // Range-based for
            for(auto& led : stage->leds) {
                led = CRGB::Green;
            }
            CHECK(stage->leds[0] == CRGB::Green);
        }
        
        SUBCASE("Model Access") {
            // Face access through model
            stage->model.faces[0].leds[0] = CRGB::Blue;
            CHECK(stage->leds[0] == CRGB::Blue);  // Same LED
            
            // Model methods work
            CHECK(stage->model.face_count() == BasicPentagonModel::FACE_COUNT);
        }
        
        SUBCASE("Scene Integration") {
            auto* scene = stage->addScene<AccessTestScene<BasicPentagonModel>>(*stage);
            stage->setScene(scene);
            
            // Set test LEDs directly
            stage->leds[0] = CRGB::Red;
            stage->leds[1] = CRGB::Green;
            
            // Verify colors were set
            CHECK(stage->leds[0] == CRGB::Red);
            CHECK(stage->leds[1] == CRGB::Green);
            
            // Now run scene update which will fade different LEDs
            stage->update();
            
            // Verify fading worked on scene's LEDs
            CHECK(stage->leds[5].r <= 128);  // Purple faded
            CHECK(stage->leds[5].b <= 128);  // Purple faded
        }
    }
} 