#include <doctest/doctest.h>
#include "PixelTheater/settings.h"
#include "PixelTheater/settings_proxy.h"
#include "PixelTheater/params/param_def.h"
#include "fixtures/parameter_test_params.h"
#include "fixtures/fireworks_params.h"
#include <iostream>

using namespace PixelTheater;

TEST_SUITE("Settings") {
    TEST_CASE("Core Settings functionality") {
        Settings settings;

        SUBCASE("Parameter definition") {
            ParamDef def = PARAM_RATIO("test_ratio", 0.5f, Flags::NONE, "Test ratio");
            settings.add_parameter(def);
            
            // Test metadata storage
            const ParamDef& stored = settings.get_metadata("test_ratio");
            CHECK(stored.type == ParamType::ratio);
            CHECK(std::string(stored.description) == "Test ratio");
        }

        SUBCASE("Value storage and retrieval") {
            settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::NONE, ""));
            
            // Direct value access
            settings.set_value("speed", ParamValue(0.75f));
            ParamValue val = settings.get_value("speed");
            CHECK(val.as_float() == doctest::Approx(0.75f));
        }

        SUBCASE("Parameter validation") {
            settings.add_parameter(PARAM_RANGE("test", -1.0f, 1.0f, 0.0f, Flags::NONE, ""));
            
            // Valid value
            CHECK_NOTHROW(settings.set_value("test", ParamValue(0.5f)));
            
            // Invalid value
            CHECK_THROWS_AS(settings.set_value("test", ParamValue(1.5f)), std::out_of_range);
        }

        SUBCASE("Parameter proxy access") {
            settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::NONE, ""));
            SettingsProxy proxy(settings);

            SUBCASE("Value access") {
                float speed = proxy["speed"];
                CHECK(speed == doctest::Approx(0.5f));
            }
        }

        SUBCASE("Invalid parameter definitions") {
            Settings settings;
            
            // Test invalid default value
            ParamDef invalid_def = PARAM_RATIO("test", 1.5f, Flags::CLAMP, "");
            CHECK_THROWS_AS(settings.add_parameter(invalid_def), std::out_of_range);
        }
    }
}

TEST_SUITE("SettingsProxy") {
    TEST_CASE("Proxy interface") {
        Settings settings;
        SettingsProxy proxy(settings);
        
        SUBCASE("Type-safe access") {
            settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::NONE, ""));
            
            // Assignment
            proxy["speed"] = 0.75f;
            
            // Access
            float speed = proxy["speed"];
            CHECK(speed == doctest::Approx(0.75f));
        }

        SUBCASE("Parameter metadata access") {
            settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::CLAMP, "Speed control"));
            
            auto param = proxy["speed"];
            CHECK(param.min() == Constants::RATIO_MIN);
            CHECK(param.max() == Constants::RATIO_MAX);
            CHECK(std::string(param.description()) == "Speed control");
        }
    }
} 