#include <doctest/doctest.h>
#include <memory> // std::unique_ptr, std::make_unique

// Class under test
#include "PixelTheater/theater.h"

// Include necessary components used by Theater internally (even if indirectly)
#include "PixelTheater/scene.h" // Needed for unique_ptr<Scene>
#include "PixelTheater/platform/platform.h" // Needed for unique_ptr<Platform>
#include "PixelTheater/core/imodel.h"
#include "PixelTheater/core/iled_buffer.h"

// Include concrete types needed for testing initialization
#include "PixelTheater/platform/native_platform.h" 
#include "PixelTheater/platform/web_platform.h" // Include WebPlatform header for native stub tests
#include "PixelTheater/model_def.h" // Base ModelDefinition
#include "fixtures/models/basic_pentagon_model.h" // Concrete ModelDef for testing

// Include a minimal scene for testing addScene etc.
#include "PixelTheater/scene.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures; // For BasicPentagonModel

// Minimal Scene subclass for testing
class MinimalTestScene : public Scene {
public:
    int setup_calls = 0;
    int tick_calls = 0;
    int reset_calls = 0;

    MinimalTestScene() { set_name("Minimal"); }
    void setup() override { setup_calls++; }
    void tick() override { Scene::tick(); tick_calls++; }
    void reset() override { Scene::reset(); reset_calls++; }
};

// Another minimal scene for switching tests
class AnotherMinimalTestScene : public Scene {
public:
    AnotherMinimalTestScene() { set_name("Another"); }
    void setup() override { }
    void tick() override { Scene::tick(); }
};

// --- Restore TheaterTester --- 
class TheaterTester : public Theater {
public:
    // Expose pointers for testing internal state
    Platform* get_platform_ptr() { return platform_.get(); }
    IModel* get_model_ptr() { return model_.get(); }
    ILedBuffer* get_leds_ptr() { return leds_.get(); }
    bool is_initialized() { return initialized_; }
};

// Define a namespace for Theater tests that need friend access
namespace TheaterTesting {

TEST_SUITE("Theater") {

    TEST_CASE("Construction and Destruction") {
        // Test that Theater can be constructed and destructed without errors.
        // This primarily checks that member initialization and cleanup work.
        std::unique_ptr<Theater> theater_ptr;
        
        CHECK_NOTHROW(theater_ptr = std::make_unique<Theater>());
        
        // Resetting the pointer triggers destruction
        CHECK_NOTHROW(theater_ptr.reset()); 
    }

    // Use original test case name
    TEST_CASE("Initialization - useNativePlatform") {
        TheaterTester theater; // Use the tester subclass

        // Check initial state via tester getters
        REQUIRE_FALSE(theater.is_initialized());
        REQUIRE(theater.get_platform_ptr() == nullptr);
        REQUIRE(theater.get_model_ptr() == nullptr);
        REQUIRE(theater.get_leds_ptr() == nullptr);

        // Initialize
        CHECK_NOTHROW(theater.useNativePlatform<BasicPentagonModel>(BasicPentagonModel::LED_COUNT));

        // Check state after initialization via tester getters
        CHECK(theater.is_initialized());
        CHECK(theater.get_platform_ptr() != nullptr);
        CHECK(theater.get_model_ptr() != nullptr);
        CHECK(theater.get_leds_ptr() != nullptr);

        // Check double initialization prevention
        // Get pointers to the initial objects
        Platform* p1 = theater.get_platform_ptr();
        IModel* m1 = theater.get_model_ptr();
        ILedBuffer* l1 = theater.get_leds_ptr();
        // Call again - should log error and return without throwing or changing pointers
        CHECK_NOTHROW(theater.useNativePlatform<BasicPentagonModel>(BasicPentagonModel::LED_COUNT));
        CHECK(theater.get_platform_ptr() == p1); // Pointers should be unchanged
        CHECK(theater.get_model_ptr() == m1);
        CHECK(theater.get_leds_ptr() == l1);
        // Double initialization no longer throws, just returns early
        // CHECK_THROWS_AS(theater.useNativePlatform<BasicPentagonModel>(BasicPentagonModel::LED_COUNT), std::runtime_error);
    }

    // Only compile this test case when building for Web/Emscripten
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
        TEST_CASE("Initialization - useWebPlatform (Web Build)") {
            // Check that useWebPlatform compiles and calls internal_prepare
            // This test will only run in the web build environment.
            MESSAGE("Testing useWebPlatform in web environment.");
            TheaterTester theater;
            REQUIRE_FALSE(theater.is_initialized());

            // WebPlatform header included at top level now

            // Call useWebPlatform - should compile and run using actual implementation
            CHECK_NOTHROW(theater.useWebPlatform<BasicPentagonModel>());
            
            // Check that initialization occurred (via internal_prepare)
            CHECK(theater.is_initialized());
            // Check that the platform pointer is set (should be a WebPlatform)
            CHECK(theater.get_platform_ptr() != nullptr);
            // Verify it's actually a WebPlatform using dynamic_cast
            CHECK(dynamic_cast<WebGL::WebPlatform*>(theater.get_platform_ptr()) != nullptr);
            // Check model and leds wrappers were created
            CHECK(theater.get_model_ptr() != nullptr);
            CHECK(theater.get_leds_ptr() != nullptr);

            // Check double initialization prevention
            Platform* p1 = theater.get_platform_ptr();
            CHECK_NOTHROW(theater.useWebPlatform<BasicPentagonModel>());
            CHECK(theater.get_platform_ptr() == p1);
        }
    #endif // End of Web-specific test case

    // --- Scene Management Tests --- 

    struct TheaterSceneFixture {
        TheaterTester theater;
        size_t led_count = BasicPentagonModel::LED_COUNT;

        TheaterSceneFixture() {
            // Initialize theater before each scene management test case
            theater.useNativePlatform<BasicPentagonModel>(led_count);
        }
    };

    TEST_CASE_FIXTURE(TheaterSceneFixture, "Scene Management - addScene") {
        REQUIRE(theater.sceneCount() == 0);
        REQUIRE(theater.currentScene() == nullptr);
        
        // Add first scene
        theater.addScene<MinimalTestScene>();
        CHECK(theater.sceneCount() == 1);
        CHECK(theater.currentScene() != nullptr); // Should be set to first scene
        CHECK(theater.currentScene() == &theater.scene(0));
        CHECK(theater.scene(0).name() == "Minimal");

        // Check connect was called (indirectly, pointers should be set in scene)
        // Requires adding getters to MinimalTestScene or making fixture friend of Scene
        // MinimalTestScene* scene_ptr = dynamic_cast<MinimalTestScene*>(theater.currentScene());
        // REQUIRE(scene_ptr != nullptr);
        // CHECK(scene_ptr->model_ptr != nullptr); // Assuming model_ptr is made accessible

        // Add second scene
        theater.addScene<AnotherMinimalTestScene>();
        CHECK(theater.sceneCount() == 2);
        CHECK(theater.currentScene() != nullptr); // Should still be first scene
        CHECK(theater.currentScene() == &theater.scene(0)); // Still first
        CHECK(theater.scene(1).name() == "Another");
    }
    
    TEST_CASE_FIXTURE(TheaterSceneFixture, "Scene Management - start/update") {
        theater.addScene<MinimalTestScene>();
        MinimalTestScene* scene_ptr = dynamic_cast<MinimalTestScene*>(&theater.scene(0));
        REQUIRE(scene_ptr != nullptr);

        REQUIRE(scene_ptr->setup_calls == 0);
        REQUIRE(scene_ptr->tick_calls == 0);
        
        // Start should call setup on current (first) scene
        theater.start();
        CHECK(scene_ptr->setup_calls == 1);
        CHECK(scene_ptr->tick_calls == 0);

        // Update should call tick on current scene
        theater.update();
        CHECK(scene_ptr->setup_calls == 1);
        CHECK(scene_ptr->tick_calls == 1);

        theater.update();
        CHECK(scene_ptr->tick_calls == 2);
    }

    TEST_CASE_FIXTURE(TheaterSceneFixture, "Scene Management - next/previousScene") {
        theater.addScene<MinimalTestScene>(); // Scene 0
        theater.addScene<AnotherMinimalTestScene>(); // Scene 1
        MinimalTestScene* scene0_ptr = dynamic_cast<MinimalTestScene*>(&theater.scene(0));
        AnotherMinimalTestScene* scene1_ptr = dynamic_cast<AnotherMinimalTestScene*>(&theater.scene(1));
        REQUIRE(scene0_ptr != nullptr);
        REQUIRE(scene1_ptr != nullptr);

        theater.start(); // Start on Scene 0, calls setup
        REQUIRE(theater.currentScene() == scene0_ptr);
        REQUIRE(scene0_ptr->setup_calls == 1);
        REQUIRE(scene0_ptr->reset_calls == 0);

        // Go next
        theater.nextScene();
        CHECK(theater.currentScene() == scene1_ptr); // Now on Scene 1
        // CHECK(scene1_ptr->setup_calls == 1); // Should call setup on switch
        // CHECK(scene1_ptr->reset_calls == 1); // Should call reset on switch

        // Go next (wrap around)
        theater.nextScene();
        CHECK(theater.currentScene() == scene0_ptr); // Back to Scene 0
        CHECK(scene0_ptr->setup_calls == 2); // Setup called again
        CHECK(scene0_ptr->reset_calls == 1); // Reset called

        // Go previous
        theater.previousScene();
        CHECK(theater.currentScene() == scene1_ptr); // Back to Scene 1
        // CHECK(scene1_ptr->setup_calls == 2); // Setup called again
        // CHECK(scene1_ptr->reset_calls == 2); // Reset called again

         // Go previous (wrap around)
        theater.previousScene();
        CHECK(theater.currentScene() == scene0_ptr); // Back to Scene 0
        CHECK(scene0_ptr->setup_calls == 3); // Setup called again
        CHECK(scene0_ptr->reset_calls == 2); // Reset called again
    }

    TEST_CASE_FIXTURE(TheaterSceneFixture, "Scene Accessors") {
        CHECK(theater.sceneCount() == 0);
        CHECK(theater.currentScene() == nullptr);
        CHECK(theater.scenes().empty());
        CHECK(theater.scene(0).name() == "DummyScene"); // Check name of returned dummy

        theater.addScene<MinimalTestScene>();
        theater.addScene<AnotherMinimalTestScene>();

        CHECK(theater.sceneCount() == 2);
        CHECK_FALSE(theater.scenes().empty());
        CHECK(theater.scenes().size() == 2);
        
        CHECK(theater.currentScene() == &theater.scene(0)); // Current defaults to first
        const TheaterTester& const_theater = theater;
        CHECK(const_theater.currentScene() == &const_theater.scene(0));

        CHECK(theater.scene(0).name() == "Minimal");
        CHECK(const_theater.scene(1).name() == "Another");
        CHECK(theater.scene(2).name() == "DummyScene"); 
    }

}

} // namespace TheaterTesting 