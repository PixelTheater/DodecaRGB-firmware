#pragma once
#include "param_flags.h"
#include "param_types.h"
#include "param_value.h"
#include <algorithm>  // for std::clamp
#include <cmath>  // for applying flags
#include "../constants.h"  // For PI

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

    // Default constructor needed for containers
    constexpr ParamDef() : name(""), type(ParamType::ratio), float_default(0), flags(Flags::NONE), description("") {}

    // Constructor for float-based parameters (ratio, angle)
    constexpr ParamDef(const char* n, ParamType t, float def, ParamFlags f, const char* d) 
        : name(n), type(t), float_default(def), flags(f), description(d) {}

    // Constructor for range parameters
    constexpr ParamDef(const char* n, ParamType t, float min, float max, float def, ParamFlags f, const char* d)
        : name(n), type(t), range_min(min), range_max(max), default_val(def), flags(f), description(d) {}

    // Constructor for count parameters
    constexpr ParamDef(const char* n, ParamType t, int min, int max, int def, ParamFlags f, const char* d)
        : name(n), type(t), range_min_i(min), range_max_i(max), default_val_i(def), flags(f), description(d) {}

    // Constructor for switch parameters
    constexpr ParamDef(const char* n, ParamType t, bool def, ParamFlags f, const char* d)
        : name(n), type(t), bool_default(def), flags(f), description(d) {}

    // Constructor for select parameters
    constexpr ParamDef(const char* n, ParamType t, int def_idx, const char* const* opts, const char* d)
        : name(n), type(t), default_idx(def_idx), options(opts), flags(Flags::NONE), description(d) {}

    // Constructor for palette parameters
    constexpr ParamDef(const char* n, ParamType t, const char* def, const char* d)
        : name(n), type(t), str_default(def), flags(Flags::NONE), description(d) {}

    // Move validation to Settings::add_parameter instead
    constexpr bool validate_definition() const {
        switch (type) {
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
            case ParamType::range:
                return float_default >= get_min() && float_default <= get_max();
            case ParamType::count:
                return default_val_i >= range_min_i && default_val_i <= range_max_i;
            default:
                return true;
        }
    }

    // Get transformed value (called during runtime)
    ParamValue get_transformed_value(const ParamValue& value) const {
        return apply_flags(value);  // Use our own apply_flags
    }

    float get_max() const {
        switch (type) {
            case ParamType::ratio:
                return Constants::RATIO_MAX;
            case ParamType::signed_ratio:
                return Constants::SIGNED_RATIO_MAX;
            case ParamType::angle:
                return Constants::ANGLE_MAX;
            case ParamType::signed_angle:
                return Constants::SIGNED_ANGLE_MAX;
            case ParamType::range:
                return range_max;
            case ParamType::count:
                return static_cast<float>(range_max_i);
            default:
                throw std::invalid_argument("Parameter type has no standard max");
        }
    }

    float get_min() const {
        switch (type) {
            case ParamType::ratio:
                return Constants::RATIO_MIN;
            case ParamType::signed_ratio:
                return Constants::SIGNED_RATIO_MIN;
            case ParamType::angle:
                return Constants::ANGLE_MIN;
            case ParamType::signed_angle:
                return Constants::SIGNED_ANGLE_MIN;
            case ParamType::range:
                return range_min;
            case ParamType::count:
                return static_cast<float>(range_min_i);
            default:
                throw std::invalid_argument("Parameter type has no standard min");
        }
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
                throw std::invalid_argument("Parameter type has no standard default");
        }
    }

    bool has_flag(ParamFlags flag) const {
        return Flags::has_flag(flags, flag);
    }

    // Value transformation according to flags
    ParamValue apply_flags(const ParamValue& value) const {
        if (!value.can_convert_to(type)) {
            throw std::out_of_range("Value has incompatible type");
        }

        // For numeric types, validate range if no CLAMP/WRAP
        if (!has_flag(Flags::CLAMP) && !has_flag(Flags::WRAP)) {
            switch (type) {
                case ParamType::ratio:
                case ParamType::signed_ratio:
                case ParamType::angle:
                case ParamType::signed_angle:
                case ParamType::range: {
                    float val = value.as_float();
                    if (val < get_min() || val > get_max()) {
                        throw std::out_of_range("Value out of range");
                    }
                    break;
                }
                case ParamType::count: {
                    int val = value.as_int();
                    if (val < range_min_i || val > range_max_i) {
                        throw std::out_of_range("Value out of range");
                    }
                    break;
                }
                case ParamType::select:
                case ParamType::switch_type:
                case ParamType::palette:
                    // These types don't need range validation
                    break;
                default:
                    throw std::invalid_argument("Unsupported parameter type");
            }
        }

        // Now apply transformations
        switch (type) {
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
            case ParamType::range: {
                float val = value.as_float();
                if (has_flag(Flags::CLAMP)) {
                    return ParamValue(std::clamp(val, get_min(), get_max()));
                } else if (has_flag(Flags::WRAP)) {
                    float range = get_max() - get_min();
                    val = std::fmod(val - get_min(), range);
                    if (val < 0) val += range;
                    return ParamValue(val + get_min());
                }
                return value;
            }
            default:
                return value;
        }
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