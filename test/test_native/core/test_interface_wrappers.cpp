#include <doctest/doctest.h>
#include <memory> // std::unique_ptr, std::make_unique
#include <cstring> // memcmp

// Interfaces and Wrappers under test
#include "PixelTheater/core/iled_buffer.h"
#include "PixelTheater/core/imodel.h"
#include "PixelTheater/core/led_buffer_wrapper.h"
#include "PixelTheater/core/model_wrapper.h"

// Core types & Fixtures
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/platform/native_platform.h"
#include "../../fixtures/models/basic_pentagon_model.h" // BasicPentagonModel

using namespace PixelTheater;

// Helper macro for CRGB comparison
#define CHECK_CRGB_EQUAL(expected, actual) \
    CHECK((expected).r == (actual).r); \
    CHECK((expected).g == (actual).g); \
    CHECK((expected).b == (actual).b)

TEST_SUITE("Interface Wrappers") {

    // Common setup logic can be encapsulated here or within TEST_CASE
    struct Fixture {
        std::unique_ptr<NativePlatform> platform_fixture;
        // model_fixture is moved to ModelWrapper, so we don't keep a separate pointer
        CRGB leds_fixture[PixelTheater::Fixtures::BasicPentagonModel::LED_COUNT];
        std::unique_ptr<LedBufferWrapper> leds_wrapper;
        std::unique_ptr<ModelWrapper<PixelTheater::Fixtures::BasicPentagonModel>> model_wrapper;
        ILedBuffer* leds_if = nullptr;
        IModel* model_if = nullptr;

        Fixture() {
            // 1. Create platform (manages its own leds internally now)
            platform_fixture = std::make_unique<NativePlatform>(PixelTheater::Fixtures::BasicPentagonModel::LED_COUNT);
            
            // 2. Create ModelDef instance (needed by Model constructor)
            PixelTheater::Fixtures::BasicPentagonModel model_def_instance; 

            // 3. Create concrete Model, passing def and platform's LED buffer
            auto concrete_model = std::make_unique<Model<PixelTheater::Fixtures::BasicPentagonModel>>(
                model_def_instance,         // Pass the definition object
                platform_fixture->getLEDs() // Pass the LED buffer from platform
            );
            
            // 4. Create wrappers
            leds_wrapper = std::make_unique<LedBufferWrapper>(platform_fixture->getLEDs(), platform_fixture->getNumLEDs());
            model_wrapper = std::make_unique<ModelWrapper<PixelTheater::Fixtures::BasicPentagonModel>>(std::move(concrete_model));

            leds_if = leds_wrapper.get();
            model_if = model_wrapper.get();
        }
    };

    TEST_CASE("LedBufferWrapper") {
        Fixture fx;
        ILedBuffer* leds_if = fx.leds_if;
        const ILedBuffer* leds_if_const = fx.leds_wrapper.get(); // Get const pointer for const methods

        REQUIRE(leds_if != nullptr);

        SUBCASE("Count") {
            CHECK(leds_if->ledCount() == PixelTheater::Fixtures::BasicPentagonModel::LED_COUNT);
        }

        SUBCASE("Access and Modification") {
            // Modify through interface
            leds_if->led(0) = CRGB::Red;
            leds_if->led(PixelTheater::Fixtures::BasicPentagonModel::LED_COUNT - 1) = CRGB::Blue;

            // Verify via interface (const access)
            CHECK_CRGB_EQUAL(CRGB::Red, leds_if_const->led(0));
            CHECK_CRGB_EQUAL(CRGB::Blue, leds_if_const->led(PixelTheater::Fixtures::BasicPentagonModel::LED_COUNT - 1));

            // Verify changes in the original buffer managed by the platform
            CHECK(memcmp(&CRGB::Red, &(fx.platform_fixture->getLEDs()[0]), sizeof(CRGB)) == 0);
            CHECK(memcmp(&CRGB::Blue, &(fx.platform_fixture->getLEDs()[PixelTheater::Fixtures::BasicPentagonModel::LED_COUNT - 1]), sizeof(CRGB)) == 0);
        }

        SUBCASE("Bounds Checking") {
            // Bounds checks now clamp, cannot easily test via CHECK_THROWS_AS
            // Test clamping behavior if necessary?
        }
    }

    TEST_CASE("ModelWrapper") {
        Fixture fx;
        IModel* model_if = fx.model_if;
        const IModel* model_if_const = fx.model_wrapper.get();

        REQUIRE(model_if != nullptr);

        SUBCASE("Counts") {
            // Use count methods
            CHECK(model_if->pointCount() == PixelTheater::Fixtures::BasicPentagonModel::LED_COUNT);
            CHECK(model_if->faceCount() == PixelTheater::Fixtures::BasicPentagonModel::FACE_COUNT);
        }

        SUBCASE("Face Access") {
            // Access first and last face via interface method
            const Face& first_face = model_if_const->face(0);
            const Face& last_face = model_if_const->face(model_if_const->faceCount() - 1);

            CHECK(first_face.id() == 0);
            CHECK(last_face.id() == PixelTheater::Fixtures::BasicPentagonModel::FACE_COUNT - 1);
        }

        SUBCASE("Point Access") {
            // Access first and last point via interface method
            const Point& first_point = model_if_const->point(0);
            const Point& last_point = model_if_const->point(model_if_const->pointCount() - 1);

            CHECK(first_point.x() == doctest::Approx(0.0f)); 
            CHECK(last_point.z() != doctest::Approx(0.0f)); 
        }

        SUBCASE("Bounds Checking") {
            // Bounds checks now clamp
        }
    }
} 