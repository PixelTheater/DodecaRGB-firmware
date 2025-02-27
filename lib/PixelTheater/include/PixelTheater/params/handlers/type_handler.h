#pragma once
#include "PixelTheater/params/param_types.h"
#include "sentinel_handler.h"
#include "PixelTheater/core/log.h"

namespace PixelTheater {
    class ParamValue;  // Forward declaration

namespace ParamHandlers {

// Add new struct for type metadata
struct TypeInfo {
    const char* name;
    bool has_range;
    bool has_options;
    bool is_resource;
    const char* description;
    ParamFlags allowed_flags;  // Add supported flags to type info
};

class TypeHandler {
public:
    static bool validate(ParamType type, const ParamValue& value);
    static bool can_convert(ParamType from, ParamType to);
    static ParamValue get_sentinel_for_type(ParamType type);
    static const char* get_name(ParamType type);

    static const TypeInfo& get_type_info(ParamType type) {
        static const TypeInfo type_info[] = {
            // Float types - support numeric flags
            {"ratio", true, false, false, "Value between 0 and 1", Flags::CLAMP | Flags::WRAP},
            {"signed_ratio", true, false, false, "Value between -1 and 1", Flags::CLAMP | Flags::WRAP},
            {"angle", true, false, false, "Angle in radians (0 to PI)", Flags::CLAMP | Flags::WRAP},
            {"signed_angle", true, false, false, "Angle in radians (-PI to PI)", Flags::CLAMP | Flags::WRAP},
            {"range", true, false, false, "Float value with custom range", Flags::CLAMP | Flags::WRAP},

            // Integer types - same numeric flags
            {"count", true, false, false, "Integer value with custom range", Flags::CLAMP | Flags::WRAP},
            {"select", false, true, false, "One of predefined options", Flags::CLAMP | Flags::WRAP},

            // Non-numeric types - no flags
            {"switch", false, false, false, "Boolean value", Flags::NONE},
            {"palette", false, false, true, "Color palette resource", Flags::NONE},
            {"bitmap", false, false, true, "Image resource", Flags::NONE}
        };
        return type_info[static_cast<size_t>(type)];
    }

    // Helper methods
    static bool has_range(ParamType type) {
        return get_type_info(type).has_range;
    }

    static bool has_options(ParamType type) {
        return get_type_info(type).has_options;
    }

    static bool is_resource(ParamType type) {
        return get_type_info(type).is_resource;
    }

    // Add method to check if type supports string-based creation
    static bool can_create_from_strings(ParamType type) {
        switch (type) {
            case ParamType::ratio:
            case ParamType::signed_ratio:
            case ParamType::angle:
            case ParamType::signed_angle:
            case ParamType::switch_type:
            case ParamType::palette:
            case ParamType::count:  // Allow count type
            case ParamType::range:  // Allow range type
                return true;
            case ParamType::select:
                Log::warning("[WARNING] Select parameters require option list\n");
                return false;
            default:
                Log::warning("[WARNING] Unsupported parameter type: %s\n", get_name(type));
                return false;
        }
    }

private:
    static bool is_numeric_type(ParamType type);
    static bool is_integer_type(ParamType type);
    static bool is_resource_type(ParamType type);
};

}} // namespace PixelTheater::ParamHandlers 