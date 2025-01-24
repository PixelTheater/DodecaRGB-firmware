#include <doctest.h>
#include "param.h"
#include "param_builder.h"
#include "param_collection.h"
#include "mock_palette.h"

TEST_CASE("Range validation works correctly") {
    Range r(0.0f, 1.0f);
    
    CHECK(r.contains(0.0f));
    CHECK(r.contains(0.5f));
    CHECK(r.contains(1.0f));
    CHECK_FALSE(r.contains(-0.1f));
    CHECK_FALSE(r.contains(1.1f));
    
    SUBCASE("Handles floating point precision") {
        Range pi_range(0.0f, M_PI);
        float almost_pi = M_PI - 1e-6f;
        CHECK(pi_range.contains(almost_pi));
    }
}

TEST_CASE("ParamBuilder creates float parameters correctly") {
    auto param = ParamBuilder("speed")
        .range(-1.0f, 1.0f)
        .set(0.0f)
        .build();
        
    CHECK_EQ(param.name, "speed");
    CHECK_EQ(param.type, ParamType::Float);
    CHECK_EQ(param.range.min, -1.0f);
    CHECK_EQ(param.range.max, 1.0f);
    CHECK_EQ(param.default_value, 0.0f);
}

TEST_CASE("Built-in ranges are defined correctly") {
    SUBCASE("Ratio range is 0 to 1") {
        auto param = ParamBuilder("intensity")
            .range(Ranges::Ratio)
            .set(0.5f)
            .build();
            
        CHECK_EQ(param.range.min, 0.0f);
        CHECK_EQ(param.range.max, 1.0f);
    }
    
    SUBCASE("SignedRatio range is -1 to 1") {
        auto param = ParamBuilder("speed")
            .range(Ranges::SignedRatio)
            .set(0.0f)
            .build();
            
        CHECK_EQ(param.range.min, -1.0f);
        CHECK_EQ(param.range.max, 1.0f);
    }
}

TEST_CASE("Parameter validation handles edge cases") {
    SUBCASE("Default values must be within range") {
        auto param = ParamBuilder("test")
            .range(0.0f, 1.0f)
            .set(0.5f)
            .build();
            
        CHECK(param.isValid(param.default_value));
    }
    
    SUBCASE("Integer parameters handle boundaries") {
        auto param = ParamBuilder("count")
            .range(1, 10)
            .set(5)
            .build();
            
        CHECK(param.isValid(1.0f));
        CHECK(param.isValid(10.0f));
        CHECK_FALSE(param.isValid(0.9f));
        CHECK_FALSE(param.isValid(10.1f));
    }

    SUBCASE("Default value must be within range") {
        CHECK_THROWS_AS(
            ParamBuilder("test")
                .range(0.0f, 1.0f)
                .set(2.0f)     // Outside range
                .build(),
            std::invalid_argument
        );

        CHECK_THROWS_AS(
            ParamBuilder("test")
                .range(Ranges::Ratio)  // 0..1
                .set(-0.5f)       // Outside range
                .build(),
            std::invalid_argument
        );
    }

    SUBCASE("Integer default value must be within range") {
        CHECK_THROWS_AS(
            ParamBuilder("count")
                .range(1, 10)
                .set(11)      // Outside range
                .build(),
            std::invalid_argument
        );
    }
}

TEST_CASE("Boolean parameters work correctly") {
    SUBCASE("Boolean defaults to false") {
        auto param = ParamBuilder("enabled")
            .boolean()
            .build();
            
        CHECK_EQ(param.type, ParamType::Bool);
        CHECK_EQ(param.default_value, 0.0f);
    }
    
    SUBCASE("Boolean can be set true") {
        auto param = ParamBuilder("enabled")
            .boolean()
            .set(1.0f)
            .build();
            
        CHECK_EQ(param.default_value, 1.0f);
    }
}

TEST_CASE("Instance parameters are configured correctly") {
    static CRGBPalette16 defaultPalette;
    auto param = ParamBuilder("colors")
        .as<CRGBPalette16>()
        .set(defaultPalette)
        .build();
        
    CHECK_EQ(param.type, ParamType::Instance);
    CHECK(param.isInstanceOf<CRGBPalette16>());
    CHECK_EQ(param.getInstance<CRGBPalette16>(), &defaultPalette);
}

TEST_CASE("All built-in ranges are defined correctly") {
    SUBCASE("Percent range is 0 to 100") {
        auto param = ParamBuilder("opacity")
            .range(Ranges::Percent)
            .set(50.0f)
            .build();
            
        CHECK_EQ(param.range.min, 0.0f);
        CHECK_EQ(param.range.max, 100.0f);
    }
    
    SUBCASE("Angle range is 0 to TWO_PI") {
        auto param = ParamBuilder("rotation")
            .range(Ranges::Angle)
            .set(0.0f)
            .build();
            
        CHECK_EQ(param.range.min, 0.0f);
        CHECK(param.range.max == doctest::Approx(M_TWO_PI).epsilon(0.00001));
    }
    
    SUBCASE("SignedAngle range is -PI to PI") {
        auto param = ParamBuilder("phase")
            .range(Ranges::SignedAngle)
            .set(0.0f)
            .build();
            
        CHECK(param.range.min == doctest::Approx(-M_PI).epsilon(0.00001));
        CHECK(param.range.max == doctest::Approx(M_PI).epsilon(0.00001));
    }
}

TEST_CASE("Parameter builder supports chaining") {
    ParameterCollection params;
    
    SUBCASE("Can chain multiple valid operations") {
        auto param = ParamBuilder("speed")
            .range(Ranges::SignedRatio)  // Set range first
            .set(0.0f)                   // Then default value
            .build();                    // Finally build
            
        CHECK_EQ(param.name, "speed");
        CHECK_EQ(param.range.min, -1.0f);
        CHECK_EQ(param.range.max, 1.0f);
        CHECK_EQ(param.default_value, 0.0f);
    }
    
    SUBCASE("Can chain custom range with value") {
        auto param = ParamBuilder("custom")
            .range(-5.0f, 5.0f)  // Custom range
            .set(0.0f)           // Within range
            .build();
            
        CHECK_EQ(param.range.min, -5.0f);
        CHECK_EQ(param.range.max, 5.0f);
        CHECK_EQ(param.default_value, 0.0f);
    }
    
    SUBCASE("Invalid chains throw appropriate errors") {
        // Setting value before range is ok
        CHECK_NOTHROW(
            ParamBuilder("test")
                .set(0.5f)
                .range(Ranges::Ratio)
                .build()
        );
        
        // Setting boolean then range should throw
        CHECK_THROWS_AS(
            ParamBuilder("test")
                .boolean()
                .range(Ranges::Ratio)  // Can't set range on boolean
                .build(),
            std::invalid_argument
        );
        
        // Setting instance then range should throw
        CHECK_THROWS_AS(
            ParamBuilder("test")
                .as<CRGBPalette16>()
                .range(Ranges::Ratio)  // Can't set range on instance
                .build(),
            std::invalid_argument
        );
    }
}

TEST_CASE("Parameter randomization works correctly") {
    SUBCASE("Randomized values stay within range") {
        // Test multiple times to ensure consistent behavior
        for (int i = 0; i < 100; i++) {
            auto param = ParamBuilder("test")
                .range(0.0f, 1.0f)
                .randomize()
                .build();
                
            CHECK_GE(param.default_value, 0.0f);
            CHECK_LE(param.default_value, 1.0f);
        }
    }
    
    SUBCASE("Works with custom ranges") {
        auto param = ParamBuilder("custom")
            .range(-5.0f, 5.0f)
            .randomize()
            .build();
            
        CHECK_GE(param.default_value, -5.0f);
        CHECK_LE(param.default_value, 5.0f);
    }
    
    SUBCASE("Works with built-in ranges") {
        auto param = ParamBuilder("angle")
            .range(Ranges::Angle)  // 0 to TWO_PI
            .randomize()
            .build();
            
        CHECK_GE(param.default_value, 0.0f);
        CHECK_LE(param.default_value, M_TWO_PI);
    }
    
    SUBCASE("Cannot randomize boolean parameters") {
        CHECK_THROWS_AS(
            ParamBuilder("test")
                .boolean()
                .randomize()
                .build(),
            std::invalid_argument
        );
    }
    
    SUBCASE("Cannot randomize instance parameters") {
        CHECK_THROWS_AS(
            ParamBuilder("test")
                .as<CRGBPalette16>()
                .randomize()
                .build(),
            std::invalid_argument
        );
    }
} 