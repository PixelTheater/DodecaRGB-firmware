#include "PixelTheater/settings.h"

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
        _values[def.name] = ParamHandlers::TypeHandler::get_sentinel_for_type(def.type);
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
        return;
    }
    
    const ParamDef& def = param_it->second;
    
    if (!is_valid_value(name, value)) {
        _values[name] = ParamHandlers::TypeHandler::get_sentinel_for_type(def.type);
        return;
    }
    
    _values[name] = def.apply_flags(value);
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
    
    if (!ParamHandlers::TypeHandler::can_create_from_strings(param_type)) {
        return;
    }

    switch (param_type) {
        case ParamType::ratio:
        case ParamType::signed_ratio:
        case ParamType::angle:
        case ParamType::signed_angle:
            add_parameter(ParamDef(name.c_str(), param_type, default_val.as_float(), param_flags, ""));
            break;
        case ParamType::switch_type:
            add_parameter(PARAM_SWITCH(name.c_str(), default_val.as_bool(), ""));
            break;
        case ParamType::palette:
            add_parameter(PARAM_PALETTE(name.c_str(), default_val.as_string(), ""));
            break;
        case ParamType::count:
            add_parameter(PARAM_COUNT(name.c_str(), 0, 100, default_val.as_int(), param_flags, ""));
            break;
        default:  // Add this to handle remaining cases
            break;  // These are already filtered by can_create_from_strings
    }
}

bool Settings::is_valid_value(const std::string& name, const ParamValue& value) const {
    const ParamDef& def = get_metadata(name);
    
    // 1. Type validation
    if (!ParamHandlers::TypeHandler::validate(def.type, value)) {
        Log::warning("[WARNING] Parameter '%s': invalid type (expected %s)\n", 
            name.c_str(), ParamHandlers::TypeHandler::get_name(def.type));
        return false;
    }

    // 2. Range validation (if no CLAMP/WRAP)
    if (!def.has_flag(Flags::CLAMP) && !def.has_flag(Flags::WRAP)) {
        // Let RangeHandler handle all range validation
        if (ParamHandlers::TypeHandler::has_range(def.type)) {
            float min = (def.type == ParamType::range) ? def.range_min : def.get_min();
            float max = (def.type == ParamType::range) ? def.range_max : def.get_max();
            return ParamHandlers::RangeHandler::validate(def.type, value.as_float(), min, max);
        }
    }

    // 3. Flag validation
    return ParamHandlers::FlagHandler::validate_flags(def.flags, def.type);
}

void Settings::inherit_from(const Settings& base) {
    // Copy all parameters and values from base
    _params = base._params;
    _values = base._values;
}

bool Settings::has_parameter(const std::string& name) const {
    return _params.find(name) != _params.end();
}

} // namespace PixelTheater 