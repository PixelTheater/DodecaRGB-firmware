#include <doctest/doctest.h>
#include "PixelTheater/parameter.h"
#include "PixelTheater/param_range.h"
#include "PixelTheater/param_factory.h"

using namespace PixelTheater;

const char* TEST_YAML = R"(
controls:
  speed:
    type: float
    range: [-1.0, 1.0]
    default: 0.5
  brightness:
    type: float
    range: [0.0, 1.0]
    default: 0.8
  num_particles:
    type: int
    range: [0, 1000]
    default: 100
)";

struct YAMLFixture {
    YAMLParser::NodeType config;
    
    YAMLFixture() {
        config = YAMLParser::Parse(TEST_YAML);
    }
};

TEST_SUITE("Parameters") {
    TEST_CASE("Parameter ranges can be validated") {
        SUBCASE("Float parameters respect their ranges") {
            ParamRange<float> ratio_range(0.0f, 1.0f);
            
            CHECK(ratio_range.validate(0.5f) == true);
            CHECK(ratio_range.validate(0.0f) == true);
            CHECK(ratio_range.validate(1.0f) == true);
            CHECK(ratio_range.validate(-0.1f) == false);
            CHECK(ratio_range.validate(1.1f) == false);
        }

        SUBCASE("Integer parameters respect their ranges") {
            ParamRange<int> count_range(0, 100);
            
            CHECK(count_range.validate(50) == true);
            CHECK(count_range.validate(0) == true);
            CHECK(count_range.validate(100) == true);
            CHECK(count_range.validate(-1) == false);
            CHECK(count_range.validate(101) == false);
        }
    }

    TEST_CASE("Parameters can be created with ranges") {
        SUBCASE("Float parameter with range") {
            Parameter<float> speed("speed", -1.0f, 1.0f);
            
            CHECK(speed.name() == "speed");
            CHECK(speed.set(0.5f) == true);
            CHECK(speed.get() == 0.5f);
            CHECK(speed.set(2.0f) == false);  // Outside range
        }

        SUBCASE("Parameters have default values") {
            Parameter<float> brightness("brightness", 0.0f, 1.0f, 0.8f);
            
            CHECK(brightness.get() == 0.8f);
            CHECK(brightness.default_value() == 0.8f);
        }
    }
}

TEST_SUITE("Parameter YAML Configuration") {
    TEST_CASE_FIXTURE(YAMLFixture, "Parameters can be loaded from YAML") {
        SUBCASE("Float parameters are parsed correctly") {
            const char* speed_type = YAMLParser::GetText(config, "controls:speed:type");
            const char* speed_min = YAMLParser::GetText(config, "controls:speed:range:0");
            const char* speed_max = YAMLParser::GetText(config, "controls:speed:range:1");
            const char* speed_default = YAMLParser::GetText(config, "controls:speed:default");
            
            CHECK(strcmp(speed_type, "float") == 0);
            CHECK(atof(speed_min) == -1.0f);
            CHECK(atof(speed_max) == 1.0f);
            CHECK(atof(speed_default) == 0.5f);

            Parameter<float> speed(
                "speed",
                atof(speed_min),
                atof(speed_max),
                atof(speed_default)
            );

            CHECK(speed.get() == 0.5f);
            CHECK(speed.range().min() == -1.0f);
            CHECK(speed.range().max() == 1.0f);
        }

        SUBCASE("Integer parameters are parsed correctly") {
            const char* particles_type = YAMLParser::GetText(config, "controls:num_particles:type");
            const char* particles_min = YAMLParser::GetText(config, "controls:num_particles:range:0");
            const char* particles_max = YAMLParser::GetText(config, "controls:num_particles:range:1");
            const char* particles_default = YAMLParser::GetText(config, "controls:num_particles:default");
            
            CHECK(strcmp(particles_type, "int") == 0);
            CHECK(atoi(particles_min) == 0);
            CHECK(atoi(particles_max) == 1000);
            CHECK(atoi(particles_default) == 100);

            Parameter<int> particles(
                "num_particles",
                atoi(particles_min),
                atoi(particles_max),
                atoi(particles_default)
            );

            CHECK(particles.get() == 100);
            CHECK(particles.range().min() == 0);
            CHECK(particles.range().max() == 1000);
        }
    }

    TEST_CASE_FIXTURE(YAMLFixture, "Parameter factory creates parameters from YAML") {
        SUBCASE("Creates float parameter") {
            auto param = ParamFactory::create<float>("speed", config["controls"]["speed"]);
            CHECK(param.name() == "speed");
            CHECK(param.get() == 0.5f);
            CHECK(param.range().min() == -1.0f);
            CHECK(param.range().max() == 1.0f);
        }

        SUBCASE("Handles missing default value") {
            YAML::Node minimal_config = YAML::Load(R"(
                type: float
                range: [-1.0, 1.0]
            )");
            auto param = ParamFactory::create<float>("minimal", minimal_config);
            CHECK(param.get() == -1.0f); // Should use min as default
        }

        SUBCASE("Throws on invalid range") {
            YAML::Node invalid_config = YAML::Load(R"(
                type: float
                range: -1.0
            )");
            CHECK_THROWS_AS(
                ParamFactory::create<float>("invalid", invalid_config),
                std::invalid_argument
            );
        }
    }
} 