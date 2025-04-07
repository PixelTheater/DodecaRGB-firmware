#include <doctest/doctest.h>
#include "PixelTheater.h" // Main header
#include "../fixtures/models/basic_pentagon_model.h" // Fixture for counts etc.
#include <string> 
#include <algorithm> // std::min

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

// --- Refactored Test Scenes --- 

// Use TestableScene derived class again
class TestableScene : public Scene { 
public:
    TestableScene() = default;
    bool setup_called = false;
    void setup() override { setup_called = true; }
    void tick() override { Scene::tick(); } 
    void call_connect(IModel& m, ILedBuffer& l, Platform& p) { connect(m, l, p); }
    // Expose tick count directly for testing
    size_t getTickCountDirectly() const { return _tick_count; }
};

// Test basic LED access via helpers
// (Derive other test scenes similarly if needed, or keep them simple)
class LEDTestScene : public TestableScene { 
public:
    LEDTestScene() = default;
    void setup() override {}
    void tick() override {
        TestableScene::tick(); 
        size_t count = ledCount();
        if (count > 0) {
             led(0) = CRGB::Red;
             leds[count - 1] = CRGB::Blue; // Correct index
        }
    }
};

// Test more complex access patterns (fading)
class AccessTestScene : public Scene { 
public:
    AccessTestScene() = default;
    void setup() override {}
    void tick() override {
        Scene::tick();  
        // Simulate some effect using point data
        if (ledCount() > 5) {
            const auto& p = model().point(5);
            leds[5].b = map(p.z(), -1.0f, 1.0f, 0.0f, 255.0f); // Color based on Z
        }
        // Fade all
        for(size_t i = 0; i < ledCount(); ++i) {
            fadeToBlackBy(led(i), 128); 
        }
    }
    void call_connect(IModel& m, ILedBuffer& l, Platform& p) { connect(m, l, p); }
};

// Test setting metadata
class MetadataTestScene : public TestableScene { 
public:
    MetadataTestScene() = default;
    void setup() override {
        set_name("Test Scene");
        set_description("A test scene with metadata");
        set_version("1.0");
        set_author("Test Author");
    }
    void tick() override { TestableScene::tick(); }
};

// --- Test Cases (Manual Setup) --- 

TEST_SUITE("Scene (Refactored Tests - Manual Setup)") {
    
    TEST_CASE("Scene Lifecycle") {
        // Manual Fixture Setup
        auto platform = std::make_unique<NativePlatform>(BasicPentagonModel::LED_COUNT);
        BasicPentagonModel model_def_instance;
        auto concrete_model = std::make_unique<Model<BasicPentagonModel>>(
            model_def_instance, platform->getLEDs()
        );
        auto leds_wrapper = std::make_unique<LedBufferWrapper>(platform->getLEDs(), platform->getNumLEDs());
        auto model_wrapper = std::make_unique<ModelWrapper<BasicPentagonModel>>(std::move(concrete_model));
        TestableScene test_scene; // Use derived class
        test_scene.call_connect(*model_wrapper, *leds_wrapper, *platform);
        test_scene.reset(); 
        test_scene.setup(); 

        CHECK(test_scene.setup_called);
        size_t count_val = test_scene.getTickCountDirectly(); 
        CHECK(count_val == 0); 
        
        test_scene.tick(); 
        count_val = test_scene.getTickCountDirectly();
        CHECK(count_val == 1); 
    }

    TEST_CASE("LED Access") {
        // Manual Fixture Setup (use LEDTestScene)
        auto platform = std::make_unique<NativePlatform>(BasicPentagonModel::LED_COUNT);
        BasicPentagonModel model_def_instance;
        auto concrete_model = std::make_unique<Model<BasicPentagonModel>>(model_def_instance, platform->getLEDs());
        auto leds_wrapper = std::make_unique<LedBufferWrapper>(platform->getLEDs(), platform->getNumLEDs());
        auto model_wrapper = std::make_unique<ModelWrapper<BasicPentagonModel>>(std::move(concrete_model));
        LEDTestScene test_scene; // Use derived class
        test_scene.call_connect(*model_wrapper, *leds_wrapper, *platform);
        test_scene.reset();
        test_scene.setup();
        
        CHECK(platform->getLEDs()[0] == CRGB::Black);
        test_scene.tick();
        CHECK(platform->getLEDs()[0] == CRGB::Red);
        CHECK(platform->getLEDs()[BasicPentagonModel::LED_COUNT - 1] == CRGB::Blue);
    }

    TEST_CASE("Scene Metadata") {
        // Manual Fixture Setup (use MetadataTestScene)
        auto platform = std::make_unique<NativePlatform>(BasicPentagonModel::LED_COUNT);
        BasicPentagonModel model_def_instance;
        auto concrete_model = std::make_unique<Model<BasicPentagonModel>>(model_def_instance, platform->getLEDs());
        auto leds_wrapper = std::make_unique<LedBufferWrapper>(platform->getLEDs(), platform->getNumLEDs());
        auto model_wrapper = std::make_unique<ModelWrapper<BasicPentagonModel>>(std::move(concrete_model));
        MetadataTestScene test_scene;
        test_scene.call_connect(*model_wrapper, *leds_wrapper, *platform);
        test_scene.reset();
        test_scene.setup();

        CHECK(test_scene.name() == "Test Scene");
        CHECK(test_scene.description() == "A test scene with metadata");
        CHECK(test_scene.version() == "1.0");
        CHECK(test_scene.author() == "Test Author");
    }

} 