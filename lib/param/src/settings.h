#pragma once

#include <string>
#include <unordered_map>
#include "param_collection.h"
#include <memory>

class Settings;  // Forward declare
class ParamRef;  // Forward declare

class Settings {
public:
    Settings(const ParameterCollection& params) : _params(params) {}
    
    // Proxy class for [] operator
    class Proxy {
    public:
        Proxy(Settings& settings, const std::string& name)
            : _settings(settings), _name(name) {}
        
        // Assignment
        template<typename T>
        Settings& operator=(T value) {
            return _settings.set(_name, value);
        }
        
        // Automatic conversion to float
        operator float() const { 
            return _settings.get(_name); 
        }
        
        // Get typed instance
        template<typename T>
        const T& as() const {
            return *_settings.get<T>(_name);
        }
        
    private:
        Settings& _settings;
        std::string _name;
    };
    
    // [] operator returns proxy
    Proxy operator[](const std::string& name) {
        return Proxy(*this, name);
    }
    
    // Core get/set methods
    float get(const std::string& name) const;
    
    template<typename T>
    const T* get(const std::string& name) const {
        const auto* param = _params.get(name);
        if (!param || !param->isInstanceOf<T>()) {
            throw std::bad_cast();
        }
        auto it = _instance_values.find(name);
        return it != _instance_values.end() ? 
            static_cast<const T*>(it->second.get()) :
            param->getInstance<T>();
    }
    
    Settings& set(const std::string& name, float value);
    
    template<typename T>
    Settings& set(const std::string& name, const T& value) {
        const auto* param = _params.get(name);
        if (!param) {
            throw std::invalid_argument("Unknown parameter: " + name);
        }
        
        if (param->type == ParamType::Instance) {
            if (!param->isInstanceOf<T>()) {
                throw std::bad_cast();
            }
            _instance_values[name] = std::make_shared<T>(value);
            return *this;
        }
        
        // ... handle numeric values ...
        return *this;
    }

private:
    const ParameterCollection& _params;
    std::unordered_map<std::string, float> _values;
    std::unordered_map<std::string, std::shared_ptr<void>> _instance_values;
};

// Now define ParamRef after Settings
class ParamRef {
public:
    ParamRef(Settings& settings, const std::string& name)
        : _settings(settings), _name(name) {}
    
    // Get value
    operator float() const { return _settings.get(_name); }
    
    // Get typed value
    template<typename T>
    const T& as() const { return _settings.get<T>(_name); }
    
    // Set value and return ref for chaining
    template<typename T>
    Settings& set(const T& value) {
        return _settings.set(_name, value);
    }

private:
    Settings& _settings;
    const std::string& _name;
}; 