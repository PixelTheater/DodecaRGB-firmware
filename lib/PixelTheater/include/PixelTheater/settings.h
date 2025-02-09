#pragma once
#include <string>
#include <unordered_map>
#include "params/param_def.h"
#include "params/param_value.h"

// Settings - A collection of parameters for a scene
//  - Manages the state of parameters for a scene
//  - Connects parameter definitions (ParamDef) to parameter values (ParamValue)
//  - Provides a consistent interface for accessing and manipulating parameters

namespace PixelTheater {

class Settings {
public:
    // Constructors
    Settings() = default;
    Settings(const ParamDef* params, size_t count);
    Settings(const Settings& other);
    Settings& operator=(const Settings& other);

    // Value access
    void set_value(const std::string& name, const ParamValue& value);
    ParamValue get_value(const std::string& name) const;

    // Metadata access
    const ParamDef& get_metadata(const std::string& name) const;
    ParamType get_type(const std::string& name) const;
    const char* get_description(const std::string& name) const;

    // Parameter management
    void reset_all();   // reset settings to default values
    void add_parameter(const ParamDef& def);  // add a parameter from a generated YAML definition
    // add a parameter from a manually defined set of strings, used in a scene's setup() method
    void add_parameter_from_strings(const std::string& name, 
                                  const std::string& type,
                                  const ParamValue& default_val, 
                                  const std::string& flags);

private:
    std::unordered_map<std::string, ParamDef> _params;
    std::unordered_map<std::string, ParamValue> _values;
};

} // namespace PixelTheater 