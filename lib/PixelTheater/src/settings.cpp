#include "PixelTheater/settings.h"
#include <vector>

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
    if (!def.validate_value(def.get_default_value())) {
        Log::warning("[WARNING] Invalid default value for parameter '%s'. Using sentinel value.\n", def.name.c_str());
        _params[def.name] = def;
        _values[def.name] = ParamHandlers::TypeHandler::get_sentinel_for_type(def.type);
        return;
    }
    
    _params[def.name] = def;
    _values[def.name] = def.get_default_value();
}

void Settings::reset_all() {
    for (const auto& param : _params) {
        const std::string& name = param.first;  // Use the map key, which is a valid std::string
        const ParamDef& def = param.second;
        
        // Get the default value and set it
        ParamValue default_value = def.get_default_value();
        _values[name] = default_value;  // Directly set the value in the _values map
    }
}

void Settings::set_value(const std::string& name, const ParamValue& value) {
    auto param_it = _params.find(name);
    if (param_it == _params.end()) {
        Log::warning("[WARNING] Parameter not found: %s\n", name.c_str());
        return;
    }
    
    const ParamDef& def = param_it->second;
    
    // First check if the value's type is compatible with the parameter's type
    if (!ParamHandlers::TypeHandler::can_convert(value.type(), def.type)) {
        Log::warning("[WARNING] Parameter '%s': incompatible type (expected %s, got %s)\n", 
            name.c_str(), ParamHandlers::TypeHandler::get_name(def.type), 
            ParamHandlers::TypeHandler::get_name(value.type()));
        _values[name] = ParamHandlers::TypeHandler::get_sentinel_for_type(def.type);
        return;
    }
    
    // For parameters with CLAMP or WRAP flags, we can skip the validation
    // since the value will be adjusted in apply_flags
    if (def.has_flag(Flags::CLAMP) || def.has_flag(Flags::WRAP)) {
        // Apply flags (which includes clamping if needed) and store the result
        _values[name] = def.apply_flags(value);
        return;
    }
    
    // For parameters without CLAMP or WRAP, we need to validate the value
    // Check for NaN/Inf/sentinel values
    if (!ParamHandlers::TypeHandler::validate(def.type, value)) {
        Log::warning("[WARNING] Parameter '%s': invalid value for type %s\n", 
            name.c_str(), ParamHandlers::TypeHandler::get_name(def.type));
        _values[name] = ParamHandlers::TypeHandler::get_sentinel_for_type(def.type);
        return;
    }
    
    // Apply flags (which includes clamping if needed) and store the result
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

std::string Settings::get_description(const std::string& name) const {
    return get_metadata(name).description;
}

void Settings::add_parameter_from_strings(const std::string& name, const std::string& type,
                                        const ParamValue& default_val, const std::string& flags,
                                        const std::string& description) {
    ParamType param_type = ParamTypes::from_string(type);
    ParamFlags param_flags = Flags::from_string(flags);
    
    if (!ParamHandlers::TypeHandler::can_create_from_strings(param_type)) {
        Log::warning("[WARNING] Cannot create parameter of type: %s\n", type.c_str());
        return;
    }

    // Parse range information from flags if present
    int min_i = 0;
    int max_i = 100;
    float min_f = 0.0f;
    float max_f = 1.0f;
    
    // Extract range information from flags string
    size_t range_pos = flags.find("range=");
    if (range_pos != std::string::npos) {
        size_t start = range_pos + 6; // length of "range="
        size_t comma = flags.find(',', start);
        if (comma != std::string::npos) {
            std::string min_str = flags.substr(start, comma - start);
            std::string max_str = flags.substr(comma + 1);
            
            // Remove trailing flags or semicolons if present
            size_t semicolon = max_str.find(';');
            if (semicolon != std::string::npos) {
                max_str = max_str.substr(0, semicolon);
            }
            
            // Parse values without using exceptions
            char* end_ptr = nullptr;
            if (param_type == ParamType::count || param_type == ParamType::select) {
                // Convert min value
                long min_val = strtol(min_str.c_str(), &end_ptr, 10);
                if (end_ptr != min_str.c_str() && *end_ptr == '\0') {
                    min_i = static_cast<int>(min_val);
                } else {
                    Log::warning("[WARNING] Failed to parse min range value: %s\n", min_str.c_str());
                }
                
                // Convert max value
                long max_val = strtol(max_str.c_str(), &end_ptr, 10);
                if (end_ptr != max_str.c_str() && *end_ptr == '\0') {
                    max_i = static_cast<int>(max_val);
                } else {
                    Log::warning("[WARNING] Failed to parse max range value: %s\n", max_str.c_str());
                }
            } else {
                // Convert min value
                float min_val = strtof(min_str.c_str(), &end_ptr);
                if (end_ptr != min_str.c_str() && *end_ptr == '\0') {
                    min_f = min_val;
                } else {
                    Log::warning("[WARNING] Failed to parse min range value: %s\n", min_str.c_str());
                }
                
                // Convert max value
                float max_val = strtof(max_str.c_str(), &end_ptr);
                if (end_ptr != max_str.c_str() && *end_ptr == '\0') {
                    max_f = max_val;
                } else {
                    Log::warning("[WARNING] Failed to parse max range value: %s\n", max_str.c_str());
                }
            }
        }
    }

    // Create parameter based on type
    ParamDef param_def;
    
    // Handle deprecated types
    if (param_type == ParamType::palette) {
        Log::warning("Palette parameters are deprecated and will be removed in a future version");
        param_def = ParamDef::create_ratio(name, 0.5f, Flags::NONE, description);
    }
    // Handle types with custom ranges
    else if (param_type == ParamType::range) {
        param_def = ParamDef::create_range(name, min_f, max_f, default_val.as_float(), param_flags, description);
    }
    else if (param_type == ParamType::count) {
        param_def = ParamDef::create_count(name, min_i, max_i, default_val.as_int(), param_flags, description);
    }
    // Handle boolean type
    else if (param_type == ParamType::switch_type) {
        param_def = ParamDef::create_switch(name, default_val.as_bool(), description);
    }
    // Handle standard float types
    else if (param_type == ParamType::ratio) {
        param_def = ParamDef::create_ratio(name, default_val.as_float(), param_flags, description);
    }
    else if (param_type == ParamType::signed_ratio) {
        param_def = ParamDef::create_signed_ratio(name, default_val.as_float(), param_flags, description);
    }
    else if (param_type == ParamType::angle) {
        param_def = ParamDef::create_angle(name, default_val.as_float(), param_flags, description);
    }
    else if (param_type == ParamType::signed_angle) {
        param_def = ParamDef::create_signed_angle(name, default_val.as_float(), param_flags, description);
    }
    // Handle unsupported types
    else {
        Log::warning("[WARNING] Unsupported parameter type: %s\n", 
                    ParamHandlers::TypeHandler::get_name(param_type));
        return;
    }
    
    // Add the parameter
    add_parameter(param_def);
}

void Settings::add_range_parameter(const std::string& name, 
                                 float min, float max, float default_val,
                                 const std::string& flags,
                                 const std::string& description) {
    ParamFlags param_flags = Flags::from_string(flags);
    ParamDef def = ParamDef::create_range(name, min, max, default_val, param_flags, description);
    add_parameter(def);
}

void Settings::add_count_parameter(const std::string& name, 
                                 int min, int max, int default_val,
                                 const std::string& flags,
                                 const std::string& description) {
    ParamFlags param_flags = Flags::from_string(flags);
    ParamDef def = ParamDef::create_count(name, min, max, default_val, param_flags, description);
    add_parameter(def);
}

bool Settings::is_valid_value(const std::string& name, const ParamValue& value) const {
    const ParamDef& def = get_metadata(name);
    
    // 1. Type validation
    if (!ParamHandlers::TypeHandler::validate(def.type, value)) {
        Log::warning("[WARNING] Parameter '%s': invalid type (expected %s)\n", 
            name.c_str(), ParamHandlers::TypeHandler::get_name(def.type));
        return false;
    }

    // 2. Range validation
    // If CLAMP or WRAP flags are set, we still need to validate the type,
    // but we don't need to validate the range since the value will be adjusted
    if (ParamHandlers::TypeHandler::has_range(def.type)) {
        // For parameters with CLAMP or WRAP flags, we always return true
        // since the value will be adjusted in apply_flags
        if (def.has_flag(Flags::CLAMP) || def.has_flag(Flags::WRAP)) {
            return true;
        }
        
        // For parameters without CLAMP or WRAP, we need to validate the range
        float min, max;
        if (ParamHandlers::TypeHandler::has_range(def.type)) {
            min = def.min_value;
            max = def.max_value;
        } else {
            ParamHandlers::RangeHandler::get_range(def.type, min, max);
        }
        
        if (!ParamHandlers::RangeHandler::validate(def.type, value.as_float(), min, max)) {
            Log::warning("[WARNING] Parameter '%s': value out of range (%.2f not in [%.2f, %.2f])\n", 
                name.c_str(), value.as_float(), min, max);
            return false;
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

std::vector<std::string> Settings::get_parameter_names() const {
    std::vector<std::string> names;
    names.reserve(_params.size());  // Reserve space for efficiency
    
    for (const auto& pair : _params) {
        names.push_back(pair.first);
    }
    
    return names;
}

} // namespace PixelTheater 