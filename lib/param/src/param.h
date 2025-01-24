#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <cmath>
#include <typeinfo>

// Define PI constants if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef M_TWO_PI
#define M_TWO_PI (2.0f * M_PI)
#endif

// First define the Range class completely
class Range {
public:
    float min;
    float max;
    static constexpr float epsilon = 1e-6f;
    
    Range(float min, float max) : min(min), max(max) {}
    
    bool contains(float value) const {
        return (value + epsilon >= min) && (value - epsilon <= max);
    }
};

// Now we can use Range in our namespace
namespace Ranges {
    inline const Range Ratio{0.0f, 1.0f};
    inline const Range SignedRatio{-1.0f, 1.0f};
    inline const Range Percent{0.0f, 100.0f};
    inline const Range Angle{0.0f, M_TWO_PI};
    inline const Range SignedAngle{-M_PI, M_PI};
}

// Forward declare FastLED type we need
class CRGBPalette16;

// Parameter value types
enum class ParamType {
    Float,      // Numeric value with range
    Int,        // Integer value with range
    Bool,       // Boolean value
    Instance    // Custom type instance
};

// Defines a parameter and its constraints
class ParamDefinition {
public:
    ParamDefinition(
        const std::string& name_,
        ParamType type_,
        const Range& range_,
        float default_value_
    ) : name(name_), type(type_), range(range_), default_value(default_value_) {}

    const std::string name;
    const ParamType type;
    const Range range;
    const float default_value;

    // Type info for custom types
    const std::type_info* instance_type = nullptr;
    const void* instance_default = nullptr;

    // Type checking for custom types
    template<typename T>
    bool isInstanceOf() const {
        return type == ParamType::Instance && 
               instance_type == &typeid(T);
    }

    // Instance getter with type checking
    template<typename T>
    const T* getInstance() const {
        if (!isInstanceOf<T>()) {
            throw std::bad_cast();
        }
        return static_cast<const T*>(instance_default);
    }

    // Validate a value against this parameter's constraints
    bool isValid(float value) const {
        if (type == ParamType::Instance) {
            return instance_default != nullptr;
        }
        if (type == ParamType::Bool) {
            return value == 0.0f || value == 1.0f;
        }
        return range.contains(value);
    }

    // Factory methods
    static ParamDefinition createFloat(const std::string& name, const Range& range, float default_value);
    static ParamDefinition createInt(const std::string& name, int min, int max, int default_value);
    static ParamDefinition createBool(const std::string& name, bool default_value);
    
    template<typename T>
    static ParamDefinition createInstance(const std::string& name, const void* default_value) {
        ParamDefinition param{name, ParamType::Instance, Range(0.0f, 0.0f), 0.0f};
        param.instance_type = &typeid(T);
        param.instance_default = default_value;
        return param;
    }
}; 