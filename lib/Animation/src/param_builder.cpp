#include "param_builder.h"
#include "settings.h"  // Now we can include the full definition
#include <iostream>

namespace Animation {

ParamBuilder::~ParamBuilder() {
    //std::cerr << "ParamBuilder destructor for " << _name 
    //    << " (built=" << _is_built 
    //    << ", original=" << _is_original << ")" << std::endl;
    
    if (_parent && !_is_built && !std::uncaught_exceptions()) {
        //std::cerr << "  Auto-building in destructor" << std::endl;
        try {
            build();
            _is_built = true;  // Mark as built after successful build
        } catch (const std::exception& e) {
            //std::cerr << "  Build failed: " << e.what() << std::endl;
        }
    }
}

ParamBuilder& ParamBuilder::setParent(Settings* parent) {
    _parent = parent;
    return *this;
}

ParamBuilder& ParamBuilder::randomize() {
    if (_type == ParamType::Bool) {
        throw std::invalid_argument("Cannot randomize boolean parameter");
    }
    if (_type == ParamType::Instance) {
        throw std::invalid_argument("Cannot randomize instance parameter");
    }
    float r = static_cast<float>(rand()) / RAND_MAX;
    _initial_value = _range.min + r * (_range.max - _range.min);
    return *this;
}

ParamBuilder& ParamBuilder::range(int min, int max) {
    checkState("set range");
    _type = ParamType::Int;  // Only int range sets the type to Int
    _range = Range(static_cast<float>(min), static_cast<float>(max));
    _has_range = true;
    return *this;
}

ParamDefinition ParamBuilder::build() {
    //std::cerr << "Building parameter " << _name << " with type " << static_cast<int>(_type)<< std::endl;
    if (!_parent) {
        throw std::runtime_error("ParamBuilder not connected to Settings");
    }

    // Create parameter based on type first
    ParamDefinition param;
    switch (_type) {
        case ParamType::Float:
            if (!_has_range) {
                throw std::runtime_error("Range must be set for numeric parameter: " + _name);
            }
            param = ParamDefinition::createFloat(_name, _range, _initial_value);
            break;

        case ParamType::Int:
            if (!_has_range) {
                throw std::runtime_error("Range must be set for numeric parameter: " + _name);
            }
            param = ParamDefinition::createInt(
                _name,
                static_cast<int>(_range.min),
                static_cast<int>(_range.max),
                static_cast<int>(_initial_value)
            );
            break;

        case ParamType::Bool:
            param = ParamDefinition::createBool(_name, _initial_value > 0.5f);
            break;

        case ParamType::Instance:
            if (!_instance_type) {
                throw std::runtime_error("Instance parameters must use as<T>(): " + _name);
            }
            param = ParamDefinition::createInstance(_name, _instance_type, _initial_instance);
            break;
    }

    _parent->addParameter(param);
    //std::cerr << "Added parameter " << _name << " to settings" << std::endl;
    _is_built = true;
    return param;
}

} // namespace Animation 