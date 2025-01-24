#include <doctest.h>
#include "param_collection.h"

TEST_CASE("ParameterCollection basic functionality") {
    ParameterCollection params;
    
    SUBCASE("Can define parameters") {
        params.param("speed")
            .range(Ranges::SignedRatio)
            .set(0.0f);
            
        params.param("size")
            .range(Ranges::Ratio)
            .set(0.5f);
            
        params.build();
        
        CHECK(params.get("speed") != nullptr);
        CHECK(params.get("size") != nullptr);
        CHECK(params.get("nonexistent") == nullptr);
    }
    
    SUBCASE("Parameters have correct values") {
        params.param("speed")
            .range(Ranges::SignedRatio)
            .set(0.0f);
        params.build();
        
        auto speed = params.get("speed");
        REQUIRE(speed != nullptr);
        CHECK_EQ(speed->name, "speed");
        CHECK_EQ(speed->type, ParamType::Float);
        CHECK_EQ(speed->default_value, 0.0f);
        CHECK_EQ(speed->range.min, -1.0f);
        CHECK_EQ(speed->range.max, 1.0f);
    }
}

TEST_CASE("ParameterCollection advanced functionality") {
    ParameterCollection params;
    
    SUBCASE("Multiple parameters work together") {
        // Define all parameters from README example
        params.param("speed").range(Ranges::SignedRatio).set(0.0f);
        params.param("size").range(Ranges::Ratio).set(0.5f);
        params.param("angle").range(Ranges::Angle).set(0.0f);
        params.param("brightness").range(Ranges::Percent).set(50);
        params.param("custom_speed").range(-5.0f, 5.0f).set(0.0f);
        params.param("num_points").range(1, 100).set(50);
        
        params.build();
        
        // Check they're all there with correct values
        auto speed = params.get("speed");
        CHECK_EQ(speed->default_value, 0.0f);
        
        auto size = params.get("size");
        CHECK_EQ(size->default_value, 0.5f);
        
        auto brightness = params.get("brightness");
        CHECK_EQ(brightness->default_value, 50.0f);
    }
    
    SUBCASE("Duplicate parameters are not allowed") {
        params.param("test").range(Ranges::SignedRatio).set(0.5f);
        CHECK_THROWS_AS(
            params.param("test").range(Ranges::SignedRatio),
            std::invalid_argument
        );
    }
    
    SUBCASE("Cannot modify after build") {
        params.param("test").range(Ranges::SignedRatio).set(0.5f);
        params.build();
        
        CHECK_THROWS_AS(
            params.param("another"),
            std::runtime_error
        );
    }
}

TEST_CASE("Parameter name validation") {
    ParameterCollection params;
    
    SUBCASE("Empty names are not allowed") {
        CHECK_THROWS_AS(
            params.param(""),
            std::invalid_argument
        );
    }
    
    SUBCASE("Names with spaces are not allowed") {
        CHECK_THROWS_AS(
            params.param("my param"),
            std::invalid_argument
        );
    }
    
    SUBCASE("Names with special characters are not allowed") {
        CHECK_THROWS_AS(
            params.param("speed!"),
            std::invalid_argument
        );
        CHECK_THROWS_AS(
            params.param("color@2"),
            std::invalid_argument
        );
    }
    
    SUBCASE("Valid names are accepted") {
        CHECK_NOTHROW(params.param("speed"));
        CHECK_NOTHROW(params.param("color2"));
        CHECK_NOTHROW(params.param("my_param"));
        CHECK_NOTHROW(params.param("_private"));
    }
} 