#pragma once
#include <string>
#include "yaml_parser.h"
#include "parameter.h"

namespace PixelTheater {

class ParamFactory {
public:
    template<typename T>
    static Parameter<T> create(const std::string& name, const YAMLParser::NodeType& config) {
        std::string min_str = YAMLParser::GetText(config, "range:0");
        std::string max_str = YAMLParser::GetText(config, "range:1");
        std::string default_str = YAMLParser::GetText(config, "default");
        
        if (min_str.empty() || max_str.empty()) {
            throw std::invalid_argument("Invalid range specification for parameter: " + name);
        }

        T min = convert<T>(min_str);
        T max = convert<T>(max_str);
        T default_val = default_str.empty() ? min : convert<T>(default_str);

        return Parameter<T>(name, min, max, default_val);
    }

private:
    template<typename T>
    static T convert(const std::string& str);
    
    template<>
    static float convert(const std::string& str) {
        return std::stof(str);
    }
    
    template<>
    static int convert(const std::string& str) {
        return std::stoi(str);
    }
};

} // namespace PixelTheater 