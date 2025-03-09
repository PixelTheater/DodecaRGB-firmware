#include <doctest/doctest.h>
#include "PixelTheater/scene.h"
#include "../helpers/stage_test_fixture.h"
#include "../fixtures/models/basic_pentagon_model.h"
#include <algorithm>

using namespace PixelTheater;
using namespace PixelTheater::Testing;
using namespace PixelTheater::Fixtures;

// Test scene that adds parameters in setup() rather than config()
template<typename ModelDef>
class ParameterTestScene : public Scene<ModelDef> {
public:
    using Scene<ModelDef>::Scene;
    
    void setup() override {
        // Add parameters in setup() instead of config()
        this->param("speed", "ratio", 0.5f, "clamp", "Controls the animation speed");
        this->param("count", "count", 1, 10, 5, "clamp", "Number of particles");
        this->param("enabled", "switch", true, "", "Enable or disable the effect");
        this->param("placeholder", "ratio", 0.3f, "", "Placeholder parameter"); // Simple placeholder instead of select
        this->param("intensity", "range", 0.0f, 1.0f, 0.7f, "clamp", "Effect intensity");
        
        setup_called = true;
    }
    
    bool setup_called = false;
};

TEST_SUITE("Scene Parameters") {
    TEST_CASE_FIXTURE(StageTestFixture<BasicPentagonModel>, "Parameter Methods") {
        auto* scene = stage->addScene<ParameterTestScene<BasicPentagonModel>>(*stage);
        stage->setScene(scene);
        
        // Call setup to ensure parameters are added
        scene->setup();
        
        // Verify setup was called
        CHECK(scene->setup_called);
        
        SUBCASE("Parameter Names") {
            // Get all parameter names
            auto names = scene->get_parameter_names();
            
            // Check that we have the correct number of parameters
            CHECK(names.size() == 5);
            
            // Check that all parameter names are in the vector
            CHECK(std::find(names.begin(), names.end(), "speed") != names.end());
            CHECK(std::find(names.begin(), names.end(), "count") != names.end());
            CHECK(std::find(names.begin(), names.end(), "enabled") != names.end());
            CHECK(std::find(names.begin(), names.end(), "placeholder") != names.end());
            CHECK(std::find(names.begin(), names.end(), "intensity") != names.end());
            
            // Test the SettingsProxy names() method
            auto proxy_names = scene->settings.names();
            CHECK(proxy_names.size() == 5);
            CHECK(std::find(proxy_names.begin(), proxy_names.end(), "speed") != proxy_names.end());
            CHECK(std::find(proxy_names.begin(), proxy_names.end(), "count") != proxy_names.end());
        }
        
        SUBCASE("Parameter Access") {
            // Access parameters through the settings proxy
            CHECK(float(scene->settings["speed"]) == doctest::Approx(0.5f));
            CHECK(int(scene->settings["count"]) == 5);
            CHECK(bool(scene->settings["enabled"]) == true);
            CHECK(float(scene->settings["placeholder"]) == doctest::Approx(0.3f));
            CHECK(float(scene->settings["intensity"]) == doctest::Approx(0.7f));
            
            // Modify parameters
            scene->settings["speed"] = 0.8f;
            scene->settings["count"] = 7;
            scene->settings["enabled"] = false;
            
            // Check that the modifications were applied
            CHECK(float(scene->settings["speed"]) == doctest::Approx(0.8f));
            CHECK(int(scene->settings["count"]) == 7);
            CHECK(bool(scene->settings["enabled"]) == false);
        }
        
        SUBCASE("Parameter Metadata") {
            // Get parameter metadata
            const auto& speed_metadata = scene->get_parameter_metadata("speed");
            const auto& count_metadata = scene->get_parameter_metadata("count");
            
            // Check parameter types using metadata
            CHECK(speed_metadata.type == ParamType::ratio);
            CHECK(count_metadata.type == ParamType::count);
            
            // Check parameter types using the method
            CHECK(scene->get_parameter_type("speed") == ParamType::ratio);
            CHECK(scene->get_parameter_type("count") == ParamType::count);
            CHECK(scene->get_parameter_type("enabled") == ParamType::switch_type);
            CHECK(scene->get_parameter_type("placeholder") == ParamType::ratio);
            CHECK(scene->get_parameter_type("intensity") == ParamType::range);
            
            // Check parameter ranges - replace range_min_i with min_value cast to int
            CHECK(static_cast<int>(count_metadata.min_value) == 1);
            CHECK(static_cast<int>(count_metadata.max_value) == 10);
            
            // Check parameter flags
            CHECK(speed_metadata.has_flag(Flags::CLAMP));
            CHECK(count_metadata.has_flag(Flags::CLAMP));
        }
        
        SUBCASE("Parameter Reset") {
            // Get original values
            float original_speed = float(scene->settings["speed"]);
            int original_count = int(scene->settings["count"]);
            
            // Modify parameters
            scene->settings["speed"] = 0.8f;
            scene->settings["count"] = 7;
            
            // Verify modifications were applied
            CHECK(float(scene->settings["speed"]) == doctest::Approx(0.8f));
            CHECK(int(scene->settings["count"]) == 7);
            
            // Reset the scene
            scene->reset();
            
            // Check that parameters were reset to default values
            CHECK(float(scene->settings["speed"]) == doctest::Approx(original_speed));
            CHECK(int(scene->settings["count"]) == original_count);
        }
        
        SUBCASE("Parameter Existence") {
            // Check that parameters exist
            CHECK(scene->has_parameter("speed"));
            CHECK(scene->has_parameter("count"));
            CHECK(scene->has_parameter("enabled"));
            CHECK(scene->has_parameter("placeholder"));
            CHECK(scene->has_parameter("intensity"));
            
            // Check that non-existent parameters don't exist
            CHECK_FALSE(scene->has_parameter("non_existent_param"));
            CHECK_FALSE(scene->has_parameter(""));
            
            // Test the SettingsProxy has_parameter() method
            CHECK(scene->settings.has_parameter("speed"));
            CHECK(scene->settings.has_parameter("count"));
            CHECK_FALSE(scene->settings.has_parameter("non_existent_param"));
        }
        
        SUBCASE("Parameter Schema") {
            // Get parameter schema
            auto schema = scene->parameter_schema();
            
            // Check schema properties
            CHECK(schema.scene_name == "Unnamed Scene");
            CHECK(schema.parameters.size() == 5);
            
            // Find the speed parameter
            auto speed_param_it = std::find_if(schema.parameters.begin(), schema.parameters.end(),
                [](const ParameterSchema& param) { return param.name == "speed"; });
            
            // Check that we found the speed parameter
            CHECK(speed_param_it != schema.parameters.end());
            
            // Check speed parameter properties
            CHECK(speed_param_it->name == "speed");
            CHECK(speed_param_it->type == "ratio");
            CHECK(speed_param_it->description == "Controls the animation speed");
            CHECK(speed_param_it->min_value == doctest::Approx(0.0f));
            CHECK(speed_param_it->max_value == doctest::Approx(1.0f));
            CHECK(speed_param_it->default_float == doctest::Approx(0.5f));
            
            // Get JSON representation
            std::string json = scene->parameter_schema_json();
            
            // Basic JSON validation
            CHECK(json.find("\"name\": \"Unnamed Scene\"") != std::string::npos);
            CHECK(json.find("\"parameters\": [") != std::string::npos);
            CHECK(json.find("\"name\": \"speed\"") != std::string::npos);
            CHECK(json.find("\"type\": \"ratio\"") != std::string::npos);
        }
        
        // SUBCASE("Parameter Descriptions") {
        //     // Check that parameter descriptions are correctly set and retrieved
        //     CHECK(scene->_settings_storage.get_description("speed") == "Controls the animation speed");
        //     CHECK(scene->_settings_storage.get_description("count") == "Number of particles");
        //     CHECK(scene->_settings_storage.get_description("enabled") == "Enable or disable the effect");
        //     CHECK(scene->_settings_storage.get_description("placeholder") == "Placeholder parameter");
        //     CHECK(scene->_settings_storage.get_description("intensity") == "Effect intensity");
            
        //     // Check description access through SettingsProxy
        //     CHECK(scene->settings["speed"].description() == "Controls the animation speed");
        //     CHECK(scene->settings["count"].description() == "Number of particles");
        // }
    }
} 