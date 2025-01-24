#include "param_builder.h"

ParamDefinition ParamBuilder::build() const {
    switch (_type) {
        case ParamType::Float:
            return ParamDefinition::createFloat(_name, _range, _default);
        case ParamType::Int:
            return ParamDefinition::createInt(_name, int(_range.min), int(_range.max), int(_default));
        case ParamType::Bool:
            return ParamDefinition::createBool(_name, _default != 0.0f);
        case ParamType::Palette:
            return ParamDefinition::createPalette(_name);
        default:
            return ParamDefinition::createFloat(_name, _range, _default);
    }
} 