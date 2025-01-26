#include <doctest/doctest.h>
#include "settings.h"
#include "mock_fastled.h"


using namespace Animation;

TEST_CASE("Parameter Definition") {
    Settings settings;
    
    SUBCASE("Float parameters with range") {
        settings.param("speed")
            .range(0.0f, 1.0f)
            .set(0.5f)
            .build();
            
        CHECK(settings("speed") == 0.5f);
        
        settings.set("speed", -0.1f);
        CHECK(settings("speed") == 0.0f);
        
        settings.set("speed", 1.1f);
        CHECK(settings("speed") == 1.0f);
    }

    SUBCASE("Boolean parameters") {
        settings.param("enabled")
            .boolean()
            .set(true)
            .build();
            
        CHECK(settings.operator()<bool>("enabled") == true);
        settings.set("enabled", false);
        CHECK(settings.operator()<bool>("enabled") == false);
    }

    SUBCASE("Integer parameters") {
        settings.param("count")
            .range(0, 10)
            .set(5)
            .build();
            
        CHECK(settings.operator()<int>("count") == 5);
    }

    SUBCASE("Instance parameters") {
        CRGBPalette16 palette;
        settings.param("colors")
            .as<CRGBPalette16>()
            .set(palette)
            .build();
            
        CHECK_NOTHROW(settings.operator()<CRGBPalette16>("colors"));
    }

    SUBCASE("Invalid definitions") {
        CHECK_THROWS_AS(settings.param("123invalid"), std::invalid_argument);
        
        // Test duplicate prevention
        settings.param("speed")
            .range(0.0f, 1.0f)
            .set(0.5f)
            .build();
            
        CHECK_THROWS_AS(settings.param("speed"), std::invalid_argument);
    }
}

TEST_CASE("Settings Return Types") {
    Settings settings;

    SUBCASE("Default operator() returns float") {
        settings.param("speed")
            .range(0.0f, 1.0f)
            .set(0.5f)
            .build();
            
        using ReturnType = decltype(settings("speed"));
        CHECK(std::is_same_v<ReturnType, float>);
    }

    SUBCASE("Templated operator() returns requested type") {
        settings.param("count")
            .range(0, 10)
            .set(5)
            .build();
            
        settings.param("enabled")
            .boolean()
            .set(true)
            .build();
            
        using IntReturn = decltype(settings.operator()<int>("count"));
        CHECK(std::is_same_v<IntReturn, int>);
        
        using BoolReturn = decltype(settings.operator()<bool>("enabled"));
        CHECK(std::is_same_v<BoolReturn, bool>);
    }
}

TEST_CASE("Settings Value Access") {
    Settings settings;
    
    SUBCASE("Float value access and updates") {
        settings.param("speed")
            .range(0.0f, 1.0f)
            .set(0.5f)
            .build();
            
        settings.set("speed", 0.8f);
        CHECK(settings("speed") == 0.8f);
    }

    SUBCASE("Instance value access and updates") {
        CRGBPalette16 palette1, palette2;
        settings.param("colors")
            .as<CRGBPalette16>()
            .set(palette1)
            .build();
            
        const auto& stored = settings.getInstance<CRGBPalette16>("colors");
        CHECK(&stored == &palette1);
        
        settings.set("colors", palette2);
        const auto& updated = settings.getInstance<CRGBPalette16>("colors");
        CHECK(&updated == &palette2);
    }
}

TEST_CASE("Preset Management") {
    Settings settings;
    
    SUBCASE("Applying presets") {
        // Setup parameters first
        settings.param("speed").range(0.0f, 1.0f).set(0.5f).build();
        settings.param("enabled").boolean().set(true).build();
        
        auto preset = settings.createPreset("fast")
            .set("speed", 0.8f)
            .set("enabled", false)
            .build();
        
        settings.applyPreset(preset);
        CHECK(settings("speed") == 0.8f);
        CHECK(settings.operator()<bool>("enabled") == false);
    }
}

TEST_CASE("Preset Builder") {
    Settings settings;
    
    SUBCASE("Preset is saved on builder destruction") {
        settings.param("speed").range(0.0f, 1.0f).set(0.5f).build();
        
        auto preset = settings.createPreset("test")
            .set("speed", 0.8f)
            .build();
        settings.applyPreset(preset);
        
        CHECK(settings("speed") == 0.8f);
    }

    SUBCASE("Multiple values in preset") {
        settings.param("speed").range(0.0f, 1.0f).set(0.5f).build();
        settings.param("size").range(0, 10).set(5).build();
        
        auto preset = settings.createPreset("multi")
            .set("speed", 0.8f)
            .set("size", 7)
            .build();
        
        settings.applyPreset(preset);
        CHECK(settings("speed") == 0.8f);
        CHECK(settings.operator()<int>("size") == 7);
    }

    SUBCASE("Instance values in preset") {
        CRGBPalette16 palette1, palette2;
        
        settings.param("colors").as<CRGBPalette16>().set(palette1).build();
        
        // Verify initial state
        CHECK(typeid(settings.getInstance<CRGBPalette16>("colors")) == typeid(palette1));
        
        auto preset = settings.createPreset("with_instance")
            .set("colors", palette2)
            .build();
        
        settings.applyPreset(preset);
        const auto& stored = settings.getInstance<CRGBPalette16>("colors");
        
        // Verify final state
        CHECK(typeid(stored) == typeid(palette2));  // Same type
        CHECK(sizeof(stored) == sizeof(palette2));   // Same size
        CHECK_NE(&stored, &palette2);               // Different object (copy)
    }
}

TEST_CASE("Settings Access") {
    Settings settings;
    // Set up parameters directly
    settings.param("speed").range(0.0f, 1.0f).set(0.5f).build();
    settings.param("enabled").boolean().set(true).build();
    settings.param("count").range(0, 10).set(5).build();
    
    SUBCASE("Basic float access") {
        CHECK(settings("speed") == 0.5f);
    }
    
    SUBCASE("Type-safe access") {
        CHECK(settings.operator()<int>("count") == 5);
        CHECK(settings.operator()<bool>("enabled") == true);
    }
    
    SUBCASE("Instance access") {
        // Create palette instance for test
        CRGBPalette16 test_palette;
        
        // Set up parameter with instance
        settings.param("colors")
            .as<CRGBPalette16>()
            .set(test_palette)
            .build();
        
        const auto& stored = settings.getInstance<CRGBPalette16>("colors");
        CHECK(&stored == &test_palette);
    }
}