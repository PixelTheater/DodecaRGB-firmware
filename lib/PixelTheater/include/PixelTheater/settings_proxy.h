#pragma once
#include "params/param_def.h"
#include "params/param_value.h"
#include "settings.h"
#include <string>
#include <vector>

namespace PixelTheater {

// SettingsProxy - A proxy for a Settings instance
//  - Provides helper functions for parameter access from within a scene
//  - Enables a nice syntax for accessing values and metadata of parameters
//  - Enforces type safety, ranges and flags for parameter values

class SettingsProxy {
public:
    // Main proxy that wraps a Settings instance
    SettingsProxy(Settings& settings) : _settings(settings) {}

    // Add a method to reset all parameters
    void reset_all() {
        _settings.reset_all();
    }

    // Parameter proxy returned by operator[]
    class Parameter {
    public:
        Parameter(Settings& settings, const std::string& name)
            : _settings(settings)
            , _name(name)
        {}

        // Direct value access
        operator float() const {
            ParamValue val = _settings.get_value(_name);
            return val.as_float();
        }
        operator int() const {
            ParamValue val = _settings.get_value(_name);
            return val.as_int();
        }
        operator bool() const {
            ParamValue val = _settings.get_value(_name);
            return val.as_bool();
        }
        
        // Add conversion operator for uint8_t
        operator uint8_t() const {
            ParamValue val = _settings.get_value(_name);
            return static_cast<uint8_t>(val.as_int());
        }

        // Direct assignment
        Parameter& operator=(float value) {
            ParamValue val(value);
            if (!_settings.is_valid_value(_name, val)) {
                Log::warning("[WARNING] Parameter '%s': invalid value %.2f. Using sentinel.\n", 
                    _name.c_str(), value);
                val = ParamHandlers::TypeHandler::get_sentinel_for_type(_settings.get_metadata(_name).type);
            }
            _settings.set_value(_name, val);
            return *this;
        }
        Parameter& operator=(int value) {
            ParamValue val(value);
            if (!_settings.is_valid_value(_name, val)) {
                Log::warning("[WARNING] Parameter '%s': invalid value %d. Using sentinel.\n", 
                    _name.c_str(), value);
                val = ParamHandlers::TypeHandler::get_sentinel_for_type(_settings.get_metadata(_name).type);
            }
            _settings.set_value(_name, val);
            return *this;
        }
        Parameter& operator=(bool value) {
            ParamValue val(value);
            if (!_settings.is_valid_value(_name, val)) {
                Log::warning("[WARNING] Parameter '%s': invalid value %s. Using sentinel.\n", 
                    _name.c_str(), value ? "true" : "false");
                val = ParamHandlers::TypeHandler::get_sentinel_for_type(_settings.get_metadata(_name).type);
            }
            _settings.set_value(_name, val);
            return *this;
        }
        Parameter& operator=(const ParamValue& value) {
            if (!_settings.is_valid_value(_name, value)) {
                Log::warning("[WARNING] Parameter '%s': invalid value. Using sentinel.\n", _name.c_str());
                ParamValue val = ParamHandlers::TypeHandler::get_sentinel_for_type(_settings.get_metadata(_name).type);
                _settings.set_value(_name, val);
            } else {
                _settings.set_value(_name, value);
            }
            return *this;
        }

        // Direct metadata access
        float min() const { return _settings.get_metadata(_name).get_min(); }
        float max() const { return _settings.get_metadata(_name).get_max(); }
        bool has_flag(ParamFlags flag) const { return _settings.get_metadata(_name).has_flag(flag); }
        std::string name() const { return _settings.get_metadata(_name).name; }
        std::string description() const { return _settings.get_metadata(_name).description; }

    private:
        Settings& _settings;
        std::string _name;
    };

    // Return Parameter proxy for operator[]
    Parameter operator[](const std::string& name) {
        return Parameter(_settings, name);
    }
    const Parameter operator[](const std::string& name) const { 
        return Parameter(_settings, name); 
    }

    // Direct methods for adding range parameters
    void add_range_parameter(const std::string& name, 
                           float min, float max, float default_val,
                           const std::string& flags = "",
                           const std::string& description = "") {
        _settings.add_range_parameter(name, min, max, default_val, flags, description);
    }
    
    void add_count_parameter(const std::string& name, 
                           int min, int max, int default_val,
                           const std::string& flags = "",
                           const std::string& description = "") {
        _settings.add_count_parameter(name, min, max, default_val, flags, description);
    }
    
    // Forward add_parameter_from_strings to Settings
    void add_parameter_from_strings(const std::string& name, 
                                  const std::string& type,
                                  const ParamValue& default_val, 
                                  const std::string& flags) {
        _settings.add_parameter_from_strings(name, type, default_val, flags);
    }

    // Get all parameter names
    std::vector<std::string> names() const {
        return _settings.get_parameter_names();
    }

    // Check if a parameter exists
    bool has_parameter(const std::string& name) const {
        return _settings.has_parameter(name);
    }

private:
    Settings& _settings;
};

} // namespace PixelTheater 