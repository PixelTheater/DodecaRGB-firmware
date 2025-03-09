#include "PixelTheater/params/handlers/flag_handler.h"
#include "PixelTheater/params/handlers/type_handler.h"
#include <sstream>

namespace PixelTheater {
namespace ParamHandlers {

bool FlagHandler::validate_flags(ParamFlags flags, ParamType type) {
    // First check for conflicting flags
    if ((flags & Flags::CLAMP) && (flags & Flags::WRAP)) {
        Log::warning("[WARNING] CLAMP and WRAP flags cannot be used together\n");
        return false;
    }

    // Check against type's allowed flags
    ParamFlags allowed = TypeHandler::get_type_info(type).allowed_flags;
    if (flags & ~allowed) {
        Log::warning("[WARNING] Type %s does not support some flags\n", 
            TypeHandler::get_name(type));
        return false;
    }

    return true;
}

bool FlagHandler::has_conflicts(ParamFlags flags) {
    return (flags & Flags::CLAMP) && (flags & Flags::WRAP);
}

ParamFlags FlagHandler::apply_flag_rules(ParamFlags flags) {
    // CLAMP takes precedence over WRAP
    if (flags & Flags::CLAMP) {
        // Remove WRAP flag if present
        flags &= ~Flags::WRAP;
    }
    
    // SLEW can combine with other flags
    // (Will be implemented in future)
    
    return flags;
}

std::string FlagHandler::to_string(ParamFlags flags) {
    std::stringstream ss;
    
    if (flags & Flags::CLAMP) {
        ss << "clamp";
    }
    
    if (flags & Flags::WRAP) {
        if (ss.str().length() > 0) ss << " ";
        ss << "wrap";
    }
    
    // Add other flags as needed
    
    return ss.str();
}

}} // namespace PixelTheater::ParamHandlers 