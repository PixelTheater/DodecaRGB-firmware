#include "PixelTheater/params/param_types.h"
#include "PixelTheater/core/log.h"
#include <stdexcept>
#include <string>

namespace PixelTheater {
namespace ParamTypes {

ParamType from_string(const std::string& type) {
    if (type == "ratio") return ParamType::ratio;
    if (type == "signed_ratio") return ParamType::signed_ratio;
    if (type == "switch") return ParamType::switch_type;
    if (type == "angle") return ParamType::angle;
    if (type == "signed_angle") return ParamType::signed_angle;
    if (type == "range") return ParamType::range;
    if (type == "count") return ParamType::count;
    if (type == "select") return ParamType::select;
    if (type == "bitmap") return ParamType::bitmap;
    
    Log::warning("[WARNING] Unknown parameter type: %s\n", type.c_str());
    return ParamType::ratio;  // Return safe default instead of throwing
}

} // namespace ParamTypes
} // namespace PixelTheater 