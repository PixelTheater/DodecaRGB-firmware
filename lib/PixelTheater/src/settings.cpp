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
    if (!def.validate_definition()) {
        Log::warning("[WARNING] Invalid default value for parameter '%s'. Using sentinel value.\n", def.name);
        _params[def.name] = def;
        _values[def.name] = def.get_sentinel_for_type(def.type);
        return;
    }
    
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
        Log::warning("[WARNING] Parameter not found: %s\n", name.c_str());
        return;  // Silently fail instead of throwing
    }
    
    const ParamDef& def = param_it->second;
    _values[name] = def.apply_flags(value);  // apply_flags already handles errors with sentinels
}

ParamValue Settings::get_value(const std::string& name) const {
    auto value_it = _values.find(name);
    if (value_it == _values.end()) {
        Log::warning("[WARNING] Parameter not found: %s\n", name.c_str());
        return ParamValue();  // Return default sentinel value
    }
    return value_it->second;
}

const ParamDef& Settings::get_metadata(const std::string& name) const {
    auto it = _params.find(name);
    if (it == _params.end()) {
        Log::warning("[WARNING] Parameter not found: %s\n", name.c_str());
        static const ParamDef sentinel_def;  // Returns empty ParamDef
        return sentinel_def;
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
            Log::warning("[WARNING] Range parameters must be defined with min/max values. Parameter '%s' not added.\n", name.c_str());
            break;
        case ParamType::count:
            Log::warning("[WARNING] Count parameters must be defined with min/max values. Parameter '%s' not added.\n", name.c_str());
            break;
        case ParamType::switch_type:
            add_parameter(PARAM_SWITCH(name.c_str(), default_val.as_bool(), ""));
            break;
        case ParamType::select:
            Log::warning("[WARNING] Select parameters must be defined with options. Parameter '%s' not added.\n", name.c_str());
            break;
        case ParamType::palette:
            add_parameter(PARAM_PALETTE(name.c_str(), default_val.as_string(), ""));
            break;
        default:
            Log::warning("[WARNING] Unsupported parameter type for '%s'. Parameter not added.\n", name.c_str());
            break;
    }
}

bool Settings::is_valid_value(const std::string& name, const ParamValue& value) const {
    const ParamDef& def = get_metadata(name);
    
    // Check type compatibility
    if (!value.can_convert_to(def.type)) {
        return false;
    }

    // Check range if applicable
    if (!def.has_flag(Flags::CLAMP) && !def.has_flag(Flags::WRAP)) {
        switch (def.type) {
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
            case ParamType::range:
                return value.as_float() >= def.get_min() && value.as_float() <= def.get_max();
            case ParamType::count:
                return value.as_int() >= def.range_min_i && value.as_int() <= def.range_max_i;
            default:
                return true;  // Other types don't need range validation
        }
    }
    
    return true;  // Value will be clamped/wrapped
}

} // namespace PixelTheater 