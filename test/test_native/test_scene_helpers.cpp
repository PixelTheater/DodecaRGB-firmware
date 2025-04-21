#include <doctest/doctest.h>
#include <memory> // std::unique_ptr, std::make_unique
#include <cstring> // memcmp

// Class under test
#include "PixelTheater/SceneKit.h" // For scene access + unqualified helpers

// Interfaces and Wrappers needed for testing
#include "PixelTheater/core/iled_buffer.h"
#include "PixelTheater/core/imodel.h"
#include "PixelTheater/core/led_buffer_wrapper.h"
#include "PixelTheater/core/model_wrapper.h"

// Core types & Fixtures
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/model/model.h" // Concrete model
#include "PixelTheater/platform/native_platform.h" // Concrete platform
#include "fixtures/models/basic_pentagon_model.h" // Fixture ModelDef

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

// Helper macro for CRGB comparison
#define CHECK_CRGB_EQUAL(expected, actual) \
    CHECK((expected).r == (actual).r); \
    CHECK((expected).g == (actual).g); \
    CHECK((expected).b == (actual).b)

// --- Test Fixture Setup --- 

// Use a derived class to access protected/private Scene methods if needed
class TestableScene : public Scene {
public:
    // Expose connect publicly for testing fixture
    void call_connect(IModel& m, ILedBuffer& l, Platform& p) {
        connect(m, l, p); // Calls protected connect method
    }
    // Provide empty implementations for pure virtual methods
    void setup() override {}
};

struct SceneHelperFixture {
    std::unique_ptr<NativePlatform> platform;
    std::unique_ptr<LedBufferWrapper> leds_wrapper;
    std::unique_ptr<ModelWrapper<BasicPentagonModel>> model_wrapper;
    TestableScene test_scene; // Use TestableScene instance
    
    ILedBuffer* leds_if = nullptr;
    IModel* model_if = nullptr;

    SceneHelperFixture() {
        // 1. Create platform
        platform = std::make_unique<NativePlatform>(BasicPentagonModel::LED_COUNT);
        
        // 2. Create concrete Model 
        auto concrete_model = std::make_unique<Model<BasicPentagonModel>>(
            platform->getLEDs()
        );
        
        // 4. Create wrappers (these will be passed to scene.connect)
        leds_wrapper = std::make_unique<LedBufferWrapper>(platform->getLEDs(), platform->getNumLEDs());
        model_wrapper = std::make_unique<ModelWrapper<BasicPentagonModel>>(std::move(concrete_model));

        leds_if = leds_wrapper.get();
        model_if = model_wrapper.get();

        // 5. Connect the scene to the interfaces (now allowed via friend declaration)
        test_scene.call_connect(*model_if, *leds_if, *platform);
    }
};

TEST_SUITE("Scene Helpers") {

    TEST_CASE_FIXTURE(SceneHelperFixture, "LED Access Helpers") {
        REQUIRE(test_scene.ledCount() == BasicPentagonModel::LED_COUNT);
        REQUIRE(test_scene.leds.size() == BasicPentagonModel::LED_COUNT);

        // Modify via helper
        test_scene.led(0) = CRGB::Red;
        test_scene.leds[1] = CRGB::Green; // Test proxy access

        // Verify via const helper
        const TestableScene& const_scene = test_scene;
        CHECK_CRGB_EQUAL(CRGB::Red, const_scene.led(0));
        CHECK_CRGB_EQUAL(CRGB::Green, const_scene.leds[1]); // Test const proxy access

        // Verify underlying buffer
        CHECK_CRGB_EQUAL(CRGB::Red, platform->getLEDs()[0]);
        CHECK_CRGB_EQUAL(CRGB::Green, platform->getLEDs()[1]);

        // Check bounds (using proxy) - Bounds checks now clamp
        // CHECK_THROWS_AS(test_scene.leds[BasicPentagonModel::LED_COUNT], std::out_of_range); 
    }

    TEST_CASE_FIXTURE(SceneHelperFixture, "Model Access Helpers") {
        // Test via model() method
        REQUIRE(test_scene.model().faceCount() == BasicPentagonModel::FACE_COUNT);
        REQUIRE(test_scene.model().pointCount() == BasicPentagonModel::LED_COUNT);

        // Access via model() and interface methods
        const Point& p0 = test_scene.model().point(0);
        const Face& f0 = test_scene.model().face(0);
        const Face& last_face = test_scene.model().face(test_scene.model().faceCount() - 1);

        CHECK(p0.x() == doctest::Approx(0.0f));
        CHECK(f0.id() == 0);
        CHECK(last_face.id() == BasicPentagonModel::FACE_COUNT - 1);

        // Check bounds - Clamping is handled by ModelWrapper implementation
        // const Point& p_out = test_scene.model().point(test_scene.model().pointCount());
        // const Face& f_out = test_scene.model().face(test_scene.model().faceCount());
        // CHECK(/* clamping worked */);
    }

    TEST_CASE_FIXTURE(SceneHelperFixture, "Timing Helpers") {
        // Check values returned by NativePlatform's dummy implementations
        CHECK(test_scene.deltaTime() == doctest::Approx(1.0f / 60.0f));
        uint32_t m1 = test_scene.millis();
        CHECK(m1 >= 0); // Check it's non-negative (could be 0 initially)
        // Could add a delay and check again, but depends on platform implementation
    }
    
    TEST_CASE_FIXTURE(SceneHelperFixture, "Random Helpers") {
        // Can't check exact values, but check ranges or basic operation
        uint8_t r8 = test_scene.random8();
        uint16_t r16 = test_scene.random16();
        uint32_t r_max = test_scene.random(1000);
        uint32_t r_min_max = test_scene.random(10, 20);
        float rf01 = test_scene.randomFloat();
        float rf_max = test_scene.randomFloat(50.0f);
        float rf_min_max = test_scene.randomFloat(-10.0f, 10.0f);

        CHECK(r_max < 1000);
        CHECK(r_min_max >= 10);
        CHECK(r_min_max < 20);
        CHECK(rf01 >= 0.0f);
        CHECK(rf01 <= 1.0f);
        CHECK(rf_max >= 0.0f);
        CHECK(rf_max <= 50.0f);
        CHECK(rf_min_max >= -10.0f);
        CHECK(rf_min_max <= 10.0f);
    }

    TEST_CASE_FIXTURE(SceneHelperFixture, "SceneKit Utilities Access") {
        // Verify access to functions brought in by SceneKit.h using unqualified names
        // We don't need to re-test the logic, just compilation/linking.
        
        // Easing Function Check (Use Scenes:: prefix)
        float eased_val = Scenes::outQuad(0.0f, 1.0f, 0.5f);
        float eased_frac = Scenes::outQuadF(0.5f);
        CHECK(eased_val == doctest::Approx(0.75f).epsilon(0.001f));
        CHECK(eased_frac == doctest::Approx(0.75f).epsilon(0.001f));
        
        // Other SceneKit Check (e.g., map - already tested indirectly but good to be explicit)
        int mapped_val = Scenes::map(50, 0, 100, 0, 200); // Use map directly (implicitly via SceneKit)
        CHECK(mapped_val == 100);
        
        // Color blending check
        CRGB c1 = CRGB::Red;
        Scenes::nblend(c1, CRGB::Blue, 128); // Use nblend directly (implicitly via SceneKit)
        // Check that color changed (approximate check)
        CHECK(c1.r < 200);
        CHECK(c1.b > 50);
    }

    // TEST_CASE_FIXTURE(SceneHelperFixture, "Logging Helpers") {
    //     // How to test printf output easily? 
    //     // Could redirect stdout, but complex for simple test.
    //     // For now, just call them to ensure they compile and don't crash.
    //     test_scene.logInfo("Info test");
    //     test_scene.logWarning("Warning test");
    //     test_scene.logError("Error test");
    // }
} 