#include "PixelTheater/settings.h"
#include <iostream>
#include <stdexcept>

namespace PixelTheater {

Settings::Settings(const ParamDef* params, size_t count) {
    for (size_t i = 0; i < count; i++) {
        add_parameter(params[i]);
    }
}

Settings::Settings(const Settings& other) {
    _params = other._params;
    _values = other._values;
}

Settings& Settings::operator=(const Settings& other) {
    if (this != &other) {
        _params = other._params;
        _values = other._values;
    }
    return *this;
}

void Settings::add_parameter(const ParamDef& def) {
    // Validate default value first
    if (!def.validate_definition()) {
        throw std::out_of_range("Invalid default value for parameter: " + 
                               std::string(def.name));
    }
    
    // Only store if validation passes
    _params[def.name] = def;
    _values[def.name] = def.get_default_as_param_value();
}

void Settings::reset_all() {
    for (const auto& param : _params) {
        const ParamDef& def = param.second;
        set_value(def.name, ParamValue(def.get_default()));
    }
}

void Settings::set_value(const std::string& name, const ParamValue& value) {
    auto param_it = _params.find(name);
    if (param_it == _params.end()) {
        throw std::invalid_argument("Parameter not found: " + name);
    }
    
    const ParamDef& def = param_it->second;
    try {
        _values[name] = def.apply_flags(value);
    } catch (const std::invalid_argument& e) {
        throw std::out_of_range(e.what());  // Convert to out_of_range
    }
}

ParamValue Settings::get_value(const std::string& name) const {
    auto value_it = _values.find(name);
    if (value_it == _values.end()) {
        throw std::invalid_argument("Parameter not found: " + name);
    }
    return value_it->second;
}

const ParamDef& Settings::get_metadata(const std::string& name) const {
    auto it = _params.find(name);
    if (it == _params.end()) {
        throw std::invalid_argument("Parameter not found: " + name);
    }
    return it->second;
}

ParamType Settings::get_type(const std::string& name) const {
    return get_metadata(name).type;
}

const char* Settings::get_description(const std::string& name) const {
    return get_metadata(name).description;
}

void Settings::add_parameter_from_strings(const std::string& name, const std::string& type,
                                        const ParamValue& default_val, const std::string& flags) {
    ParamType param_type = ParamTypes::from_string(type);
    ParamFlags param_flags = Flags::from_string(flags);
    
    switch (param_type) {
        case ParamType::ratio:
            add_parameter(PARAM_RATIO(name.c_str(), default_val.as_float(), param_flags, ""));
            break;
        case ParamType::signed_ratio:
            add_parameter(PARAM_SIGNED_RATIO(name.c_str(), default_val.as_float(), param_flags, ""));
            break;
        case ParamType::angle:
            add_parameter(PARAM_ANGLE(name.c_str(), default_val.as_float(), param_flags, ""));
            break;
        case ParamType::signed_angle:
            add_parameter(PARAM_SIGNED_ANGLE(name.c_str(), default_val.as_float(), param_flags, ""));
            break;
        case ParamType::range:
            // Note: Range needs min/max which we don't have from strings
            throw std::invalid_argument("Range parameters must be defined with min/max values");
        case ParamType::count:
            // Note: Count needs min/max which we don't have from strings
            throw std::invalid_argument("Count parameters must be defined with min/max values");
        case ParamType::switch_type:
            add_parameter(PARAM_SWITCH(name.c_str(), default_val.as_bool(), ""));
            break;
        case ParamType::select:
            // Note: Select needs options which we don't have from strings
            throw std::invalid_argument("Select parameters must be defined with options");
        case ParamType::palette:
            add_parameter(PARAM_PALETTE(name.c_str(), default_val.as_string(), ""));
            break;
        default:
            throw std::invalid_argument("Unsupported parameter type");
    }
}

} // namespace PixelTheater 