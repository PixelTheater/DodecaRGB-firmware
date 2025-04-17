#include <doctest/doctest.h>
#include "PixelTheater/params/handlers/sentinel_handler.h"
#include "PixelTheater/params/handlers/type_handler.h"
#include "PixelTheater/params/param_value.h"
#include "PixelTheater/params/handlers/range_handler.h"
#include "PixelTheater/params/handlers/flag_handler.h"
#include "PixelTheater/constants.h"

using namespace PixelTheater;
using namespace PixelTheater::ParamHandlers;

TEST_CASE("SentinelHandler") {
    SUBCASE("Basic sentinel values") {
        CHECK(SentinelHandler::get_sentinel<float>() == 0.0f);
        CHECK(SentinelHandler::get_sentinel<int>() == -1);
        CHECK(SentinelHandler::get_sentinel<bool>() == false);
    }

    SUBCASE("Sentinel detection") {
        CHECK(SentinelHandler::is_sentinel(0.0f));
        CHECK(SentinelHandler::is_sentinel(-1));
        CHECK(SentinelHandler::is_sentinel(false));
        
        CHECK_FALSE(SentinelHandler::is_sentinel(1.0f));
        CHECK_FALSE(SentinelHandler::is_sentinel(42));
        CHECK_FALSE(SentinelHandler::is_sentinel(true));
    }

    SUBCASE("Type-specific sentinels") {
        // Test that each type gets its own sentinel
        CHECK(SentinelHandler::is_sentinel(SentinelHandler::get_sentinel<float>()));
        CHECK(SentinelHandler::is_sentinel(SentinelHandler::get_sentinel<int>()));
        CHECK(SentinelHandler::is_sentinel(SentinelHandler::get_sentinel<bool>()));
    }
}

TEST_CASE("TypeHandler") {
    SUBCASE("Type names") {
        CHECK(std::string(TypeHandler::get_name(ParamType::ratio)) == "ratio");
        CHECK(std::string(TypeHandler::get_name(ParamType::signed_ratio)) == "signed_ratio");
        CHECK(std::string(TypeHandler::get_name(ParamType::angle)) == "angle");
        CHECK(std::string(TypeHandler::get_name(ParamType::signed_angle)) == "signed_angle");
        CHECK(std::string(TypeHandler::get_name(ParamType::range)) == "range");
        CHECK(std::string(TypeHandler::get_name(ParamType::count)) == "count");
        CHECK(std::string(TypeHandler::get_name(ParamType::select)) == "select");
        CHECK(std::string(TypeHandler::get_name(ParamType::switch_type)) == "switch");
        CHECK(std::string(TypeHandler::get_name(ParamType::bitmap)) == "bitmap");
    }

    SUBCASE("Type conversion rules") {
        // Keep numeric type conversions
        CHECK(TypeHandler::can_convert(ParamType::ratio, ParamType::range));
        CHECK(TypeHandler::can_convert(ParamType::signed_ratio, ParamType::angle));
        CHECK_FALSE(TypeHandler::can_convert(ParamType::ratio, ParamType::count));

        // Keep integer type conversions
        CHECK(TypeHandler::can_convert(ParamType::count, ParamType::select));
        CHECK_FALSE(TypeHandler::can_convert(ParamType::count, ParamType::ratio));

        // Keep switch type conversions
        CHECK(TypeHandler::can_convert(ParamType::switch_type, ParamType::switch_type));
        CHECK_FALSE(TypeHandler::can_convert(ParamType::switch_type, ParamType::count));
    }

    SUBCASE("Type metadata") {
        // Check type info
        const auto& ratio_info = TypeHandler::get_type_info(ParamType::ratio);
        CHECK(std::string(ratio_info.name) == "ratio");
        CHECK(ratio_info.has_range == true);
        CHECK(ratio_info.has_options == false);
        CHECK(ratio_info.is_resource == false);

        // Check helper methods
        CHECK(TypeHandler::has_range(ParamType::range));
        CHECK(TypeHandler::has_options(ParamType::select));
        CHECK(TypeHandler::is_resource(ParamType::bitmap));
    }

    SUBCASE("Type classification") {
        CHECK(TypeHandler::has_range(ParamType::range));
        CHECK(!TypeHandler::has_range(ParamType::switch_type));
        CHECK(TypeHandler::has_options(ParamType::select));
        CHECK(!TypeHandler::has_options(ParamType::count));
        CHECK(TypeHandler::is_resource(ParamType::bitmap));
        CHECK(!TypeHandler::is_resource(ParamType::ratio));
    }

    SUBCASE("Complete conversion matrix") {
        const ParamType all_types[] = {
            ParamType::ratio, ParamType::signed_ratio,
            ParamType::angle, ParamType::signed_angle,
            ParamType::range, ParamType::count,
            ParamType::select, ParamType::switch_type
        };

        for (auto from : all_types) {
            for (auto to : all_types) {
                bool result = TypeHandler::can_convert(from, to);
                INFO("From: ", TypeHandler::get_name(from), 
                     " To: ", TypeHandler::get_name(to));
                // Verify conversion rules are symmetric
                CHECK(result == TypeHandler::can_convert(to, from));
            }
        }
    }

    SUBCASE("Type validation") {
        // Test numeric validation
        ParamValue valid_float(0.5f);
        CHECK(TypeHandler::validate(ParamType::ratio, valid_float));
        
        ParamValue nan_float(NAN);
        CHECK_FALSE(TypeHandler::validate(ParamType::ratio, nan_float));
    }
}

TEST_CASE("RangeHandler") {
    SUBCASE("Range validation") {
        CHECK(RangeHandler::validate(ParamType::range, 0.5f, 0.0f, 1.0f));
        CHECK_FALSE(RangeHandler::validate(ParamType::range, 1.5f, 0.0f, 1.0f));
        CHECK_FALSE(RangeHandler::validate(ParamType::range, -0.5f, 0.0f, 1.0f));
    }

    SUBCASE("Flag application") {
        // Test CLAMP
        float clamped = RangeHandler::apply_flags(1.5f, 0.0f, 1.0f, Flags::CLAMP);
        CHECK(clamped == 1.0f);

        // Test WRAP
        float wrapped = RangeHandler::apply_flags(1.5f, 0.0f, 1.0f, Flags::WRAP);
        CHECK(wrapped == doctest::Approx(0.5f));

        // Test no flags returns sentinel for out of range
        float invalid = RangeHandler::apply_flags(1.5f, 0.0f, 1.0f, Flags::NONE);
        CHECK(SentinelHandler::is_sentinel(invalid));
    }

    SUBCASE("Default ranges") {
        float min, max;
        
        RangeHandler::get_range(ParamType::ratio, min, max);
        CHECK(min == Constants::RATIO_MIN);
        CHECK(max == Constants::RATIO_MAX);

        RangeHandler::get_range(ParamType::signed_ratio, min, max);
        CHECK(min == Constants::SIGNED_RATIO_MIN);
        CHECK(max == Constants::SIGNED_RATIO_MAX);
    }

    SUBCASE("Integer range validation") {
        CHECK(RangeHandler::validate_int(ParamType::count, 5, 0, 10));
        CHECK_FALSE(RangeHandler::validate_int(ParamType::count, 11, 0, 10));
        CHECK_FALSE(RangeHandler::validate_int(ParamType::count, -1, 0, 10));
    }

    SUBCASE("Integer flag application") {
        // Test CLAMP
        int wrapped = RangeHandler::apply_flags(11, 0, 10, Flags::WRAP);
        CHECK(wrapped == 0);  // back to start

        // Test negative wrap
        int neg_wrapped = RangeHandler::apply_flags(-1, 0, 10, Flags::WRAP);
        CHECK(neg_wrapped == 10);  // Wraps to end

        // Test no flags returns sentinel
        int invalid = RangeHandler::apply_flags(11, 0, 10, Flags::NONE);
        CHECK(SentinelHandler::is_sentinel(invalid));

        // Test edge cases
        CHECK(RangeHandler::apply_flags(42, 5, 5, Flags::WRAP) == 5);
        CHECK(RangeHandler::apply_flags(-1, 5, 5, Flags::WRAP) == 5);

        // Test reversed ranges
        CHECK(RangeHandler::apply_flags(12, 10, 0, Flags::WRAP) == 
              RangeHandler::apply_flags(12, 0, 10, Flags::WRAP));

        // Test large numbers
        CHECK(RangeHandler::apply_flags(100, 0, 10, Flags::WRAP) == 
              RangeHandler::apply_flags(12, 0, 10, Flags::WRAP));  // 100 % 11 = 1

        // Test negative large numbers
        CHECK(RangeHandler::apply_flags(-20, 0, 10, Flags::WRAP) == 
              RangeHandler::apply_flags(2, 0, 10, Flags::WRAP));   // -20 wraps to 2

        // Test flag combinations
        CHECK(RangeHandler::apply_flags(12, 0, 10, Flags::CLAMP | Flags::WRAP) == 10);
    }

    SUBCASE("Basic range operations") {
        // Test normal wrapping
        CHECK(RangeHandler::apply_flags(11.0f, 0.0f, 10.0f, Flags::WRAP) == doctest::Approx(1.0f));
        CHECK(RangeHandler::apply_flags(-1.0f, 0.0f, 10.0f, Flags::WRAP) == doctest::Approx(9.0f));

        // Test normal clamping
        CHECK(RangeHandler::apply_flags(11.0f, 0.0f, 10.0f, Flags::CLAMP) == 10.0f);
        CHECK(RangeHandler::apply_flags(-1.0f, 0.0f, 10.0f, Flags::CLAMP) == 0.0f);
    }

    SUBCASE("Integer range handling") {
        // Test integer wrapping
        CHECK(RangeHandler::apply_flags(100, 0, 10, Flags::WRAP) == 1);  // 100 % 11 = 1
        CHECK(RangeHandler::apply_flags(-1, 0, 10, Flags::WRAP) == 10);

        // Test integer clamping
        CHECK(RangeHandler::apply_flags(100, 0, 10, Flags::CLAMP) == 10);
        CHECK(RangeHandler::apply_flags(-100, 0, 10, Flags::CLAMP) == 0);
    }

    SUBCASE("Edge cases") {
        // Zero-width ranges
        CHECK(RangeHandler::apply_flags(5.0f, 10.0f, 10.0f, Flags::CLAMP) == 10.0f);
        CHECK(RangeHandler::apply_flags(5, 10, 10, Flags::WRAP) == 10);

        // Very large numbers
        CHECK(RangeHandler::apply_flags(1000000, 0, 10, Flags::WRAP) == 
              RangeHandler::apply_flags(1000000 % 11, 0, 10, Flags::WRAP));
    }

    SUBCASE("Flag combinations") {
        // CLAMP takes precedence over WRAP
        ParamFlags flags = Flags::CLAMP | Flags::WRAP;
        CHECK(RangeHandler::apply_flags(11.0f, 0.0f, 10.0f, flags) == 10.0f);
    }

    SUBCASE("Float precision") {
        // Test that float wrapping handles precision correctly
        float wrapped = RangeHandler::apply_flags(1.0f + 1e-6f, 0.0f, 1.0f, Flags::WRAP);
        CHECK(wrapped == doctest::Approx(1e-6f));
    }
}

TEST_CASE("TypeHandler complete functionality") {
    SUBCASE("Type validation for all types") {
        // Float types
        CHECK(TypeHandler::validate(ParamType::ratio, ParamValue(0.5f)));
        CHECK_FALSE(TypeHandler::validate(ParamType::ratio, ParamValue(NAN)));
        CHECK_FALSE(TypeHandler::validate(ParamType::ratio, ParamValue(INFINITY)));

        // Integer types
        CHECK(TypeHandler::validate(ParamType::count, ParamValue(42)));
        CHECK_FALSE(TypeHandler::validate(ParamType::count, ParamValue(SentinelHandler::get_sentinel<int>())));

        // Boolean type
        CHECK(TypeHandler::validate(ParamType::switch_type, ParamValue(true)));
        CHECK(TypeHandler::validate(ParamType::switch_type, ParamValue(false)));
    }

    SUBCASE("Sentinel value generation") {
        // Float types
        CHECK(TypeHandler::get_sentinel_for_type(ParamType::ratio).as_float() == 
              SentinelHandler::get_sentinel<float>());

        // Integer types
        CHECK(TypeHandler::get_sentinel_for_type(ParamType::count).as_int() == 
              SentinelHandler::get_sentinel<int>());

        // Boolean type
        CHECK(TypeHandler::get_sentinel_for_type(ParamType::switch_type).as_bool() == 
              SentinelHandler::get_sentinel<bool>());
    }

    SUBCASE("Type conversion complete matrix") {
        const ParamType all_types[] = {
            ParamType::ratio, ParamType::signed_ratio,
            ParamType::angle, ParamType::signed_angle,
            ParamType::range, ParamType::count,
            ParamType::select, ParamType::switch_type
        };

        for (auto from : all_types) {
            for (auto to : all_types) {
                // Just test the conversion matrix
                bool result = TypeHandler::can_convert(from, to);
                // Print types if test fails
                INFO("From: ", TypeHandler::get_name(from), 
                     " To: ", TypeHandler::get_name(to));
                CHECK(result == (TypeHandler::can_convert(to, from)));  // Should be symmetric
            }
        }
    }
}

TEST_CASE("FlagHandler") {
    SUBCASE("Flag validation") {
        // Basic flag validation
        CHECK(FlagHandler::validate_flags(Flags::NONE, ParamType::ratio));
        CHECK(FlagHandler::validate_flags(Flags::CLAMP, ParamType::ratio));
        CHECK(FlagHandler::validate_flags(Flags::WRAP, ParamType::ratio));
        
        // Conflicting flags
        CHECK_FALSE(FlagHandler::validate_flags(Flags::CLAMP | Flags::WRAP, ParamType::ratio));
    }

    SUBCASE("Type-specific flag rules") {
        CHECK(FlagHandler::validate_flags(Flags::CLAMP, ParamType::signed_ratio));
        CHECK(FlagHandler::validate_flags(Flags::WRAP, ParamType::angle));

        // Integer types don't support SLEW
        CHECK(FlagHandler::validate_flags(Flags::CLAMP, ParamType::count));
        CHECK(FlagHandler::validate_flags(Flags::WRAP, ParamType::select));
        
        // Non-numeric types
        CHECK(FlagHandler::validate_flags(Flags::NONE, ParamType::switch_type));
    }

    SUBCASE("Flag conflicts") {
        CHECK_FALSE(FlagHandler::has_conflicts(Flags::NONE));
        CHECK_FALSE(FlagHandler::has_conflicts(Flags::CLAMP));
        CHECK_FALSE(FlagHandler::has_conflicts(Flags::WRAP));
        CHECK(FlagHandler::has_conflicts(Flags::CLAMP | Flags::WRAP));
    }

    SUBCASE("Flag application order") {
        // CLAMP takes precedence over WRAP
        ParamFlags flags = Flags::CLAMP | Flags::WRAP;
        CHECK(FlagHandler::apply_flag_rules(flags) == Flags::CLAMP);
        
        // SLEW can combine with other flags
        flags = Flags::CLAMP | Flags::SLEW;
        CHECK(FlagHandler::apply_flag_rules(flags) == (Flags::CLAMP | Flags::SLEW));
    }

    SUBCASE("FlagHandler type-specific rules") {
        SUBCASE("Switch type rejects all flags") {
            CHECK_FALSE(FlagHandler::validate_flags(Flags::CLAMP, ParamType::switch_type));
            CHECK_FALSE(FlagHandler::validate_flags(Flags::WRAP, ParamType::switch_type));
            CHECK(FlagHandler::validate_flags(Flags::NONE, ParamType::switch_type));
        }

        SUBCASE("Select type allows CLAMP/WRAP but not SLEW") {
            CHECK(FlagHandler::validate_flags(Flags::CLAMP, ParamType::select));
            CHECK(FlagHandler::validate_flags(Flags::WRAP, ParamType::select));
            CHECK_FALSE(FlagHandler::validate_flags(Flags::SLEW, ParamType::select));
        }
    }
}