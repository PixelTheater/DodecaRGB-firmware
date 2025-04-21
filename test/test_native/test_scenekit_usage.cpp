#include "doctest/doctest.h"
#include "PixelTheater/SceneKit.h" // Include the header under test
// Include the model fixture relative to the test file location
#include "../fixtures/models/basic_pentagon_model.h" 

// Remove include for non-existent helper
// #include "../helpers/scene_test_fixture.h" 

// Need these for manual setup
#include "PixelTheater/core/led_buffer_wrapper.h"
#include "PixelTheater/core/model_wrapper.h"
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"

#include <memory> // std::unique_ptr
#include <cmath> // For std::acos etc.

// Remove usage of non-existent fixture
// using SceneFixture = PixelTheater::Testing::SceneTestFixture<PixelTheater::Fixtures::BasicPentagonModel>;

// Use namespaces directly
using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

namespace Scenes {

// Define a minimal scene inheriting from the SceneKit-provided Scene alias
class SceneKitTestScene : public Scene {
public:
    SceneKitTestScene() = default;

    // Add public helper to call protected connect
    void call_connect(IModel& m, ILedBuffer& l, Platform& p) {
        connect(m, l, p);
    }

    void setup() override {
        // Use aliased metadata setters
        set_name("SceneKit Usage Test");
        set_author("Test Suite");

        // Use aliased parameter definition
        param("test_param", "ratio", 0.5f, "clamp", "Test Ratio");
    }

    void tick() override {
        Scene::tick(); // Call base tick

        // --- Test various SceneKit features ---

        // Parameter access
        float p_val = settings["test_param"];
        CHECK(p_val >= 0.0f); // Basic check

        // LED access and color types (CRGB, CHSV)
        size_t num_leds = ledCount();
        if (num_leds > 0) {
            leds[0] = CRGB::Red; // Aliased CRGB struct
            leds[0] = CHSV(100, 200, 150); // Aliased CHSV struct
            leds[0].fadeToBlackBy(10); // Aliased member function

            // Check CRGB += operator
            leds[0] = CRGB::Red;
            leds[0] += CRGB(0, 50, 0); // Add some green
            CHECK(leds[0].r == 255);
            CHECK(leds[0].g == 50);
        }

        // Model access
        const auto& m = model(); // Base model() method
        if (m.pointCount() > 0) {
            const auto& p0 = m.point(0); // Access Point
            float x = p0.x();
            CHECK(x == x); // Dummy check
        }
        float radius = m.getSphereRadius(); // Test new method
        CHECK(radius > 0.0f);

        // Timing
        uint32_t ms = millis();
        float dt = deltaTime();
        CHECK(ms >= 0); // Basic checks
        CHECK(dt >= 0.0f);

        // Math/Random
        float r1 = randomFloat();
        int r2 = random(100);
        float mapped = map(50, 0, 100, 0, 1); // Aliased map
        CHECK(r1 >= 0.0f);
        CHECK(r2 >= 0);
        CHECK(mapped == mapped); // Dummy check

        // Check lerp8by8 alias
        uint8_t lerped = lerp8by8(0, 255, 128); // Use unqualified alias
        CHECK(lerped >= 127); // Approx check for midpoint
        CHECK(lerped <= 129);

        // Constants
        float angle = PT_PI / 2.0f; // Aliased constant
        CHECK(angle > 1.5f);

        // Utilities
        if (num_leds > 1) {
            nblend(leds[1], CRGB::Blue, 128); // Correct: nblend modifies leds[1] in place
        }
        CRGB palette_color = colorFromPalette(
            PixelTheater::Palettes::PartyColors, // Namespace needed for Palette enum
            100, 200, PixelTheater::LINEARBLEND // Namespace needed for Blend enum
        );
        CHECK(palette_color.r >= 0); // Basic check

        // Logging (just check compilation)
        logInfo("SceneKit Test: Info log %d", 1);
        logWarning("SceneKit Test: Warning log %.2f", 3.14f);
        logError("SceneKit Test: Error log %s", "test");

    }
};

} // namespace Scenes

// --- Test Case --- 
// Change to simple TEST_CASE and manually setup environment
TEST_CASE("SceneKit usage compiles and runs") {
    // Manual Fixture Setup (similar to test_scene.cpp)
    auto platform = std::make_unique<NativePlatform>(BasicPentagonModel::LED_COUNT);
    auto concrete_model = std::make_unique<Model<BasicPentagonModel>>(
        platform->getLEDs()
    );
    auto leds_wrapper = std::make_unique<LedBufferWrapper>(platform->getLEDs(), platform->getNumLEDs());
    auto model_wrapper = std::make_unique<ModelWrapper<BasicPentagonModel>>(std::move(concrete_model));
    
    // Create an instance of the test scene
    Scenes::SceneKitTestScene test_scene;
    
    // Manually connect the scene using fixture's components via helper method
    test_scene.call_connect(*model_wrapper, *leds_wrapper, *platform);

    // Run setup and a few ticks to exercise the code
    test_scene.setup();
    test_scene.tick();
    test_scene.tick();
} 