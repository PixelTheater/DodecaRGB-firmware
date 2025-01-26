#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <random>
#include <map>
#include "param.h"

namespace Animation {
    // Forward declarations
    class Settings;  // Keep this as forward declaration
    class ParameterCollection;

    // Fluent interface for building parameters
    // Note: template methods set() and as() must be in this header file
    class ParamBuilder {
    public:
        ParamBuilder() : _parent(nullptr) {}
        
        ParamBuilder(const std::string& name) : _name(name) {
            // std::cerr << "ParamBuilder constructor for " << name << std::endl;
        }
        
        // Destructor declaration - implementation in .cpp
        ~ParamBuilder();
        
        // Get the parameter name
        const std::string& getName() const { return _name; }
        
        // Connect builder to settings
        ParamBuilder& setParent(Settings* parent);
        
        // Initialize with parameter
        ParamBuilder& init(ParamDefinition& param) {
            _param = &param;
            return *this;
        }
        
        // Built-in ranges
        ParamBuilder& range(const Range& r) {
            checkState("set range");
            _range = r;
            _has_range = true;
            return *this;
        }
        
        // Custom range
        ParamBuilder& range(float min, float max) {
            checkState("set range");
            _range = Range(min, max);
            _has_range = true;
            return *this;
        }
        
        // Integer range - declaration only
        ParamBuilder& range(int min, int max);
        
        // Value setter
        template<typename T>
        ParamBuilder& set(const T& value) {
            if constexpr (std::is_arithmetic_v<T>) {
                _initial_value = static_cast<float>(value);
                if constexpr (std::is_integral_v<T>) {
                    if (_type != ParamType::Bool) {  // Runtime check
                        _type = ParamType::Int;
                    }
                }
            } else {
                _initial_instance = &value;
                _instance_type = &typeid(T);
            }
            return *this;
        }
        
        // Random value within range
        ParamBuilder& randomize();
        
        // Boolean type
        ParamBuilder& boolean() {
            checkState("set boolean");
            _type = ParamType::Bool;
            return *this;
        }
        
        // Custom type
        template<typename T>
        ParamBuilder& as() {
            checkState("set type");
            _type = ParamType::Instance;
            _instance_type = &typeid(T);
            _initial_value = 0.0f;
            _range = Range(0.0f, 0.0f);
            _initial_instance = nullptr;
            return *this;
        }
        
    public:
        // Build the parameter definition
        ParamDefinition build();

    protected:
        bool _is_built = false;
        bool _has_range = false;
        ParamDefinition* _param = nullptr;
        bool _is_original = true;  // Set to true for the initial builder
        
        void checkState(const char* operation) const {
            if (_is_built) {
                throw std::runtime_error(std::string("Cannot ") + operation + " after build()");
            }
        }

    private:
        std::string _name;
        ParamType _type = ParamType::Float;
        Range _range = Ranges::Ratio;
        float _initial_value = 0.0f;
        const std::type_info* _instance_type = nullptr;
        const void* _initial_instance = nullptr;
        Settings* _parent = nullptr;
    };
} // namespace Animation
