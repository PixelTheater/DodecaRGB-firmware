#pragma once
#include <string>
#include "param_range.h"

namespace PixelTheater {

template<typename T>
class Parameter {
public:
    Parameter(const std::string& name, T min, T max, T default_val = T())
        : _name(name)
        , _range(min, max)
        , _value(default_val)
        , _default(default_val) {
        // Validate default value
        if (!_range.validate(default_val)) {
            throw std::invalid_argument("Default value out of range");
        }
    }

    bool set(T value) {
        if (_range.validate(value)) {
            _value = value;
            return true;
        }
        return false;
    }

    T get() const { return _value; }
    T default_value() const { return _default; }
    const std::string& name() const { return _name; }
    const ParamRange<T>& range() const { return _range; }

private:
    std::string _name;
    ParamRange<T> _range;
    T _value;
    T _default;
};

} // namespace PixelTheater 