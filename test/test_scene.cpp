#include <doctest/doctest.h>
#include "PixelTheater/scene.h"
#include "PixelTheater/params/param_def.h"
#include "helpers/fastled_test_helper.h"
#include <iostream>

using namespace PixelTheater;

// Scene metadata
static constexpr ParamDef::Metadata TEST_SCENE_INFO = {
    "test_scene",
    "Test scene for parameter configuration"
};

// Example scene implementation like a user would create
class ColorScene : public Scene {
public:
    ColorScene() : Scene() {}

    void setup() override {
        // Define parameters as a user would
        param("hue", "angle", 0.0f, "wrap");
        param("saturation", "ratio", 1.0f, "clamp");
        param("brightness", "ratio", 0.8f, "clamp");
        
        _is_setup = true;
    }

    void tick() override {
        Scene::tick();
        // Use parameters as a user would
        _current_hue = settings["hue"];
        _current_brightness = settings["brightness"];
    }

    // Public methods for testing state
    float get_hue() const { return _current_hue; }
    float get_brightness() const { return _current_brightness; }
    bool is_setup() const { return _is_setup; }

private:
    float _current_hue{0.0f};
    float _current_brightness{0.8f};
    bool _is_setup{false};
};

TEST_SUITE("Scene") {
    TEST_CASE("Scene lifecycle") {
        ColorScene scene;

        SUBCASE("Setup initializes parameters") {
            scene.setup();
            CHECK(scene.is_setup());
            CHECK(float(scene.settings["brightness"]) == doctest::Approx(0.8f));
        }

        SUBCASE("Tick updates scene state") {
            scene.setup();
            scene.settings["hue"] = Constants::HALF_PI;
            scene.tick();
            CHECK(scene.get_hue() == doctest::Approx(Constants::HALF_PI));
        }
    }

    TEST_CASE("Parameter validation") {
        ColorScene scene;
        scene.setup();

        SUBCASE("Valid parameter changes") {
            CHECK_NOTHROW(scene.settings["brightness"] = 0.5f);
            CHECK(float(scene.settings["brightness"]) == doctest::Approx(0.5f));
        }

        SUBCASE("Values are clamped") {
            scene.settings["brightness"] = 1.5f;
            CHECK(float(scene.settings["brightness"]) == doctest::Approx(1.0f));
        }
    }
} 