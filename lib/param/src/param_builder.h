#pragma once

#include <string>
#include <stdexcept>
#include <random>
#include "param.h"

// Fluent interface for building parameters
class ParamBuilder {
public:
    ParamBuilder(const std::string& name) : _name(name) {}
    
    // Get the parameter name
    const std::string& getName() const { return _name; }
    
    // Built-in ranges
    ParamBuilder& range(const Range& r) {
        if (_type == ParamType::Bool) {
            throw std::invalid_argument("Cannot set range on boolean parameter");
        }
        _range = r;
        return *this;
    }
    
    // Custom range (shorthand)
    ParamBuilder& range(float min, float max) {
        if (_type == ParamType::Bool) {
            throw std::invalid_argument("Cannot set range on boolean parameter");
        }
        _range = Range(min, max);
        return *this;
    }
    
    // Default value setter
    template<typename T>
    ParamBuilder& set(const T& value) {
        if (_type == ParamType::Instance) {
            _instance_default = &value;
        } else {
            if constexpr (std::is_arithmetic_v<T>) {
                _default = static_cast<float>(value);
            } else {
                throw std::invalid_argument("Non-numeric type requires .as<T>()");
            }
        }
        _has_default = true;
        return *this;
    }
    
    // Random value within range
    ParamBuilder& randomize() {
        if (_type == ParamType::Bool) {
            throw std::invalid_argument("Cannot randomize boolean parameter");
        }
        if (_type == ParamType::Instance) {
            throw std::invalid_argument("Cannot randomize instance parameter");
        }
        float r = static_cast<float>(rand()) / RAND_MAX;
        _default = _range.min + r * (_range.max - _range.min);
        _has_default = true;
        return *this;
    }
    
    // Boolean type
    ParamBuilder& boolean() {
        _type = ParamType::Bool;
        // Store boolean as float: false=0.0f, true=1.0f
        // This matches our single-storage approach and is converted 
        // back to bool in createBool()
        _default = static_cast<float>(false);
        _has_default = true;
        return *this;
    }
    
    // Custom type
    template<typename T>
    ParamBuilder& as() {
        _type = ParamType::Instance;
        _instance_type = &typeid(T);
        return *this;
    }
    
    // Build the parameter definition
    ParamDefinition build() const {
        if (!_has_default) {
            throw std::invalid_argument("Parameter requires default value");
        }
        
        // Validate default value is within range
        if (_type != ParamType::Instance && !_range.contains(_default)) {
            throw std::invalid_argument(
                "Default value " + std::to_string(_default) + 
                " is outside range [" + std::to_string(_range.min) + 
                ", " + std::to_string(_range.max) + "]"
            );
        }
        
        if (_type == ParamType::Instance) {
            if (!_instance_type || !_instance_default) {
                throw std::invalid_argument("Instance parameter requires type and default value");
            }
            ParamDefinition param{_name, ParamType::Instance, _range, _default};
            param.instance_type = _instance_type;
            param.instance_default = _instance_default;
            return param;
        }
        
        switch (_type) {
            case ParamType::Bool:
                return ParamDefinition::createBool(_name, _default != 0.0f);
            case ParamType::Int:
                return ParamDefinition::createInt(_name, static_cast<int>(_range.min), 
                                                      static_cast<int>(_range.max), 
                                                      static_cast<int>(_default));
            default:
                return ParamDefinition::createFloat(_name, _range, _default);
        }
    }
    
private:
    std::string _name;
    ParamType _type = ParamType::Float;
    Range _range = Ranges::Ratio;  // Default to 0-1 range
    float _default = 0.0f;
    bool _has_default = false;
    const std::type_info* _instance_type = nullptr;
    const void* _instance_default = nullptr;
}; 