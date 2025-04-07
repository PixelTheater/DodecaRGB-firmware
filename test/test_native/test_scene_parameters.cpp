#include <doctest/doctest.h>
#include "PixelTheater.h"
#include <string>
#include <vector>
#include <algorithm> // For std::find

// Reuse the fixture definition from test_scene.cpp (requires careful linking or separate header)
// For simplicity here, let's redefine a similar fixture.
#include "../fixtures/models/basic_pentagon_model.h"
#include "PixelTheater/platform/native_platform.h" 
#include "PixelTheater/core/led_buffer_wrapper.h"
#include "PixelTheater/core/model_wrapper.h"
#include "PixelTheater/model/model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

// --- Refactored Test Scene --- 
class ParamTestScene : public Scene { 
public:
    ParamTestScene() = default;
    
    void setup() override {
        param("brightness", "ratio", 0.8f, "clamp", "Overall brightness");
        param("speed", "range", 0.1f, 2.0f, 1.0f, "", "Animation speed");
        param("color_hue", "count", 0, 255, 0, "wrap", "Base color hue");
        param("enabled", "switch", true, "", "Enable feature");
        param("count", "count", 1, 10, 5, "", "Item count");
    }
    
    void tick() override {
        Scene::tick();
    }

    void call_connect(IModel& m, ILedBuffer& l, Platform& p) { connect(m, l, p); }
};

// --- Test Fixture --- 
template<typename SceneType>
struct ParamSceneFixture {
    std::unique_ptr<NativePlatform> platform;
    std::unique_ptr<LedBufferWrapper> leds_wrapper;
    std::unique_ptr<ModelWrapper<BasicPentagonModel>> model_wrapper;
    SceneType test_scene; 
    ILedBuffer* leds_if = nullptr;
    IModel* model_if = nullptr;

    ParamSceneFixture() {
        platform = std::make_unique<NativePlatform>(BasicPentagonModel::LED_COUNT);
        BasicPentagonModel model_def_instance;
        auto concrete_model = std::make_unique<Model<BasicPentagonModel>>(
            model_def_instance, platform->getLEDs()
        );
        leds_wrapper = std::make_unique<LedBufferWrapper>(platform->getLEDs(), platform->getNumLEDs());
        model_wrapper = std::make_unique<ModelWrapper<BasicPentagonModel>>(std::move(concrete_model));
        leds_if = leds_wrapper.get();
        model_if = model_wrapper.get();
        test_scene.call_connect(*model_if, *leds_if, *platform);
        test_scene.setup(); 
    }
};


TEST_SUITE("Scene Parameters") {

    TEST_CASE_FIXTURE(ParamSceneFixture<ParamTestScene>, "Parameter Methods") {
        // Setup is called by fixture
        
        SUBCASE("Parameter Names") {
            auto names = test_scene.get_parameter_names();
            CHECK(names.size() == 5);
            CHECK(std::find(names.begin(), names.end(), "speed") != names.end());
            CHECK(std::find(names.begin(), names.end(), "count") != names.end());
            CHECK(std::find(names.begin(), names.end(), "enabled") != names.end());
            CHECK(std::find(names.begin(), names.end(), "brightness") != names.end());
            CHECK(std::find(names.begin(), names.end(), "color_hue") != names.end());
            auto proxy_names = test_scene.settings.names();
            CHECK(proxy_names.size() == 5);
            CHECK(std::find(proxy_names.begin(), proxy_names.end(), "speed") != proxy_names.end());
        }
        
        SUBCASE("Parameter Access") {
            CHECK(float(test_scene.settings["brightness"]) == doctest::Approx(0.8f));
            CHECK(float(test_scene.settings["speed"]) == doctest::Approx(1.0f));
            CHECK(static_cast<uint8_t>(test_scene.settings["color_hue"]) == 0);
            CHECK(bool(test_scene.settings["enabled"]) == true);
            CHECK(int(test_scene.settings["count"]) == 5);
            test_scene.settings["speed"] = 0.5f;
            test_scene.settings["count"] = 8;
            test_scene.settings["enabled"] = false;
            CHECK(float(test_scene.settings["speed"]) == doctest::Approx(0.5f));
            CHECK(int(test_scene.settings["count"]) == 8);
            CHECK(bool(test_scene.settings["enabled"]) == false);
        }
        
        SUBCASE("Parameter Metadata & Type") {
            const auto& speed_meta = test_scene.get_parameter_metadata("speed");
            CHECK(speed_meta.type == ParamType::range);
            CHECK(test_scene.get_parameter_type("speed") == ParamType::range);
            CHECK(test_scene.get_parameter_type("count") == ParamType::count);
            CHECK(test_scene.get_parameter_type("enabled") == ParamType::switch_type);
            CHECK(test_scene.get_parameter_type("brightness") == ParamType::ratio);
            CHECK(test_scene.get_parameter_type("color_hue") == ParamType::count);
        }
        
        SUBCASE("Parameter Reset") {
            float original_speed = test_scene.settings["speed"];
            int original_count = test_scene.settings["count"];
            test_scene.settings["speed"] = 0.1f;
            test_scene.settings["count"] = 1;
            CHECK(float(test_scene.settings["speed"]) == doctest::Approx(0.1f));
            CHECK(int(test_scene.settings["count"]) == 1);
            test_scene.reset(); 
            CHECK(float(test_scene.settings["speed"]) == doctest::Approx(original_speed));
            CHECK(int(test_scene.settings["count"]) == original_count);
        }
        
        SUBCASE("Parameter Existence") {
            CHECK(test_scene.has_parameter("speed"));
            CHECK(test_scene.has_parameter("count"));
            CHECK_FALSE(test_scene.has_parameter("non_existent"));
            CHECK(test_scene.settings.has_parameter("speed"));
            CHECK_FALSE(test_scene.settings.has_parameter("non_existent"));
        }
    }
} 