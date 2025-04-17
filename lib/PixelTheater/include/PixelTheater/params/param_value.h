#pragma once
#include "param_types.h"
#include "PixelTheater/core/math_platform.h"
#include "PixelTheater/core/log.h"
#include "PixelTheater/params/handlers/sentinel_handler.h"
#include "PixelTheater/params/handlers/type_handler.h"

namespace PixelTheater {

using ParamHandlers::SentinelHandler;  // Add this to simplify usage

// ParamValue - Type-safe container for animation parameter values
// 
// This class is used to store and maniplulate the values of scene parameters.
//  - enables simpler syntax for accesing settings values from a running Scene (without casts)
//  - allows safe changes to settings values at runtime
//  - used for range checks and flag-based behavior (clamping, wrapping, etc)
//  - used to validate default values when parameters are defined

class ParamValue {
public:
    // Simple value storage with type safety
    constexpr ParamValue(float v) : _type(ParamType::range), _float_val(v) {}
    constexpr ParamValue(int v) : _type(ParamType::count), _int_val(v) {}
    constexpr ParamValue(bool v) : _type(ParamType::switch_type), _bool_val(v) {}

    // Add default constructor - initialize as float 0.0
    constexpr ParamValue() : _type(ParamType::range), _float_val(0.0f) {}

    // Type-safe getters
    constexpr float as_float() const {
        switch (_type) {
            case ParamType::range:
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
                // Check for NaN/Inf at runtime
                if (is_nan(_float_val) || is_inf(_float_val)) {
                    return SentinelHandler::get_sentinel<float>();
                }
                return _float_val;
            default:
                return SentinelHandler::get_sentinel<float>();
        }
    }

    constexpr int as_int() const {
        switch(_type) {
            case ParamType::count:
            case ParamType::select:
                return _int_val;
            default:
                return SentinelHandler::get_sentinel<int>();
        }
    }

    constexpr bool as_bool() const {
        if (_type == ParamType::switch_type) {
            return _bool_val;
        }
        return SentinelHandler::get_sentinel<bool>();
    }

    // String getter (returns empty string for non-string types)
    const char* as_string() const {
        // if (_type == ParamType::palette) { // <<< REMOVED CHECK
        //     return _str_val;              // <<< REMOVED CHECK
        // }                                 // <<< REMOVED CHECK
        // TODO: Handle 'select' type if it stores string internally?
        return ""; // Return empty string for non-string types currently
    }

    ParamType type() const { return _type; }

    // Type checking
    bool can_convert_to(ParamType target_type) const {
        return ParamHandlers::TypeHandler::can_convert(_type, target_type);
    }

    // Add explicit conversion
    ParamValue convert_to(ParamType target_type) const {
        if (!can_convert_to(target_type)) {
            return ParamHandlers::TypeHandler::get_sentinel_for_type(target_type);
        }
        return *this;
    }

    // Runtime validation
    bool is_valid() const {
        return ParamHandlers::TypeHandler::validate(_type, *this);
    }

private:
    ParamType _type;
    union {
        float _float_val;
        int _int_val;
        bool _bool_val;
        // const char* _str_val;  // Add string storage
    };
};

} // namespace PixelTheater 