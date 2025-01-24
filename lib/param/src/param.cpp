#include "param.h"
#include <cmath>

ParamDefinition ParamDefinition::createFloat(const std::string& name, const Range& range, float default_value) {
    return ParamDefinition{name, ParamType::Float, range, default_value};
}

ParamDefinition ParamDefinition::createInt(const std::string& name, int min, int max, int default_value) {
    return ParamDefinition{name, ParamType::Int, Range(float(min), float(max)), float(default_value)};
}

ParamDefinition ParamDefinition::createBool(const std::string& name, bool default_value) {
    return ParamDefinition{name, ParamType::Bool, Range(0.0f, 1.0f), default_value ? 1.0f : 0.0f};
} 