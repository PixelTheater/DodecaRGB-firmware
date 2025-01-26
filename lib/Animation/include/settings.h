#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include "param.h"
#include "param_builder.h"
#include "preset.h"
#include "preset_builder.h"
#include <map>
#include <vector>
#include <regex>
#include <type_traits>

/*
Notes about settings:

The Settings class is responsible for managing the current state of parameters for a scene. 
It stores the values of parameters, provides access to these values, and ensures they are within defined ranges.
The class also handles the definition and application of presets, which are named collections of parameter values.

Settings are accessed via the operator() method, which returns a SettingProxy object. 
Settings are set via the set() method, which takes a parameter name and a value.

The Settings class stores four maps:
1. _param_definitions: Stores the definitions of all parameters, including their types and ranges.
2. _active_values: Stores the current (scene) values of all definedparameters.
3. _active_instances: Stores the current (scene) instances of all custom parameters.
4. _presets: Stores the named collections of parameter values that can be quickly applied to settings.

Helper methods:
- getParam: Retrieves the parameter definition for a given name.
- applyPreset: Applies a named preset to the current settings.
- resetToInitial: Resets a parameter to its initial value.

For more details on how settings are used within the animation system, refer to the "Settings" section in the README.md file.
*/



namespace Animation {

// Forward declarations
class ParamBuilder;
class PresetBuilder;  // Just forward declare here

// Range definitions for parameters
namespace Ranges {
    extern const Range Ratio;
    extern const Range SignedRatio;
    // ... other ranges
}

class Settings {
public:
    Settings() = default;  // Just use default constructor
    
    // Add move operations
    Settings(Settings&&) = default;
    Settings& operator=(Settings&&) = default;
    
    // Keep copy operations deleted (inherited from ParameterCollection)
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    
    // Default float access
    float operator()(const std::string& name) const {
        const auto* param = getParam(name);
        if (!param) {
            throw std::invalid_argument("Unknown parameter: " + name);
        }
        return _active_values.at(name);
    }
    
    // Template operator for type-safe access
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    operator()(const std::string& name) const {
        const auto* param = getParam(name);
        if (!param) {
            throw std::invalid_argument("Unknown parameter: " + name);
        }
        
        float value = _active_values.at(name);
        if constexpr (std::is_same_v<T, bool>) {
            return value > 0.5f;
        } else {
            return static_cast<T>(value);
        }
    }

    // Specialization for instance types
    template<typename T>
    typename std::enable_if<!std::is_arithmetic<T>::value, const T&>::type
    operator()(const std::string& name) const {
        const auto* param = getParam(name);
        if (!param || !param->isInstanceOf<T>()) {
            throw std::bad_cast();
        }
        return *static_cast<const T*>(_active_instances.at(name).get());
    }

    // Instance getter with type checking
    template<typename T>
    const T& getInstance(const std::string& name) const {
        const auto* param = getParam(name);
        if (!param || !param->isInstanceOf<T>()) {
            throw std::bad_cast();
        }
        return *static_cast<const T*>(_active_instances.at(name).get());
    }
    
    // Set parameter value with clamping - arithmetic types
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, Settings&>::type
    set(const std::string& name, T value) {
        const auto* param = getParam(name);
        if (!param) {
            throw std::invalid_argument("Unknown parameter: " + name);
        }
        if constexpr (std::is_same_v<T, bool>) {
            _active_values[name] = value ? 1.0f : 0.0f;
        } else {
            _active_values[name] = param->range.clamp(static_cast<float>(value));
        }
        return *this;
    }

    // Set parameter value - instance types
    template<typename T>
    typename std::enable_if<!std::is_arithmetic<T>::value, Settings&>::type
    set(const std::string& name, const T& value) {
        const auto* param = getParam(name);
        if (!param || !param->isInstanceOf<T>()) {
            throw std::bad_cast();
        }
        _active_instances[name] = std::shared_ptr<T>(const_cast<T*>(&value), [](T*){});  // Store reference
        return *this;
    }

    // Proxy class to handle operator[] assignment
    class SettingProxy {
    public:
        SettingProxy(Settings& settings, const std::string& name)
            : _settings(settings), _name(name) {}
        
        // Constructor for const Settings
        SettingProxy(const Settings& settings, const std::string& name)
            : _settings(const_cast<Settings&>(settings)), _name(name) {}
        
        // Convert to float for reading
        operator float() const { 
            return _settings(_name);
        }
        
        // Allow direct use in printf
        operator double() const { 
            return static_cast<double>(_settings(_name));
        }
        
        // Explicit comparison operators for tests
        bool operator==(float other) const { 
            return _settings(_name) == other;
        }
        bool operator!=(float other) const { return _settings(_name) != other; }
        
        // Assignment for writing
        Settings& operator=(float value) { return _settings.set(_name, value); }
        
        // Template assignment for instance types
        template<typename T>
        Settings& operator=(const T& value) {
            const auto* param = _settings.getParam(_name);
            if (!param || !param->isInstanceOf<T>()) {
                throw std::bad_cast();
            }
            _settings._active_instances[_name] = std::shared_ptr<T>(const_cast<T*>(&value), [](T*){});  // Store reference
            return _settings;
        }
        
        // Template for instance types
        template<typename T>
        const T& as() const {
            const auto* param = _settings.getParam(_name);
            if (!param || !param->isInstanceOf<T>()) {
                throw std::bad_cast();
            }
            return *static_cast<const T*>(_settings._active_instances.at(_name).get());
        }
        
    private:
        Settings& _settings;
        std::string _name;
    };

    // Return proxy for operator[]
    SettingProxy operator[](const std::string& name) { return SettingProxy(*this, name); }
    const SettingProxy operator[](const std::string& name) const { return SettingProxy(*this, name); }
    
    // Preset management
    PresetBuilder createPreset(const std::string& name) {
        return PresetBuilder(name);
    }
    
    void applyPreset(const Preset& preset) {
        for (const auto& [param, value] : preset.values) {
            set(param, value);
        }
        for (const auto& [param, instance] : preset.instance_values) {
            _active_instances[param] = instance;
        }
    }

    ParamBuilder& param(const std::string& name) {
        if (!std::regex_match(name, std::regex("^[a-zA-Z][a-zA-Z0-9-_]*$"))) {
            throw std::invalid_argument("Invalid parameter name: " + name);
        }
        // Check for duplicate parameters
        if (_definitions.find(name) != _definitions.end()) {
            throw std::invalid_argument("Parameter already exists: " + name);
        }
        //std::cerr << "Settings::param() creating builder for " << name << std::endl;

        // Create new builder
        _current_builder = std::make_unique<ParamBuilder>(name);
        _current_builder->setParent(this);
        return *_current_builder;
    }

    // Add parameter to collection
    void addParameter(const ParamDefinition& param) {
        //std::cerr << "Settings::addParameter() for " << param.name << std::endl;
        _definitions[param.name] = param;
        //std::cerr << "Settings now has " << _definitions.size() << " parameters: ";
        for (const auto& [name, _] : _definitions) {
            //std::cerr << name << " ";
        }
        //std::cerr << std::endl;
        if (param.type != ParamType::Instance) {
            _active_values[param.name] = param.initial_value;
        } else {
            // For instance types, also initialize the instance
            _active_instances[param.name] = std::shared_ptr<void>(
                const_cast<void*>(param.initial_instance),
                [](void*) {} // Empty deleter since instance is owned by ParamDefinition
            );
        }
    }

    // Create and store a preset
    void storePreset(const std::string& name, Preset preset) {
        _presets.insert_or_assign(name, std::move(preset));
    }

    // Apply a preset
    void applyPreset(const std::string& name) {
        const auto& preset = _presets.at(name);
        for (const auto& [param, value] : preset.values) {
            set(param, value);
        }
        for (const auto& [param, instance] : preset.instance_values) {
            _active_instances[param] = instance;
        }
    }

    ~Settings() {
        // Build any remaining parameter
        if (_current_builder) {
            try {
                _current_builder->build();
            } catch (const std::exception& e) {
                //std::cerr << "Failed to build parameter in Settings destructor: " << e.what() << std::endl;
            }
        }
    }

private:
    // Core parameter definitions (type, range, initial values)
    std::map<std::string, ParamDefinition> _definitions;

    // Current working values (can be modified by scenes/presets)
    std::unordered_map<std::string, float> _active_values;
    std::unordered_map<std::string, std::shared_ptr<void>> _active_instances;

    // Saved parameter configurations
    std::unordered_map<std::string, Preset> _presets;

    // Internal parameter access
    const ParamDefinition* getParam(const std::string& name) const {
        //  std::cerr << "Looking for parameter " << name << " in settings with " << _definitions.size() << " parameters" << std::endl;
        auto it = _definitions.find(name);
        if (it == _definitions.end()) {
            //std::cerr << "Parameter " << name << " not found!" << std::endl;
            throw std::invalid_argument("Unknown parameter: " + name);
        }
        //std::cerr << "Found parameter " << name << std::endl;
        return &it->second;
    }

    // Reset a parameter to its initial value
    void resetToInitial(const std::string& name);

    friend class PresetBuilder;  // Keep friend declaration

    void finalize() {
        // Nothing to do - validation happens in build()
    }

    std::unique_ptr<ParamBuilder> _current_builder;  // Owns current builder
};


} // namespace Animation 