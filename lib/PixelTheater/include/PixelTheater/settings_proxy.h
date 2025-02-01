#pragma once
#include "parameter.h"

namespace PixelTheater {

// Forward declare Settings
class Settings;

class SettingsProxy {
public:
    SettingsProxy(Settings& settings, const std::string& name);

    // Value access - move implementations to cpp file
    operator float() const;
    operator int() const;
    operator bool() const;
    
    // Assignment
    SettingsProxy& operator=(float value);
    SettingsProxy& operator=(int value);
    SettingsProxy& operator=(bool value);
    
    // Parameter interface
    float max() const;
    float min() const;
    float default_value() const;
    const char* description() const;
    const ParamDef& metadata() const;

    // Flag helpers
    bool has_flag(ParamFlags flag) const {
        return Flags::has_flag(metadata().flags, flag);
    }

    // Type-specific getters
    float get_float() const { return operator float(); }
    int get_int() const { return operator int(); }
    bool get_bool() const { return operator bool(); }

private:
    Settings& _settings;
    std::string _name;
};

} // namespace PixelTheater 