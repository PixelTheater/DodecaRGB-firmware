#pragma once
#include <string>
#include "param_range.h"

namespace PixelTheater {

template<typename T>
class Parameter {
public:
    // Use type's default or min value if no default provided
    Parameter(const std::string& name, T min, T max, T default_val = T(), ParamFlags flags = Clamp)
        : _name(name)
        , _range(min, max)
        , _flags(flags)
        // Default rule: if range is -n..n use 0, otherwise use min
        , _value(default_val == T() ? (min < T() && max > T() ? T() : min) : default_val)
        , _default(default_val == T() ? (min < T() && max > T() ? T() : min) : default_val) {
        // Validate default value
        if (!_range.validate(_default)) {
            throw std::invalid_argument("Default value out of range");
        }
    }

    bool set(T value) {
        _value = _range.apply(value, _flags);
        return _range.validate(value);  // Return if original value was in range
    }

    T get() const { return _value; }
    T default_value() const { return _default; }
    const std::string& name() const { return _name; }
    const ParamRange<T>& range() const { return _range; }

    ParamFlags flags() const { return _flags; }

    // Reset to default value
    void reset() {
        _value = _default;
    }

private:
    std::string _name;
    ParamRange<T> _range;
    ParamFlags _flags;
    T _value;
    T _default;
};

} // namespace PixelTheater 