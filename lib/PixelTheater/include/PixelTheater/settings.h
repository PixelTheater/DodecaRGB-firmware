#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "parameter.h"
#include "settings_proxy.h"

namespace PixelTheater {

class Settings {
public:
    Settings(const ParamDef* params, size_t count);
    
    // Main interface - operator[]
    SettingsProxy operator[](const std::string& name);
    const SettingsProxy operator[](const std::string& name) const;
    
    // Reset all parameters to defaults
    void reset_all();

    // Allow SettingsProxy to access private members
    friend class SettingsProxy;

private:
    // Internal helpers used by SettingsProxy
    float get_float(const std::string& name) const;
    int get_int(const std::string& name) const;
    bool get_bool(const std::string& name) const;
    
    void set_float(const std::string& name, float value);
    void set_int(const std::string& name, int value);
    void set_bool(const std::string& name, bool value);
    
    const ParamDef& get_metadata(const std::string& name) const;

private:
    std::unordered_map<std::string, std::unique_ptr<IParameter>> _params;
    std::unordered_map<std::string, ParamDef> _metadata;
};

} // namespace PixelTheater 