#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <cmath>
#include <typeinfo>

/* 
This class defines a parameter: the type, range, initial value, and name.
- range: Ranges and Range classes that define Ratio, SignedRatio, Percent, Angle, and SignedAngle
- ParamDefinition: The schema definition of a parameter
- ParamType: The type of the parameter
- ParamDefinition: The schema definition of a parameter
*/

// Define PI constants if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef M_TWO_PI
#define M_TWO_PI (2.0f * M_PI)
#endif

namespace Animation {

// Move Range class inside Animation namespace
class Range {
public:
    float min;
    float max;
    static constexpr float epsilon = 1e-6f;
    
    Range(float min = 0.0f, float max = 1.0f) : min(min), max(max) {}
    
    bool contains(float value) const {
        return value >= min && value <= max;
    }

    float clamp(float value) const {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    bool operator==(const Range& other) const {
        return std::abs(min - other.min) < epsilon && 
               std::abs(max - other.max) < epsilon;
    }

    bool operator!=(const Range& other) const {
        return !(*this == other);
    }
};

namespace Ranges {
    extern const Range Ratio;         // 0 to 1
    extern const Range SignedRatio;   // -1 to 1
    extern const Range Percent;       // 0 to 100
    extern const Range Angle;         // 0 to 2π
    extern const Range SignedAngle;   // -π to π
}

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
    // Core schema data
    std::string name;
    ParamType type = ParamType::Float;
    Range range{0.0f, 1.0f};
    float initial_value = 0.0f;        // Initial numeric value for Float/Int/Bool
    const std::type_info* instance_type = nullptr;  // Type info for Instance params
    const void* initial_instance = nullptr;         // Initial instance value

    // Add default constructor for std::map
    ParamDefinition() = default;

    // Move and copy operations
    ParamDefinition(ParamDefinition&&) = default;
    ParamDefinition& operator=(ParamDefinition&&) = default;
    ParamDefinition(const ParamDefinition&) = default;
    ParamDefinition& operator=(const ParamDefinition&) = default;

    ParamDefinition(
        const std::string& name,
        ParamType type,
        const Range& range,
        float initial_value,
        const std::type_info* instance_type = nullptr,
        const void* initial_instance = nullptr
    ) : name(name), type(type), range(range), 
        initial_value(initial_value),
        instance_type(instance_type),
        initial_instance(initial_instance)
    {}

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
        return static_cast<const T*>(initial_instance);
    }

    // Validate a value against this parameter's constraints
    bool isValid(float value) const {
        if (type == ParamType::Instance) {
            return initial_instance != nullptr;
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
    static ParamDefinition createInstance(
        const std::string& name,
        const std::type_info* type_info,
        const void* default_value
    );
};

} // namespace Animation
