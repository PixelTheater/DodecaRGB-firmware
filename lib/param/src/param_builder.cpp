#include "param_builder.h"

// ParamDefinition ParamBuilder::build() const {
//     // Validate default value is within range
//     if (_has_default && !_range.contains(_default)) {
//         throw std::invalid_argument(
//             "Default value " + std::to_string(_default) + 
//             " is outside range [" + std::to_string(_range.min) + 
//             ", " + std::to_string(_range.max) + "]"
//         );
//     }

//     switch (_type) {
//         case ParamType::Float:
//             return ParamDefinition::createFloat(_name, _range, _default);
//         case ParamType::Int:
//             return ParamDefinition::createInt(_name, int(_range.min), int(_range.max), int(_default));
//         case ParamType::Bool:
//             return ParamDefinition::createBool(_name, _default != 0.0f);
//         case ParamType::Palette:
//             return ParamDefinition::createPalette(_name);
//         default:
//             return ParamDefinition::createFloat(_name, _range, _default);
//     }
// } 