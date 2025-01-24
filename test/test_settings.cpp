#include <doctest.h>
#include "settings.h"
#include "param_collection.h"
#include "mock_palette.h"

TEST_CASE("Settings basic functionality") {
    ParameterCollection params;
    params.param("speed").range(Ranges::SignedRatio).set(0.0f);
    params.build();
    
    Settings settings(params);
    
    SUBCASE("Can get default values") {
        CHECK_EQ(settings["speed"], 0.0f);
    }
}

TEST_CASE("Settings value management") {
    ParameterCollection params;
    params.param("speed").range(Ranges::SignedRatio).set(0.0f);
    params.param("size").range(Ranges::Ratio).set(0.5f);
    params.build();
    
    Settings settings(params);
    
    SUBCASE("Can set and get values") {
        settings["speed"] = 0.5f;
        CHECK_EQ(settings["speed"], 0.5f);
    }
    
    SUBCASE("Invalid values are rejected") {
        CHECK_THROWS_AS(
            settings["speed"] = 2.0f,  // Outside -1..1 range
            std::invalid_argument
        );
    }
}

TEST_CASE("Settings custom type support") {
    ParameterCollection params;
    // Create palettes with different states
    CRGBPalette16 defaultPalette;
    CRGBPalette16 newPalette;
    
    // Add a way to track state
    defaultPalette.state = 1;
    newPalette.state = 2;
    
    params.param("colors")
        .as<CRGBPalette16>()
        .set(defaultPalette);
    params.build();
    
    Settings settings(params);
    
    SUBCASE("Can get default instance") {
        const auto& palette = settings["colors"].as<CRGBPalette16>();
        CHECK_EQ(palette.state, defaultPalette.state);
    }
    
    SUBCASE("Can set instance") {
        settings["colors"] = newPalette;
        const auto& palette = settings["colors"].as<CRGBPalette16>();
        // Verify we got the new state
        CHECK_EQ(palette.state, newPalette.state);
    }
}

TEST_CASE("Settings supports chaining") {
    ParameterCollection params;
    params.param("speed").range(Ranges::SignedRatio).set(0.0f);
    params.param("size").range(Ranges::Ratio).set(0.5f);
    params.build();
    
    Settings settings(params);
    
    SUBCASE("Can chain multiple settings") {
        settings["speed"] = 0.5f;
        settings["size"] = 0.8f;
        
        CHECK_EQ(settings["speed"], 0.5f);
        CHECK_EQ(settings["size"], 0.8f);
    }
    
    SUBCASE("Invalid chains throw appropriate errors") {
        CHECK_THROWS_AS(
            settings["speed"] = 2.0f,  // Invalid value
            std::invalid_argument
        );
        
        CHECK_THROWS_AS(
            settings["unknown"] = 1.0f,  // Unknown parameter
            std::invalid_argument
        );
    }
} 