#pragma once
#include <string>
#include "params/param_def.h"
#include "params/param_flags.h"
#include "params/param_range.h"
#include "params/param_types.h"

namespace PixelTheater {

class IParameter {  // Add non-template base
public:
    virtual ~IParameter() = default;
    virtual void reset() = 0;
    virtual const std::string& name() const = 0;
    virtual const ParamDef& metadata() const = 0;
};

// Make base class template
template<typename T>
class ParameterBase : public IParameter {
public:
    virtual ~ParameterBase() = default;
    virtual void reset() = 0;
    virtual const std::string& name() const = 0;
    virtual const ParamDef& metadata() const = 0;
    
    // Add type-safe accessors
    virtual T get() const = 0;
    virtual bool set(const T& value) = 0;
};

template<typename T>
class Parameter : public ParameterBase<T> {
public:
    Parameter(const std::string& name, T min, T max, T default_val = T{}, 
             const ParamFlags& flags = {})
        : _name(name)
        , _range(min, max)
        , _value(default_val)
        , _default(default_val)
        , _flags(flags)  // Just store the flags
    {
        if (!_range.validate(default_val)) {
            throw std::invalid_argument("Default value out of range");
        }
    }

    // Basic parameter interface
    void reset() override { _value = _default; }
    const std::string& name() const override { return _name; }
    T get() const override { return _value; }
    bool set(const T& value) override {
        T temp = value;
        if (!_range.validate(temp)) {
            if (Flags::has_flag(_flags, Flags::CLAMP)) {
                temp = std::clamp(temp, _range.min(), _range.max());
            } 
            else if (Flags::has_flag(_flags, Flags::WRAP)) {
                T range = _range.max() - _range.min();
                while (temp < _range.min()) temp += range;
                while (temp > _range.max()) temp -= range;
            }
            else {
                return false;
            }
        }
        _value = temp;
        return true;
    }

    // Flag access
    const ParamFlags& flags() const { return _flags; }

    const ParamDef& metadata() const override {
        return _metadata;
    }

    T default_value() const { return _default; }

private:
    std::string _name;
    ParamRange<T> _range;
    T _value;
    T _default;
    ParamFlags _flags;
    ParamDef _metadata;  // Store metadata
};

} // namespace PixelTheater 