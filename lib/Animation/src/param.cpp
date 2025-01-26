#include "param.h"
#include <cmath>
#include <regex>

namespace Animation {
    ParamDefinition ParamDefinition::createFloat(const std::string& name, const Range& range, float default_value) {
        // Add name validation
        static const std::regex name_pattern("^[a-zA-Z_][a-zA-Z0-9_]*$");
        if (!std::regex_match(name, name_pattern)) {
            throw std::invalid_argument(
                "Invalid parameter name '" + name + "'. " +
                "Names must start with letter/underscore and contain only letters, numbers, and underscores."
            );
        }
        
        // Clamp default value to range
        float clamped = range.clamp(default_value);
        return ParamDefinition{
            name,
            ParamType::Float,
            range,
            clamped,
            nullptr,
            nullptr
        };
    }

    ParamDefinition ParamDefinition::createInt(const std::string& name, int min, int max, int default_value) {
        return ParamDefinition{name, ParamType::Int, Range(float(min), float(max)), float(default_value)};
    }

    ParamDefinition ParamDefinition::createBool(const std::string& name, bool default_value) {
        return ParamDefinition{
            name,
            ParamType::Bool,
            Range(0.0f, 1.0f),
            default_value ? 1.0f : 0.0f,
            nullptr,
            nullptr
        };
    }

    ParamDefinition ParamDefinition::createInstance(
        const std::string& name,
        const std::type_info* type_info,
        const void* default_value
    ) {
        return ParamDefinition{
            name,
            ParamType::Instance,
            Range(0.0f, 0.0f),
            0.0f,
            type_info,
            default_value
        };
    }
} // namespace Animation

namespace Animation {
namespace Ranges {
    const Range Ratio{0.0f, 1.0f};
    const Range SignedRatio{-1.0f, 1.0f};
    const Range Percent{0.0f, 100.0f};
    const Range Angle{0.0f, M_TWO_PI};
    const Range SignedAngle{-M_PI, M_PI};
} // namespace Ranges

/*
 * Common parameter ranges used throughout the animation system:
 * - Ratio: Values between 0 and 1 (e.g., opacity, intensity)
 * - SignedRatio: Values between -1 and 1 (e.g., direction, velocity)
 * - Percent: Values between 0 and 100 (e.g., progress)
 * - Angle: Values between 0 and 2π (e.g., rotation)
 * - SignedAngle: Values between -π and π (e.g., relative angles)
 */
} // namespace Animation 