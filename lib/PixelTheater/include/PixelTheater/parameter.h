#pragma once
#include <string>
#include "PixelTheater/params/param_def.h"
#include "PixelTheater/params/param_flags.h"
#include "PixelTheater/params/param_types.h"
#include "PixelTheater/constants.h"  // For PI
#include "PixelTheater/params/handlers/sentinel_handler.h"
#include "PixelTheater/params/handlers/range_handler.h"
#include "PixelTheater/core/log.h"

// Parameter - A single parameter for a scene
//  - connects the static definition of a parameter (ParamDef) to a running scene
//  - provides consistent interface for validation, accessing, and manipulating parameter values
//  - provides access to parameter metadata
// IParameter - Interface for a parameter
//  - Enables polymorphic parameter handling
//  - Provides a consistent interface for parameter operations

/*  Flow Example (simplified):

    // 1. Define parameter
    ParamDef speed_def = PARAM_RATIO("speed", 0.5f, Flags::CLAMP, "Animation speed");

    // 2. Create parameter instance
    Parameter speed_param(speed_def);  // Links to definition

    // 3. Use through interface
    IParameter& param = speed_param;
    param.set_value(0.75f);  // Validates against ParamDef

*/

namespace PixelTheater {

template<typename T>
class IParameter {
public:
    virtual ~IParameter() = default;
    virtual void reset() = 0;
    virtual const std::string& name() const = 0;
    virtual const ParamDef& metadata() const = 0;
    virtual std::unique_ptr<IParameter<T>> clone() const = 0;
    virtual bool is_valid(const T& value) const = 0;
    virtual void set(const T& value) = 0;
    virtual T get() const = 0;
};

// Make base class template
template<typename T>
class ParameterBase : public IParameter<T> {
public:
    virtual ~ParameterBase() = default;
    virtual void reset() = 0;
    virtual const std::string& name() const = 0;
    virtual const ParamDef& metadata() const = 0;
    
    // Add type-safe accessors
    virtual T get() const = 0;
    virtual void set(const T& value) = 0;  // Changed from bool to void
};

template<typename T>
class Parameter : public ParameterBase<T> {
public:
    Parameter(const std::string& name, T min, T max, T default_val,
             const ParamFlags& flags, const ParamDef& metadata)
        : _name(name)
        , _min(min)
        , _max(max)
        , _flags(flags)
        , _metadata(metadata)
    {
        if (!ParamHandlers::RangeHandler::validate(_metadata.type, default_val, _min, _max)) {
            Log::warning("[WARNING] Parameter '%s': default value out of range. Using sentinel value.\n", name.c_str());
            _value = SentinelHandler::get_sentinel<T>();
            _default = SentinelHandler::get_sentinel<T>();
        } else {
            _value = default_val;
            _default = default_val;
        }
    }

    // Reset implementation returns to default
    void reset() override { _value = _default; }
    
    // Value access/modification
    T get() const override { return _value; }
    void set(const T& value) override {
        if (!is_valid(value)) {
            if (Flags::has_flag(_flags, Flags::CLAMP)) {
                _value = std::clamp(value, _min, _max);
            } else if (Flags::has_flag(_flags, Flags::WRAP)) {
                _value = wrap_value(value);
            } else {
                Log::warning("[WARNING] Parameter '%s': value out of range. Using sentinel value.\n", _name.c_str());
                _value = SentinelHandler::get_sentinel<T>();
            }
        } else {
            _value = value;
        }
    }

    // Flag access
    const ParamFlags& flags() const { return _flags; }

    const ParamDef& metadata() const override {
        return _metadata;
    }

    T default_value() const { return _default; }

    // Implement clone
    std::unique_ptr<IParameter<T>> clone() const override {
        return std::make_unique<Parameter<T>>(*this);
    }

    // Implement the pure virtual method
    const std::string& name() const override { return _name; }

    bool is_valid(const T& value) const override {
        switch (_metadata.type) {
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
            case ParamType::range:
                return ParamHandlers::RangeHandler::validate(_metadata.type, value, _min, _max);

            case ParamType::count:
                return value >= _min && value <= _max 
                       && static_cast<int>(value) == value;  // Must be integer

            case ParamType::switch_type:
                return true;  // All bool values are valid

            case ParamType::select:
                return value >= 0 && value < _option_count;  // Must be valid index

            default:
                return false;
        }
    }

    // Helper for select parameters
    void set_option_count(size_t count) {
        _option_count = count;
    }

private:
    std::string _name;
    T _min;
    T _max;
    T _value;      // Current value
    T _default;    // Default value
    ParamFlags _flags;
    const ParamDef& _metadata;
    size_t _option_count{0};  // For select parameters
};

} // namespace PixelTheater 