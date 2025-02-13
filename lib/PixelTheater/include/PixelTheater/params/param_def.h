#pragma once
#include "param_flags.h"
#include "param_types.h"
#include "param_value.h"
#include <algorithm>  // for std::clamp
#include <cmath>  // for applying flags
#include "../constants.h"  // For PI
#include "handlers/range_handler.h"
#include "PixelTheater/params/handlers/flag_handler.h"

// ParamDef - Fully defines a single parameter for a scene (static definition)
//  - tracks parameter metadata (name, type, description, etc)
//  - handles validation, default values, and value transformations (like angle wrapping)
//  - provides configuration methods for declaring parameters in a scene (via YAML or in scene setup)
//  - used by Settings to define how parameter values are stored and manipulated

namespace PixelTheater {

// Parameter definition structure
struct ParamDef {
    // Scene metadata
    struct Metadata {
        const char* name;
        const char* description;
    };

    const char* name;
    ParamType type;
    
    union {
        // Switch parameters
        bool bool_default;
        
        // Float range parameters (range, ratio, angle)
        struct {
            float range_min;
            float range_max;
            float default_val;
        };
        
        // Integer range parameters (count)
        struct {
            int range_min_i;
            int range_max_i;
            int default_val_i;
        };
        
        // Single float value parameters
        float float_default;
        
        // Select parameters
        struct { 
            int default_idx;
            const char* const* options;
        };
        
        // Resource parameters
        const char* str_default;
    };
    
    ParamFlags flags;  // Now a vector<string>
    const char* description;

    // Default constructor
    constexpr ParamDef() 
        : name(""), type(ParamType::ratio), float_default(0), flags(Flags::NONE), description("") 
    {}

    // Basic constructor for float-based parameters (ratio, angle)
    constexpr ParamDef(const char* n, ParamType t, float def, ParamFlags f, const char* d)
        : name(n), type(t), float_default(def), flags(f), description(d)
    {}

    // Range parameters
    constexpr ParamDef(const char* n, ParamType t, float min, float max, float def, ParamFlags f, const char* d)
        : name(n), type(t), range_min(min), range_max(max), default_val(def), flags(f), description(d)
    {}

    // Constructor for count parameters
    constexpr ParamDef(const char* n, ParamType t, int min, int max, int def, ParamFlags f, const char* d)
        : name(n), type(t), range_min_i(min), range_max_i(max), default_val_i(def), flags(f), description(d)
    {}

    // Constructor for switch parameters
    constexpr ParamDef(const char* n, ParamType t, bool def, ParamFlags f, const char* d)
        : name(n), type(t), bool_default(def), flags(f), description(d)
    {}

    // Constructor for select parameters
    constexpr ParamDef(const char* n, ParamType t, int def_idx, const char* const* opts, const char* d)
        : name(n), type(t), default_idx(def_idx), options(opts), flags(Flags::NONE), description(d)
    {}

    // Constructor for palette parameters
    constexpr ParamDef(const char* n, ParamType t, const char* def, const char* d)
        : name(n), type(t), str_default(def), flags(Flags::NONE), description(d)
    {}

    // Move validation to Settings::add_parameter instead
    bool validate_definition() const {
        // Get default value first
        ParamValue default_val = get_default_as_param_value();
        
        // Use handlers to validate both flags and value
        return ParamHandlers::FlagHandler::validate_flags(flags, type) &&
               ParamHandlers::TypeHandler::validate(type, default_val);
    }

    // Get transformed value (called during runtime)
    ParamValue get_transformed_value(const ParamValue& value) const {
        return apply_flags(value);  // Use our own apply_flags
    }

    float get_max() const {
        float min, max;
        ParamHandlers::RangeHandler::get_range(type, min, max);
        return max;
    }

    float get_min() const {
        float min, max;
        ParamHandlers::RangeHandler::get_range(type, min, max);
        return min;
    }

    float get_default() const {
        switch (type) {
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
                return float_default;
            case ParamType::range:
                return default_val;
            case ParamType::count:
                return static_cast<float>(default_val_i);
            case ParamType::switch_type:
                return static_cast<float>(bool_default);
            default:
                Log::warning("[WARNING] Parameter '%s': type has no standard default value. Using sentinel.\n", name);
                return SentinelHandler::get_sentinel<float>();
        }
    }

    bool has_flag(ParamFlags flag) const {
        return Flags::has_flag(flags, flag);
    }

    // Value transformation according to flags
    ParamValue apply_flags(const ParamValue& value) const {
        ParamFlags effective_flags = ParamHandlers::FlagHandler::apply_flag_rules(flags);

        // Get range based on type
        if (ParamHandlers::TypeHandler::has_range(type)) {
            if (type == ParamType::range) {
                return ParamValue(ParamHandlers::RangeHandler::apply_flags(
                    value.as_float(), range_min, range_max, effective_flags));
            }
            // Handle integer types separately
            if (type == ParamType::count || type == ParamType::select) {
                return ParamValue(ParamHandlers::RangeHandler::apply_flags(
                    value.as_int(), range_min_i, range_max_i, effective_flags));
            }
            float min, max;
            ParamHandlers::RangeHandler::get_range(type, min, max);
            return ParamValue(ParamHandlers::RangeHandler::apply_flags(
                value.as_float(), min, max, effective_flags));
        }
        return value; // Non-range types pass through unchanged
    }

public:
    // Move from private to public
    constexpr ParamValue get_default_as_param_value() const {
        switch (type) {
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
                return ParamValue(float_default);
            case ParamType::range:
                return ParamValue(default_val);
            case ParamType::count:
                return ParamValue(default_val_i);
            case ParamType::switch_type:
                return ParamValue(bool_default);
            default:
                return ParamValue(0.0f); // Safe default for constexpr
        }
    }

    // Get value with validation
    ParamValue get_validated_value() const {
        if (!validate_definition()) {
            return ParamHandlers::TypeHandler::get_sentinel_for_type(type);
        }
        return get_default_as_param_value();
    }
};

// Simple array literal for options
#define SELECT_OPTIONS(...) \
    (const char*const[]){__VA_ARGS__}

// Parameter definition macros
#define PARAM_SWITCH(name, def, desc) \
    ParamDef(name, ParamType::switch_type, def, Flags::NONE, desc)

#define PARAM_RANGE(name, min, max, def, flags, desc) \
    ParamDef(name, ParamType::range, min, max, def, flags, desc)

#define PARAM_COUNT(name, min, max, def, flags, desc) \
    ParamDef(name, ParamType::count, min, max, def, flags, desc)

#define PARAM_RATIO(name, def, flags, desc) \
    ParamDef(name, ParamType::ratio, def, flags, desc)

#define PARAM_SIGNED_RATIO(name, def, flags, desc) \
    ParamDef(name, ParamType::signed_ratio, def, flags, desc)

#define PARAM_ANGLE(name, def, flags, desc) \
    ParamDef(name, ParamType::angle, def, flags, desc)

#define PARAM_SIGNED_ANGLE(name, def, flags, desc) \
    ParamDef(name, ParamType::signed_angle, def, flags, desc)

#define PARAM_SELECT(name, def, options, desc) \
    ParamDef(name, ParamType::select, def, options, desc)

#define PARAM_PALETTE(name, def, desc) \
    ParamDef(name, ParamType::palette, def, desc)

// Scene metadata definition macro
#define SCENE_METADATA(name, desc) \
    static constexpr ParamDef::Metadata SCENE_INFO = {name, desc}

} // namespace PixelTheater 