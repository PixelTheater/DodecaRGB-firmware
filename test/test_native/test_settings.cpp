#include <doctest/doctest.h>
#include "PixelTheater/settings.h"
#include "PixelTheater/settings_proxy.h"
#include "PixelTheater/params/param_def.h"
#include "fixtures/test_scene_params.h"
#include "helpers/log_capture.h"
#include <iostream>

using namespace PixelTheater;

TEST_SUITE("Settings") {
    TEST_CASE("Parameter storage") {
        Settings settings;
        
        SUBCASE("Basic storage") {
            settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::NONE, ""));
            CHECK(settings.has_parameter("speed"));
            CHECK(settings.get_value("speed").as_float() == 0.5f);
        }

        SUBCASE("Metadata storage") {
            settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::CLAMP, "Speed control"));
            const auto& def = settings.get_metadata("speed");
            CHECK(def.type == ParamType::ratio);
            CHECK(def.has_flag(Flags::CLAMP));
            CHECK(std::string(def.description) == "Speed control");
        }
    }

    TEST_CASE("Parameter loading") {
        SUBCASE("YAML loading") {
            Settings settings(TEST_SCENE_PARAMS, sizeof(TEST_SCENE_PARAMS)/sizeof(ParamDef));
            CHECK(settings.get_type("speed") == ParamType::ratio);
            CHECK(settings.get_type("count") == ParamType::count);
            CHECK(settings.get_value("speed").as_float() == doctest::Approx(0.5f));
        }

        SUBCASE("String-based loading") {
            Settings settings;
            settings.add_parameter_from_strings("speed", "ratio", ParamValue(0.5f), "clamp");
            CHECK(settings.get_type("speed") == ParamType::ratio);
            CHECK(settings.get_metadata("speed").has_flag(Flags::CLAMP));
        }
    }

    TEST_CASE("Parameter inheritance") {
        Settings base;
        Settings derived;
        
        SUBCASE("Basic inheritance") {
            base.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::CLAMP, ""));
            derived.inherit_from(base);
            CHECK(derived.has_parameter("speed"));
            CHECK(derived.get_metadata("speed").has_flag(Flags::CLAMP));
        }

        SUBCASE("Override behavior") {
            base.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::CLAMP, ""));
            derived.inherit_from(base);
            derived.add_parameter(PARAM_RATIO("speed", 0.8f, Flags::WRAP, ""));
            CHECK(derived.get_value("speed").as_float() == 0.8f);
            CHECK(derived.get_metadata("speed").has_flag(Flags::WRAP));
        }
    }

    TEST_CASE("Settings proxy") {
        Settings settings;
        SettingsProxy proxy(settings);
        
        SUBCASE("Type-safe access") {
            settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::NONE, ""));
            proxy["speed"] = 0.75f;
            float speed = proxy["speed"];
            CHECK(speed == doctest::Approx(0.75f));
        }

        SUBCASE("Validation") {
            settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::CLAMP, ""));
            proxy["speed"] = 1.5f;  // Should clamp
            CHECK(float(proxy["speed"]) == 1.0f);
        }
    }
} 