#pragma once
#include "PixelTheater/params/param_types.h"
#include "PixelTheater/params/param_flags.h"
#include "PixelTheater/params/handlers/sentinel_handler.h"
#include "PixelTheater/constants.h"
#include "PixelTheater/core/math_platform.h"
#include "PixelTheater/core/math.h"
#include "PixelTheater/core/log.h"

namespace PixelTheater {
namespace ParamHandlers {

class RangeHandler {
public:
    // Range validation
    static bool validate(ParamType type, float value, float min, float max) {
        if (value < min || value > max) {
            Log::warning("[WARNING] Value %.2f out of range [%.2f, %.2f]\n", 
                value, min, max);
            return false;
        }
        return true;
    }

    // Use platform-specific constrain implementation
    template<typename T>
    static T clamp(T value, T min, T max) {
        return constrain_value(value, min, max);
    }

    // Value wrapping
    static float wrap(float value, float min, float max) {
        // Handle degenerate case
        if (min == max) return min;

        // Order bounds
        float lo = std::min(min, max);
        float hi = std::max(min, max);
        float range = hi - lo;

        // Avoid division by zero
        if (range == 0) return lo;

        // Normalize to [0,1) range
        float normalized = (value - lo) / range;
        normalized = normalized - std::floor(normalized);  // Proper float modulo

        // Scale back to original range
        return lo + normalized * range;
    }

    static int wrap(int value, int min, int max) {
        // Handle degenerate case
        if (min == max) return min;

        // Order bounds
        int lo = std::min(min, max);
        int hi = std::max(min, max);
        int range = hi - lo + 1;

        // Handle large numbers by first reducing to a smaller range
        // that won't overflow but preserves the same modulo result
        int reduced = value;
        if (value > hi) {
            reduced = lo + ((value - lo) % range);
        } else if (value < lo) {
            reduced = hi - ((lo - value - 1) % range);
        }
        
        // Now we can safely do the wrap
        if (reduced > hi) {
            return lo + (reduced - hi - 1);
        } else if (reduced < lo) {
            return hi - (lo - reduced - 1);
        }
        return reduced;
    }

    // Apply flags to value
    static float apply_flags(float value, float min, float max, ParamFlags flags) {
        if (Flags::has_flag(flags, Flags::CLAMP)) {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
        if (Flags::has_flag(flags, Flags::WRAP)) {
            // Handle degenerate case
            if (min == max) return min;

            // Order bounds
            float lo = std::min(min, max);
            float hi = std::max(min, max);
            float range = hi - lo;

            // Avoid division by zero
            if (range == 0) return lo;

            // Normalize to [0,1) range
            float normalized = (value - lo) / range;
            normalized = normalized - std::floor(normalized);  // Proper float modulo

            // Scale back to original range
            return lo + normalized * range;
        }
        // No flags or invalid value - return sentinel
        if (!validate(ParamType::range, value, min, max)) {
            return SentinelHandler::get_sentinel<float>();
        }
        return value;
    }

    static int apply_flags(int value, int min, int max, ParamFlags flags) {
        if (Flags::has_flag(flags, Flags::CLAMP)) {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
        if (Flags::has_flag(flags, Flags::WRAP)) {
            // Handle degenerate case
            if (min == max) {
                return min;
            }

            // Order bounds
            int lo = std::min(min, max);
            int hi = std::max(min, max);
            int range = hi - lo + 1;

            // Handle large numbers by first reducing to a smaller range
            // that won't overflow but preserves the same modulo result
            int reduced = value;
            if (value > hi) {
                reduced = lo + ((value - lo) % range);
            } else if (value < lo) {
                reduced = hi - ((lo - value - 1) % range);
            }
            
            // Now we can safely do the wrap
            if (reduced > hi) {
                return lo + (reduced - hi - 1);
            } else if (reduced < lo) {
                return hi - (lo - reduced - 1);
            }
            return reduced;
        }
        // No flags or invalid value - return sentinel
        if (!validate(ParamType::count, value, min, max)) {
            return SentinelHandler::get_sentinel<int>();
        }
        return value;
    }

    // Get default range for type
    static void get_range(ParamType type, float& min, float& max) {
        switch (type) {
            case ParamType::ratio:
                min = Constants::RATIO_MIN;
                max = Constants::RATIO_MAX;
                break;
            case ParamType::signed_ratio:
                min = Constants::SIGNED_RATIO_MIN;
                max = Constants::SIGNED_RATIO_MAX;
                break;
            case ParamType::angle:
                min = Constants::ANGLE_MIN;
                max = Constants::ANGLE_MAX;
                break;
            case ParamType::signed_angle:
                min = Constants::SIGNED_ANGLE_MIN;
                max = Constants::SIGNED_ANGLE_MAX;
                break;
            default:
                min = max = SentinelHandler::get_sentinel<float>();
                break;
        }
    }

    // Integer range validation
    static bool validate_int(ParamType type, int value, int min, int max) {
        if (value < min || value > max) {
            Log::warning("[WARNING] Value %d out of range [%d, %d]\n", 
                value, min, max);
            return false;
        }
        return true;
    }
};

}} // namespace PixelTheater::ParamHandlers 