#pragma once
#include "param_types.h"
#include <stdexcept>
#include "PixelTheater/core/math_platform.h"
#include "PixelTheater/core/sentinel.h"
#include "PixelTheater/core/log.h"

// ParamValue - Type-safe container for animation parameter values
// 
// This class is used to store and maniplulate the values of scene parameters.
//  - enables simpler syntax for accesing settings values from a running Scene (without casts)
//  - allows safe changes to settings values at runtime
//  - used for range checks and flag-based behavior (clamping, wrapping, etc)
//  - used to validate default values when parameters are defined

namespace PixelTheater {
    class ParamValue {
    public:
        // Simple value storage with type safety
        constexpr ParamValue(float v) : _type(ParamType::range), _float_val(validate_float(v) ? v : SentinelHandler::get_sentinel<float>()) {}
        constexpr ParamValue(int v) : _type(ParamType::count), _int_val(v) {}
        constexpr ParamValue(bool v) : _type(ParamType::switch_type), _bool_val(v) {}

        // Add default constructor - initialize as float 0.0
        constexpr ParamValue() : _type(ParamType::range), _float_val(0.0f) {}

        // Add string constructor
        constexpr ParamValue(const char* v) : _type(ParamType::palette), _str_val(v) {}

        // Type-safe getters
        constexpr float as_float() const {
            switch (_type) {
                case ParamType::range:
                case ParamType::ratio:
                case ParamType::signed_ratio:
                case ParamType::angle:
                case ParamType::signed_angle:
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

        // Add string accessor
        constexpr const char* as_string() const {
            if (_type == ParamType::palette) {
                return _str_val;
            }
            return SentinelHandler::get_sentinel<const char*>();  // Returns nullptr
        }

        ParamType type() const { return _type; }

        // Type checking
        constexpr bool can_convert_to(ParamType target_type) const {
            switch (_type) {
                case ParamType::ratio:
                case ParamType::signed_ratio:
                case ParamType::angle:
                case ParamType::signed_angle:
                case ParamType::range:
                    return target_type == ParamType::ratio || 
                           target_type == ParamType::signed_ratio ||
                           target_type == ParamType::angle ||
                           target_type == ParamType::signed_angle ||
                           target_type == ParamType::range;
                case ParamType::count:
                case ParamType::select:
                    return target_type == ParamType::count ||
                           target_type == ParamType::select;
                case ParamType::switch_type:
                    return target_type == ParamType::switch_type;
                default:
                    return false;
            }
        }

        // Add explicit conversion
        ParamValue convert_to(ParamType target_type) const {
            if (!can_convert_to(target_type)) {
                // Return a sentinel value of the target type
                switch (target_type) {
                    case ParamType::range:
                    case ParamType::ratio:
                    case ParamType::signed_ratio:
                    case ParamType::angle:
                    case ParamType::signed_angle:
                        return ParamValue(SentinelHandler::get_sentinel<float>());
                    case ParamType::count:
                    case ParamType::select:
                        return ParamValue(SentinelHandler::get_sentinel<int>());
                    case ParamType::switch_type:
                        return ParamValue(SentinelHandler::get_sentinel<bool>());
                    default:
                        return ParamValue();  // Default sentinel
                }
            }
            return *this;
        }

        bool validate_float(float value) {
            if (is_nan(value) || is_inf(value)) {
                Log::warning("[WARNING] Invalid float value: %s. Using sentinel value instead.\n",
                    is_nan(value) ? "NaN" : "Inf");
                return false;
            }
            return true;
        }

    private:
        ParamType _type;
        union {
            float _float_val;
            int _int_val;
            bool _bool_val;
            const char* _str_val;  // Add string storage
        };
    };
} 