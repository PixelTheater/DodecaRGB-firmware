#include <doctest/doctest.h>
#include "param.h"
#include "param_builder.h"
#include "settings.h"
#include "mock_fastled.h"

using namespace Animation;

TEST_CASE("Parameter Definitions") {
    SUBCASE("Float parameters") {
        auto param = ParamDefinition::createFloat("speed", Range(0.0f, 1.0f), 0.5f);
        CHECK(param.name == "speed");
        CHECK(param.type == ParamType::Float);
        CHECK(param.initial_value == 0.5f);
        CHECK(param.range == Range(0.0f, 1.0f));
        CHECK(param.instance_type == nullptr);
        CHECK(param.initial_instance == nullptr);
    }

    SUBCASE("Int parameters") {
        auto param = ParamDefinition::createInt("count", 0, 10, 5);
        CHECK(param.name == "count");
        CHECK(param.type == ParamType::Int);
        CHECK(param.initial_value == 5.0f);
        CHECK(param.range == Range(0.0f, 10.0f));
        CHECK(param.instance_type == nullptr);
        CHECK(param.initial_instance == nullptr);
    }

    SUBCASE("Bool parameters") {
        auto param = ParamDefinition::createBool("enabled", true);
        CHECK(param.name == "enabled");
        CHECK(param.type == ParamType::Bool);
        CHECK(param.initial_value == 1.0f);
        CHECK(param.range == Range(0.0f, 1.0f));
        CHECK(param.instance_type == nullptr);
        CHECK(param.initial_instance == nullptr);
    }

    SUBCASE("Instance parameters") {
        CRGBPalette16 palette;
        auto param = ParamDefinition::createInstance("colors", &typeid(CRGBPalette16), &palette);
        CHECK(param.name == "colors");
        CHECK(param.type == ParamType::Instance);
        CHECK(param.initial_value == 0.0f);
        CHECK(param.range == Range(0.0f, 0.0f));
        CHECK(param.instance_type == &typeid(CRGBPalette16));
        CHECK(param.initial_instance == &palette);
        CHECK(param.isInstanceOf<CRGBPalette16>());
    }
}

TEST_CASE("Parameter Type Safety") {
    SUBCASE("Instance type checking") {
        CRGBPalette16 palette;
        auto param = ParamDefinition::createInstance("colors", &typeid(CRGBPalette16), &palette);
        CHECK(param.isInstanceOf<CRGBPalette16>());
        CHECK_FALSE(param.isInstanceOf<int>());
        CHECK_THROWS_AS(param.getInstance<int>(), std::bad_cast);
        CHECK_NOTHROW(param.getInstance<CRGBPalette16>());
    }
    
    SUBCASE("Value validation") {
        auto param = ParamDefinition::createInt("count", 0, 10, 5);
        CHECK(param.isValid(5.0f));  // Integer
        CHECK(param.isValid(5.5f));  // Rounded float
        CHECK_FALSE(param.isValid(-1.0f));  // Out of range
    }
}

TEST_CASE("ParamDefinition Creation") {
    SUBCASE("Float parameters") {
        auto param = ParamDefinition::createFloat("speed", Ranges::SignedRatio, 0.5f);
        CHECK(param.name == "speed");
        CHECK(param.type == ParamType::Float);
        CHECK(param.range == Ranges::SignedRatio);
        CHECK(param.initial_value == doctest::Approx(0.5f));
    }

    SUBCASE("Integer parameters") {  // New test
        auto param = ParamDefinition::createInt("count", 0, 10, 5);
        CHECK(param.name == "count");
        CHECK(param.type == ParamType::Int);
        CHECK(param.range.min == 0.0f);
        CHECK(param.range.max == 10.0f);
        CHECK(param.initial_value == 5.0f);
    }

    SUBCASE("Boolean parameters") {
        auto param = ParamDefinition::createBool("active", true);
        CHECK(param.name == "active");
        CHECK(param.type == ParamType::Bool);
        CHECK(param.initial_value == 1.0f);
    }
}

TEST_CASE("Built-in Ranges") {
    SUBCASE("Ratio range") {
        CHECK(Ranges::Ratio.min == 0.0f);
        CHECK(Ranges::Ratio.max == 1.0f);
        CHECK(Ranges::Ratio.contains(0.5f));
        CHECK_FALSE(Ranges::Ratio.contains(-0.1f));
        CHECK_FALSE(Ranges::Ratio.contains(1.1f));
    }
    
    SUBCASE("SignedRatio range") {
        CHECK(Ranges::SignedRatio.min == -1.0f);
        CHECK(Ranges::SignedRatio.max == 1.0f);
        CHECK(Ranges::SignedRatio.contains(0.0f));
        CHECK(Ranges::SignedRatio.contains(-0.5f));
        CHECK_FALSE(Ranges::SignedRatio.contains(-1.1f));
    }
    
    SUBCASE("Percent range") {
        CHECK(Ranges::Percent.min == 0.0f);
        CHECK(Ranges::Percent.max == 100.0f);
        CHECK(Ranges::Percent.contains(50.0f));
        CHECK_FALSE(Ranges::Percent.contains(-1.0f));
        CHECK_FALSE(Ranges::Percent.contains(101.0f));
    }
    
    SUBCASE("Angle range") {
        CHECK(Ranges::Angle.min == 0.0f);
        CHECK(Ranges::Angle.max == doctest::Approx(M_TWO_PI));
        CHECK(Ranges::Angle.contains(M_PI));
        CHECK_FALSE(Ranges::Angle.contains(-0.1f));
        CHECK_FALSE(Ranges::Angle.contains(M_TWO_PI + 0.1f));
    }
    
    SUBCASE("SignedAngle range") {
        CHECK(Ranges::SignedAngle.min == doctest::Approx(-M_PI));
        CHECK(Ranges::SignedAngle.max == doctest::Approx(M_PI));
        CHECK(Ranges::SignedAngle.contains(0.0f));
        CHECK(Ranges::SignedAngle.contains(-M_PI/2));
        CHECK_FALSE(Ranges::SignedAngle.contains(-M_PI - 0.1f));
    }
    
    SUBCASE("Range clamping") {
        CHECK(Ranges::Percent.clamp(-10.0f) == 0.0f);
        CHECK(Ranges::Percent.clamp(110.0f) == 100.0f);
        CHECK(Ranges::Percent.clamp(50.0f) == 50.0f);
        
        CHECK(Ranges::SignedAngle.clamp(-M_TWO_PI) == doctest::Approx(-M_PI));
        CHECK(Ranges::SignedAngle.clamp(M_TWO_PI) == doctest::Approx(M_PI));
        CHECK(Ranges::SignedAngle.clamp(0.0f) == 0.0f);
    }
} 