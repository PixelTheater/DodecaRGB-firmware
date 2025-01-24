#include "settings.h"

float Settings::get(const std::string& name) const {
    const auto* param = _params.get(name);
    if (!param) {
        throw std::invalid_argument("Unknown parameter: " + name);
    }
    
    auto it = _values.find(name);
    return it != _values.end() ? it->second : param->default_value;
}

Settings& Settings::set(const std::string& name, float value) {
    const auto* param = _params.get(name);
    if (!param) {
        throw std::invalid_argument("Unknown parameter: " + name);
    }
    
    if (!param->isValid(value)) {
        throw std::invalid_argument(
            "Invalid value " + std::to_string(value) + 
            " for parameter '" + name + "'"
        );
    }
    
    _values[name] = value;
    return *this;
} 