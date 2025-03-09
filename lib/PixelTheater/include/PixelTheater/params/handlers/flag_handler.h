#pragma once
#include "PixelTheater/params/param_flags.h"
#include "PixelTheater/params/param_types.h"
#include "PixelTheater/core/log.h"
#include <string>

namespace PixelTheater {
namespace ParamHandlers {

class FlagHandler {
public:
    // Validate flag combinations
    static bool validate_flags(ParamFlags flags, ParamType type);
    
    // Apply flags in correct order
    static ParamFlags apply_flag_rules(ParamFlags flags);
    
    // Check for conflicting flags
    static bool has_conflicts(ParamFlags flags);
    
    // Convert flags to string representation
    static std::string to_string(ParamFlags flags);
};

}} // namespace PixelTheater::ParamHandlers 