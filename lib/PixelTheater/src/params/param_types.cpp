#include "PixelTheater/params/param_types.h"
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
    if (type == "palette") return ParamType::palette;
    if (type == "bitmap") return ParamType::bitmap;
    
    throw std::invalid_argument("Unknown parameter type: " + type);
}

} // namespace ParamTypes
} // namespace PixelTheater 