#include <doctest/doctest.h>
#include "PixelTheater/settings.h"
#include "PixelTheater/settings_proxy.h"
#include "PixelTheater/params/param_def.h"
#include "../../helpers/log_capture.h"
#include <iostream>

using namespace PixelTheater;

// Add a simple test scene class to test the param method
class TestScene {
public:
    TestScene() : settings(), proxy(settings) {}
    
    void param(const std::string& name, const std::string& type,
              int min, int max, int default_val, const std::string& flags = "",
              const std::string& description = "") {
        // Use the direct method for count parameters
        if (type == "count") {
            settings.add_count_parameter(name, min, max, default_val, flags, description);
        } else {
            // For other types, use the string-based method
            settings.add_parameter_from_strings(name, type, ParamValue(default_val), flags, description);
        }
    }
    
    void param(const std::string& name, const std::string& type,
              float min, float max, float default_val, const std::string& flags = "",
              const std::string& description = "") {
        // Use the direct method for range parameters
        if (type == "range") {
            settings.add_range_parameter(name, min, max, default_val, flags, description);
        } else {
            // For other types, use the string-based method
            settings.add_parameter_from_strings(name, type, ParamValue(default_val), flags, description);
        }
    }
    
    // For simple parameters without ranges
    void param(const std::string& name, const std::string& type,
              float default_val, const std::string& flags = "",
              const std::string& description = "") {
        settings.add_parameter_from_strings(name, type, ParamValue(default_val), flags, description);
    }
    
    void param(const std::string& name, const std::string& type,
              int default_val, const std::string& flags = "",
              const std::string& description = "") {
        settings.add_parameter_from_strings(name, type, ParamValue(default_val), flags, description);
    }
    
    void param(const std::string& name, const std::string& type,
              bool default_val, const std::string& flags = "",
              const std::string& description = "") {
        settings.add_parameter_from_strings(name, type, ParamValue(default_val), flags, description);
    }
    
    Settings settings;
    SettingsProxy proxy;
};

TEST_SUITE("Settings") {
    TEST_CASE("Parameter storage") {
        Settings settings;
        
        SUBCASE("Basic storage") {
            settings.add_parameter(ParamDef::create_ratio("speed", 0.5f, Flags::NONE, ""));
            CHECK(settings.has_parameter("speed"));
            CHECK(settings.get_value("speed").as_float() == 0.5f);
        }

        SUBCASE("Metadata storage") {
            settings.add_parameter(ParamDef::create_ratio("speed", 0.5f, Flags::CLAMP, "Speed control"));
            const auto& def = settings.get_metadata("speed");
            CHECK(def.type == ParamType::ratio);
            CHECK(def.has_flag(Flags::CLAMP));
            CHECK(def.description == "Speed control");
        }
    }

    TEST_CASE("Parameter loading") {
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
            base.add_parameter(ParamDef::create_ratio("speed", 0.5f, Flags::CLAMP, ""));
            derived.inherit_from(base);
            CHECK(derived.has_parameter("speed"));
            CHECK(derived.get_metadata("speed").has_flag(Flags::CLAMP));
        }

        SUBCASE("Override behavior") {
            base.add_parameter(ParamDef::create_ratio("speed", 0.5f, Flags::CLAMP, ""));
            derived.inherit_from(base);
            derived.add_parameter(ParamDef::create_ratio("speed", 0.8f, Flags::WRAP, ""));
            CHECK(derived.get_value("speed").as_float() == 0.8f);
            CHECK(derived.get_metadata("speed").has_flag(Flags::WRAP));
        }
    }

    TEST_CASE("Settings proxy") {
        Settings settings;
        SettingsProxy proxy(settings);
        
        SUBCASE("Type-safe access") {
            settings.add_parameter(ParamDef::create_ratio("speed", 0.5f, Flags::NONE, ""));
            proxy["speed"] = 0.75f;
            float speed = proxy["speed"];
            CHECK(speed == doctest::Approx(0.75f));
            
            // Test uint8_t conversion
            settings.add_parameter(ParamDef::create_count("fade", 1, 20, 5, Flags::CLAMP, ""));
            proxy["fade"] = 10;
            uint8_t fade = proxy["fade"];
            CHECK(fade == 10);
        }

        SUBCASE("Validation") {
            settings.add_parameter(ParamDef::create_ratio("speed", 0.5f, Flags::CLAMP, ""));
            proxy["speed"] = 1.5f;  // Should clamp
            CHECK(float(proxy["speed"]) == 1.0f);
        }
    }

    TEST_CASE("Parameter range from flags") {
        SUBCASE("Count parameter with range in flags") {
            Settings settings;
            
            // Add a count parameter with range in flags
            settings.add_count_parameter("count_param", 1, 10, 5, "clamp");
            
            // Verify the parameter was created with the correct range
            const auto& def = settings.get_metadata("count_param");
            CHECK(def.type == ParamType::count);
            CHECK(static_cast<int>(def.min_value) == 1);
            CHECK(static_cast<int>(def.max_value) == 10);
            CHECK(def.default_int == 5);
            
            // Test value clamping
            settings.set_value("count_param", ParamValue(15));
            CHECK(settings.get_value("count_param").as_int() == 10);  // Should be clamped to max
            
            settings.set_value("count_param", ParamValue(0));
            CHECK(settings.get_value("count_param").as_int() == 1);  // Should be clamped to min
        }
        
        SUBCASE("Range parameter with direct method") {
            Settings settings;
            
            // Add a range parameter with direct method
            settings.add_range_parameter("range_param", 1.0f, 10.0f, 5.0f, "clamp");
            
            // Verify the parameter was created with the correct range
            const auto& def = settings.get_metadata("range_param");
            CHECK(def.type == ParamType::range);
            CHECK(def.min_value == doctest::Approx(1.0f));
            CHECK(def.max_value == doctest::Approx(10.0f));
            CHECK(def.default_float == doctest::Approx(5.0f));
            
            // Test value clamping
            settings.set_value("range_param", ParamValue(15.0f));
            CHECK(settings.get_value("range_param").as_float() == doctest::Approx(10.0f));  // Should be clamped to max
            
            settings.set_value("range_param", ParamValue(0.0f));
            CHECK(settings.get_value("range_param").as_float() == doctest::Approx(1.0f));  // Should be clamped to min
        }
    }

    TEST_CASE("Parameter reset") {
        Settings settings;
        
        SUBCASE("Basic reset") {
            // Add parameters with default values
            settings.add_parameter(ParamDef::create_ratio("speed", 0.5f, Flags::NONE, ""));
            settings.add_parameter(ParamDef::create_count("count", 1, 10, 5, Flags::NONE, ""));
            
            // Modify the values
            settings.set_value("speed", ParamValue(0.8f));
            settings.set_value("count", ParamValue(7));
            
            // Verify the modifications were applied
            CHECK(settings.get_value("speed").as_float() == doctest::Approx(0.8f));
            CHECK(settings.get_value("count").as_int() == 7);
            
            // Reset all parameters
            settings.reset_all();
            
            // Verify the values were reset to defaults
            CHECK(settings.get_value("speed").as_float() == doctest::Approx(0.5f));
            CHECK(settings.get_value("count").as_int() == 5);
        }
    }
}

TEST_SUITE("Scene Parameter Methods") {
    TEST_CASE("Scene param method") {
        TestScene scene;
        
        SUBCASE("Count parameter with min/max") {
            scene.param("count_param", "count", 1, 10, 5, "clamp");
            
            // Verify the parameter was created with the correct range
            const auto& def = scene.settings.get_metadata("count_param");
            CHECK(def.type == ParamType::count);
            CHECK(static_cast<int>(def.min_value) == 1);
            CHECK(static_cast<int>(def.max_value) == 10);
            CHECK(def.default_int == 5);
            
            // Test value clamping
            scene.proxy["count_param"] = 15;
            CHECK(int(scene.proxy["count_param"]) == 10);  // Should be clamped to max
            
            scene.proxy["count_param"] = 0;
            CHECK(int(scene.proxy["count_param"]) == 1);  // Should be clamped to min
        }
    }
} 