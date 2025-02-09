#pragma once
#include <cstdint>
#include <string>

// ParamFlags - Defines the flags that can be used to modify the behavior of a parameter
//  - used by ParamDef to declare flag-based behavior (clamping, wrapping, etc)
//  - used by ParamDef::apply_flags() to manipulate values
//  - used by Settings to validate flags when parameters are configured

namespace PixelTheater {

using ParamFlags = uint32_t;  // 32 possible flags

namespace Flags {
    // Flag definitions - each is a bit position
    constexpr ParamFlags NONE  = 0;
    constexpr ParamFlags CLAMP = 1 << 0;  // 0x01
    constexpr ParamFlags WRAP  = 1 << 1;  // 0x02
    constexpr ParamFlags SLEW  = 1 << 2;  // 0x04
    // ... room for 29 more flags

    // Helper to check if flag is set
    constexpr bool has_flag(ParamFlags flags, ParamFlags flag) {
        return (flags & flag) == flag;
    }

    // Helper to get flag name
    constexpr const char* get_name(ParamFlags flag) {
        switch (flag) {
            case CLAMP: return "clamp";
            case WRAP:  return "wrap";
            case SLEW:  return "slew";
            default:    return "";
        }
    }

    // Add flag parsing here
    inline ParamFlags from_string(const std::string& flags) {
        ParamFlags bits = NONE;
        if (flags.find("clamp") != std::string::npos) bits |= CLAMP;
        if (flags.find("wrap") != std::string::npos) bits |= WRAP;
        if (flags.find("slew") != std::string::npos) bits |= SLEW;
        return bits;
    }
}

} // namespace PixelTheater 