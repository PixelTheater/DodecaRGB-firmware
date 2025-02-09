#pragma once
#include "param_types.h"
#include <stdexcept>

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
        constexpr ParamValue(float v) : _type(ParamType::range), _float_val(v) {}
        constexpr ParamValue(int v) : _type(ParamType::count), _int_val(v) {}
        constexpr ParamValue(bool v) : _type(ParamType::switch_type), _bool_val(v) {}

        // Add default constructor - initialize as float 0.0
        constexpr ParamValue() : _type(ParamType::range), _float_val(0.0f) {}

        // Add string constructor
        constexpr ParamValue(const char* v) : _type(ParamType::palette), _str_val(v) {}

        // Type-safe getters
        constexpr float as_float() const {
            switch(_type) {
                case ParamType::ratio:
                case ParamType::signed_ratio:
                case ParamType::angle:
                case ParamType::signed_angle:
                case ParamType::range:
                    return _float_val;
                default:
                    throw std::bad_cast();
            }
        }

        constexpr int as_int() const {
            switch(_type) {
                case ParamType::count:
                case ParamType::select:
                    return _int_val;
                default:
                    throw std::bad_cast();
            }
        }

        constexpr bool as_bool() const {
            if (_type != ParamType::switch_type) {
                throw std::bad_cast();
            }
            return _bool_val;
        }

        // Add string accessor
        constexpr const char* as_string() const {
            if (_type != ParamType::palette) {
                throw std::bad_cast();
            }
            return _str_val;
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
                throw std::bad_cast();
            }
            return *this;  // Value stays same, just type changes
        }

        void validate_float(float value) {
            // Should throw std::invalid_argument if value is invalid
            if (std::isnan(value) || std::isinf(value)) {
                throw std::invalid_argument("Invalid float value");
            }
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