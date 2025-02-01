#pragma once
#include "param_flags.h"
#include "param_types.h"

namespace PixelTheater {

// Parameter definition structure
struct ParamDef {
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
};

// Simple array literal for options
#define SELECT_OPTIONS(...) \
    (const char*const[]){__VA_ARGS__}

// Parameter definition macros
#define PARAM_SWITCH(name, def, desc) \
    {name, ParamType::switch_type, {.bool_default = def}, Flags::NONE, desc}

#define PARAM_RANGE(name, min, max, def, flags, desc) \
    {name, ParamType::range, {.range_min = min, .range_max = max, .default_val = def}, flags, desc}

#define PARAM_COUNT(name, min, max, def, flags, desc) \
    {name, ParamType::count, {.range_min_i = min, .range_max_i = max, .default_val_i = def}, flags, desc}

#define PARAM_RATIO(name, def, flags, desc) \
    {name, ParamType::ratio, {.float_default = def}, flags, desc}

#define PARAM_ANGLE(name, def, flags, desc) \
    {name, ParamType::angle, {.float_default = def}, flags, desc}

#define PARAM_SELECT(name, def, options, desc) \
    {name, ParamType::select, {.default_idx = def, .options = options}, Flags::NONE, desc}

#define PARAM_PALETTE(name, def, desc) \
    {name, ParamType::palette, {.str_default = def}, Flags::NONE, desc}

} // namespace PixelTheater 