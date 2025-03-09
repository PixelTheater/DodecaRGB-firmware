#pragma once
#include "param_flags.h"
#include "param_types.h"
#include "param_value.h"
#include <string>
#include <vector>
#include "handlers/range_handler.h"
#include "handlers/flag_handler.h"
#include "handlers/type_handler.h"
#include "../constants.h"

// ParamDef - Fully defines a single parameter for a scene (static definition)
//  - tracks parameter metadata (name, type, description, etc)
//  - handles validation, default values, and value transformations (like angle wrapping)
//  - provides configuration methods for declaring parameters in a scene (via YAML or in scene setup)
//  - used by Settings to define how parameter values are stored and manipulated

namespace PixelTheater {

// Simplified ParamDef structure
struct ParamDef {
    std::string name;
    ParamType type;
    std::string type_name;  // Human-readable type name
    
    // Unified storage for range values
    float min_value;
    float max_value;
    
    // Default values (only one is used based on type)
    float default_float;
    int default_int;
    bool default_bool;
    
    std::vector<std::string> options;  // For select type
    
    ParamFlags flags;
    std::string description;
    
    // Default constructor
    ParamDef() 
        : name("")
        , type(ParamType::range)
        , type_name("range")
        , min_value(0.0f)
        , max_value(1.0f)
        , default_float(0.0f)
        , default_int(0)
        , default_bool(false)
        , flags(Flags::NONE)
        , description("")
    {}
    
    // Constructor for basic float types (ratio, angle)
    ParamDef(const std::string& n, ParamType t, float def, ParamFlags f, const std::string& d)
        : name(n)
        , type(t)
        , type_name(ParamHandlers::TypeHandler::get_name(t))
        , default_float(def)
        , default_int(0)
        , default_bool(false)
        , flags(f)
        , description(d)
    {
        // Set range based on type
        float min, max;
        ParamHandlers::RangeHandler::get_range(type, min, max);
        min_value = min;
        max_value = max;
    }
    
    // Constructor for range parameters
    ParamDef(const std::string& n, ParamType t, float min, float max, float def, ParamFlags f, const std::string& d)
        : name(n)
        , type(t)
        , type_name(ParamHandlers::TypeHandler::get_name(t))
        , min_value(min)
        , max_value(max)
        , default_float(def)
        , default_int(0)
        , default_bool(false)
        , flags(f)
        , description(d)
    {}
    
    // Constructor for count parameters
    ParamDef(const std::string& n, ParamType t, int min, int max, int def, ParamFlags f, const std::string& d)
        : name(n)
        , type(t)
        , type_name(ParamHandlers::TypeHandler::get_name(t))
        , min_value(static_cast<float>(min))
        , max_value(static_cast<float>(max))
        , default_float(0.0f)
        , default_int(def)
        , default_bool(false)
        , flags(f)
        , description(d)
    {}
    
    // Constructor for switch parameters
    ParamDef(const std::string& n, ParamType t, bool def, ParamFlags f, const std::string& d)
        : name(n)
        , type(t)
        , type_name(ParamHandlers::TypeHandler::get_name(t))
        , min_value(0.0f)
        , max_value(1.0f)
        , default_float(0.0f)
        , default_int(0)
        , default_bool(def)
        , flags(f)
        , description(d)
    {}
    
    // Constructor for select parameters
    ParamDef(const std::string& n, ParamType t, const std::vector<std::string>& opts, 
             const std::string& default_opt, ParamFlags f, const std::string& d)
        : name(n)
        , type(t)
        , type_name(ParamHandlers::TypeHandler::get_name(t))
        , min_value(0.0f)
        , max_value(static_cast<float>(opts.size() - 1))
        , default_float(0.0f)
        , default_int(0)
        , default_bool(false)
        , options(opts)
        , flags(f)
        , description(d)
    {
        // Find default option index
        for (size_t i = 0; i < options.size(); i++) {
            if (options[i] == default_opt) {
                default_int = static_cast<int>(i);
                break;
            }
        }
    }
    
    // Helper methods
    bool has_flag(ParamFlags flag) const {
        return Flags::has_flag(flags, flag);
    }
    
    bool has_range() const {
        return ParamHandlers::TypeHandler::has_range(type);
    }
    
    bool is_select_type() const {
        return type == ParamType::select;
    }
    
    // Value methods
    float get_min() const {
        return min_value;
    }
    
    float get_max() const {
        return max_value;
    }
    
    ParamValue get_default_value() const {
        switch (type) {
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
            case ParamType::range:
                return ParamValue(default_float);
            case ParamType::count:
            case ParamType::select:
                return ParamValue(default_int);
            case ParamType::switch_type:
                return ParamValue(default_bool);
            default:
                return ParamValue(0.0f); // Safe default
        }
    }
    
    // Value validation and transformation
    bool validate_value(const ParamValue& value) const {
        // Check type compatibility
        if (!value.can_convert_to(type)) {
            return false;
        }
        
        // Check range for range types
        if (has_range()) {
            if (ParamHandlers::TypeHandler::is_float_type(type)) {
                return ParamHandlers::RangeHandler::validate(type, value.as_float(), min_value, max_value);
            } else if (ParamHandlers::TypeHandler::is_int_type(type)) {
                return ParamHandlers::RangeHandler::validate_int(type, value.as_int(), 
                                                               static_cast<int>(min_value), 
                                                               static_cast<int>(max_value));
            }
        }
        
        // Other types are always valid if type matches
        return true;
    }
    
    ParamValue apply_flags(const ParamValue& value) const {
        ParamFlags effective_flags = ParamHandlers::FlagHandler::apply_flag_rules(flags);
        
        // Apply flags based on type
        if (has_range()) {
            if (ParamHandlers::TypeHandler::is_float_type(type)) {
                float result = ParamHandlers::RangeHandler::apply_flags(
                    value.as_float(), min_value, max_value, effective_flags);
                return ParamValue(result);
            } else if (ParamHandlers::TypeHandler::is_int_type(type)) {
                int result = ParamHandlers::RangeHandler::apply_flags(
                    value.as_int(), static_cast<int>(min_value), static_cast<int>(max_value), effective_flags);
                return ParamValue(result);
            }
        }
        
        // Non-range types pass through unchanged
        return value;
    }
    
    // Factory methods
    static ParamDef create_ratio(const std::string& name, float default_val, 
                               ParamFlags flags, const std::string& description) {
        return ParamDef(name, ParamType::ratio, default_val, flags, description);
    }
    
    static ParamDef create_signed_ratio(const std::string& name, float default_val, 
                                      ParamFlags flags, const std::string& description) {
        return ParamDef(name, ParamType::signed_ratio, default_val, flags, description);
    }
    
    static ParamDef create_angle(const std::string& name, float default_val, 
                               ParamFlags flags, const std::string& description) {
        return ParamDef(name, ParamType::angle, default_val, flags, description);
    }
    
    static ParamDef create_signed_angle(const std::string& name, float default_val, 
                                      ParamFlags flags, const std::string& description) {
        return ParamDef(name, ParamType::signed_angle, default_val, flags, description);
    }
    
    static ParamDef create_range(const std::string& name, float min, float max, float default_val, 
                               ParamFlags flags, const std::string& description) {
        return ParamDef(name, ParamType::range, min, max, default_val, flags, description);
    }
    
    static ParamDef create_count(const std::string& name, int min, int max, int default_val, 
                               ParamFlags flags, const std::string& description) {
        return ParamDef(name, ParamType::count, min, max, default_val, flags, description);
    }
    
    static ParamDef create_switch(const std::string& name, bool default_val, 
                                const std::string& description) {
        return ParamDef(name, ParamType::switch_type, default_val, Flags::NONE, description);
    }
    
    static ParamDef create_select(const std::string& name, const std::vector<std::string>& options, 
                                const std::string& default_option, ParamFlags flags, 
                                const std::string& description) {
        return ParamDef(name, ParamType::select, options, default_option, flags, description);
    }
};

} // namespace PixelTheater 