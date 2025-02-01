#pragma once
#include <cstdint>

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
}

} // namespace PixelTheater 