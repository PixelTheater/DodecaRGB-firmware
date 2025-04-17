#include "PixelTheater/params/handlers/type_handler.h"
#include "PixelTheater/params/param_value.h"
#include "PixelTheater/core/math_platform.h"

namespace PixelTheater {
namespace ParamHandlers {

bool TypeHandler::validate(ParamType type, const ParamValue& value) {
    // First check type compatibility
    if (!can_convert(value.type(), type)) {
        return false;
    }

    // Then check value validity based on type
    const auto& info = get_type_info(type);
    
    if (info.has_range) {
        if (is_numeric_type(type)) {
            float val = value.as_float();
            // Check if we got a sentinel value (which includes NaN/Inf)
            return !SentinelHandler::is_sentinel(val);
        } else if (is_integer_type(type)) {
            int val = value.as_int();
            // Check for sentinel values in integers
            return !SentinelHandler::is_sentinel(val);
        }
        // Integer range validation will be handled by RangeHandler
    }

    if (info.has_options) {
        // Option validation will be handled by separate handler
        return true;
    }

    if (info.is_resource) {
        const char* resource = value.as_string();
        return resource != nullptr && resource[0] != '\0';
    }

    return true;
}

bool TypeHandler::can_convert(ParamType from, ParamType to) {
    switch (from) {
        case ParamType::ratio:
        case ParamType::signed_ratio:
        case ParamType::angle:
        case ParamType::signed_angle:
        case ParamType::range:
            return is_numeric_type(to);

        case ParamType::count:
        case ParamType::select:
            return is_integer_type(to);

        case ParamType::switch_type:
            return to == ParamType::switch_type;

        case ParamType::bitmap:
            return is_resource_type(to);

        default:
            return false;
    }
}

ParamValue TypeHandler::get_sentinel_for_type(ParamType type) {
    switch (type) {
        case ParamType::range:
        case ParamType::ratio:
        case ParamType::signed_ratio:
        case ParamType::angle:
        case ParamType::signed_angle:
            return ParamValue(SentinelHandler::get_sentinel<float>());
        case ParamType::count:
        case ParamType::select:
            return ParamValue(SentinelHandler::get_sentinel<int>());
        case ParamType::switch_type:
            return ParamValue(SentinelHandler::get_sentinel<bool>());
        default:
            return ParamValue();
    }
}

const char* TypeHandler::get_name(ParamType type) {
    switch (type) {
        case ParamType::ratio: return "ratio";
        case ParamType::signed_ratio: return "signed_ratio";
        case ParamType::angle: return "angle";
        case ParamType::signed_angle: return "signed_angle";
        case ParamType::range: return "range";
        case ParamType::count: return "count";
        case ParamType::select: return "select";
        case ParamType::switch_type: return "switch";
        case ParamType::bitmap: return "bitmap";
        default: return "unknown";
    }
}

bool TypeHandler::is_numeric_type(ParamType type) {
    return type == ParamType::ratio ||
           type == ParamType::signed_ratio ||
           type == ParamType::angle ||
           type == ParamType::signed_angle ||
           type == ParamType::range;
}

bool TypeHandler::is_integer_type(ParamType type) {
    return type == ParamType::count ||
           type == ParamType::select;
}

bool TypeHandler::is_resource_type(ParamType type) {
    return type == ParamType::bitmap;
}

}} // namespace PixelTheater::ParamHandlers 