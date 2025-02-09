#pragma once
#include "param_flags.h"

// ParamTypes - Defines the types of parameters that can be used in a scene
//  - used to validate default values and ranges
//  - used by ParamValue to determine how to store and manipulate parameter values

namespace PixelTheater {

// Core parameter types
enum class ParamType {
    // Semantic types (fixed ranges)
    ratio,          // 0.0 .. 1.0
    signed_ratio,   // -1.0 .. 1.0
    angle,          // 0.0 .. PI
    signed_angle,   // -PI .. PI
    
    // Range types (custom ranges)
    range,          // min .. max (float)
    count,          // min .. max (int)
    
    // Choice types
    select,         // Named options
    switch_type,    // Boolean
    
    // Resource types
    palette,        // Color palette
    bitmap          // Image data
};

namespace ParamTypes {
    // Move type conversion here
    ParamType from_string(const std::string& type);
}

} // namespace PixelTheater 