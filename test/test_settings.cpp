#include <doctest/doctest.h>
#include "PixelTheater/settings.h"
#include "PixelTheater/params/param_def.h"
#include "fixtures/fireworks_params.h"
#include <iostream>

using namespace PixelTheater;

TEST_SUITE("Settings") {
    TEST_CASE("Settings can be initialized from ParamDef array") {
        Settings settings(FIREWORKS_PARAMS, sizeof(FIREWORKS_PARAMS)/sizeof(ParamDef));

        SUBCASE("Parameters have correct default values") {
            CHECK(bool(settings["sparkle"]) == true);
            CHECK(int(settings["num_particles"]) == 100);
            CHECK(float(settings["gravity"]) == doctest::Approx(-0.8f));
        }

        SUBCASE("Parameters respect their ranges") {
            CHECK_THROWS_AS(settings["num_particles"] = 2000, std::out_of_range);  // Above max
            CHECK_THROWS_AS(settings["num_particles"] = 5, std::out_of_range);     // Below min
            
            settings["num_particles"] = 500;  // Valid
            CHECK(int(settings["num_particles"]) == 500);
        }

        SUBCASE("Parameters can be reset") {
            settings["speed"] = 0.75f;
            settings.reset_all();
            CHECK(float(settings["speed"]) == doctest::Approx(0.5f));
        }

        SUBCASE("Parameter metadata is accessible") {
            auto speed = settings["speed"];
            CHECK(speed.min() == 0.0f);
            CHECK(speed.max() == 1.0f);
            CHECK(speed.default_value() == doctest::Approx(0.5f));
            CHECK(std::string(speed.description()) == "Animation speed multiplier");
        }
    }

    TEST_CASE("Settings can be accessed via operator[]") {
        Settings settings(FIREWORKS_PARAMS, sizeof(FIREWORKS_PARAMS)/sizeof(ParamDef));

        SUBCASE("Basic value access") {
            // Direct type conversion
            bool sparkle = settings["sparkle"];
            float speed = settings["speed"];
            int particles = settings["num_particles"];
            
            CHECK(sparkle == true);
            CHECK(speed == doctest::Approx(0.5f));
            CHECK(particles == 100);
        }

        SUBCASE("Assignment") {
            settings["speed"] = 0.8f;
            settings["num_particles"] = 500;
            settings["sparkle"] = false;
            
            CHECK(float(settings["speed"]) == doctest::Approx(0.8f));
            CHECK(int(settings["num_particles"]) == 500);
            CHECK(bool(settings["sparkle"]) == false);
        }

        SUBCASE("Range validation") {
            // Out of range values should throw
            CHECK_THROWS_AS(settings["speed"] = 2.0f, std::out_of_range);
            CHECK_THROWS_AS(settings["num_particles"] = 2000, std::out_of_range);
            
            // In range values should work
            settings["speed"] = 0.75f;
            CHECK(float(settings["speed"]) == doctest::Approx(0.75f));
        }

        SUBCASE("Parameter metadata") {
            auto speed = settings["speed"];
            CHECK(speed.min() == 0.0f);
            CHECK(speed.max() == 1.0f);
            CHECK(speed.default_value() == doctest::Approx(0.5f));
            CHECK(std::string(speed.description()) == "Animation speed multiplier");
        }

        SUBCASE("Invalid parameters") {
            CHECK_THROWS_AS(settings["invalid"], std::out_of_range);
        }

        SUBCASE("Reset to defaults") {
            settings["speed"] = 0.8f;
            settings.reset_all();
            CHECK(float(settings["speed"]) == doctest::Approx(0.5f));
        }
    }
} 