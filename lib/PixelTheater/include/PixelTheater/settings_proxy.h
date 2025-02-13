#pragma once
#include "params/param_def.h"
#include "params/param_value.h"
#include "settings.h"
#include <string>

namespace PixelTheater {

// SettingsProxy - A proxy for a Settings instance
//  - Provides helper functions for parameter access from within a scene
//  - Enables a nice syntax for accessing values and metadata of parameters
//  - Enforces type safety, ranges and flags for parameter values

class SettingsProxy {
public:
    // Main proxy that wraps a Settings instance
    SettingsProxy(Settings& settings) : _settings(settings) {}

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
        const char* name() const { return _settings.get_metadata(_name).name; }
        const char* description() const { return _settings.get_metadata(_name).description; }

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

private:
    Settings& _settings;
};

} // namespace PixelTheater 