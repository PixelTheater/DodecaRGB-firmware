#include <doctest/doctest.h>
#include "PixelTheater/parameter.h"
#include "fixtures/parameter_test_params.h"
#include "helpers/log_capture.h"

using namespace PixelTheater;

TEST_SUITE("Parameters") {
    TEST_CASE("Parameter definitions are validated") {
        SUBCASE("Basic types have correct defaults") {
            const auto& bool_def = TEST_PARAMS[0];  // test_bool
            CHECK(bool_def.type == ParamType::switch_type);
            CHECK(bool_def.bool_default == true);
            
            const auto& int_def = TEST_PARAMS[1];  // test_int
            CHECK(int_def.type == ParamType::count);
            CHECK(int_def.range_min_i == 0);
            CHECK(int_def.range_max_i == 100);
            CHECK(int_def.default_val_i == 50);
        }

        SUBCASE("Range validation") {
            const auto& range_def = TEST_PARAMS[4];  // test_range_float
            CHECK(range_def.type == ParamType::range);
            CHECK(range_def.range_min == -1.0f);
            CHECK(range_def.range_max == 1.0f);
        }

        SUBCASE("Flag combinations") {
            const auto& clamp_def = TEST_PARAMS[5];  // test_clamp
            CHECK(Flags::has_flag(clamp_def.flags, Flags::CLAMP));
            CHECK(!Flags::has_flag(clamp_def.flags, Flags::WRAP));
        }
    }

    TEST_CASE("Parameter macros create valid definitions") {
        SUBCASE("Switch parameters") {
            constexpr ParamDef def = PARAM_SWITCH("test", true, "Test switch");
            CHECK(def.type == ParamType::switch_type);
            CHECK(def.bool_default == true);
            CHECK(std::string(def.description) == "Test switch");
        }

        SUBCASE("Range parameters") {
            constexpr ParamDef float_def = PARAM_RANGE("test", -1.0f, 1.0f, 0.0f, 
                                                      Flags::CLAMP, "Test range");
            CHECK(float_def.type == ParamType::range);
            CHECK(float_def.range_min == -1.0f);
            CHECK(float_def.range_max == 1.0f);
            CHECK(float_def.default_val == 0.0f);
            CHECK(Flags::has_flag(float_def.flags, Flags::CLAMP));
        }

        SUBCASE("Select parameters") {
            static constexpr const char* const options[] = {
                "one", "two", "three", nullptr
            };
            constexpr ParamDef def = PARAM_SELECT("test", 1, options, "Test select");
            CHECK(def.type == ParamType::select);
            CHECK(def.default_idx == 1);
            CHECK(std::string(def.options[0]) == "one");
            CHECK(def.options[3] == nullptr);
        }
    }

    TEST_CASE("Flag operations") {
        SUBCASE("Flag combinations") {
            ParamFlags flags = Flags::CLAMP | Flags::SLEW;
            CHECK(Flags::has_flag(flags, Flags::CLAMP));
            CHECK(Flags::has_flag(flags, Flags::SLEW));
            CHECK(!Flags::has_flag(flags, Flags::WRAP));
        }

        SUBCASE("Flag names") {
            CHECK(std::string(Flags::get_name(Flags::CLAMP)) == "clamp");
            CHECK(std::string(Flags::get_name(Flags::WRAP)) == "wrap");
            CHECK(std::string(Flags::get_name(Flags::SLEW)) == "slew");
            CHECK(std::string(Flags::get_name(Flags::NONE)) == "");
        }
    }

    TEST_CASE("Parameter validation") {
        SUBCASE("Ratio parameters") {
            ParamDef def = PARAM_RATIO("test", 0.5f, Flags::NONE, "");
            
            ParamValue valid(0.5f);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(1.5f);
            ParamValue result = def.apply_flags(invalid);
            CHECK(SentinelHandler::is_sentinel(result.as_float()));
        }

        SUBCASE("Signed ratio parameters") {
            ParamDef def = PARAM_SIGNED_RATIO("test", 0.0f, Flags::NONE, "");
            
            ParamValue valid(0.0f);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(-1.1f);
            ParamValue result = def.apply_flags(invalid);
            CHECK(SentinelHandler::is_sentinel(result.as_float()));
        }

        SUBCASE("Angle parameters") {
            ParamDef def = PARAM_ANGLE("test", Constants::PT_HALF_PI, Flags::NONE, "");
            
            ParamValue valid(Constants::PT_HALF_PI);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(Constants::PT_TWO_PI);
            ParamValue result = def.apply_flags(invalid);
            CHECK(SentinelHandler::is_sentinel(result.as_float()));
        }

        SUBCASE("Signed angle parameters") {
            ParamDef def = PARAM_SIGNED_ANGLE("test", 0.0f, Flags::NONE, "");
            
            ParamValue valid(0.0f);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(-Constants::PT_TWO_PI);
            ParamValue result = def.apply_flags(invalid);
            CHECK(SentinelHandler::is_sentinel(result.as_float()));
        }

        SUBCASE("Invalid values generate warning messages") {
            Test::LogCapture log;
            
            ParamDef def = PARAM_RATIO("test", 1.5f, Flags::NONE, "");  // Invalid default
            CHECK(log.contains_warning());
            
            log.clear();
            ParamValue val(1.5f);
            def.apply_flags(val);  // Out of range
            CHECK(log.contains_warning());
        }
    }

    TEST_CASE("Parameter flags affect validation behavior") {
        SUBCASE("CLAMP flag") {
            ParamDef def = PARAM_RATIO("test", 0.5f, Flags::CLAMP, "");
            
            ParamValue param(1.5f);
            // With CLAMP, should clamp to 1.0
            CHECK(def.apply_flags(param).as_float() == 1.0f);
            
            param = ParamValue(0.5f);
            CHECK(def.apply_flags(param).as_float() == 0.5f);
        }

        SUBCASE("WRAP flag") {
            ParamDef def = PARAM_ANGLE("test", 0.0f, Flags::WRAP, "");
            
            ParamValue param(Constants::PT_TWO_PI);
            // With WRAP, should wrap to 0.0
            CHECK(def.apply_flags(param).as_float() == 0.0f);
            
            param = ParamValue(Constants::PT_HALF_PI);
            CHECK(def.apply_flags(param).as_float() == Constants::PT_HALF_PI);
        }

        SUBCASE("CLAMP with sentinel values") {
            ParamDef def = PARAM_RATIO("test", 0.5f, Flags::CLAMP, "");
            
            // Test NaN/Inf handling with CLAMP
            ParamValue nan_val(NAN);
            ParamValue inf_val(INFINITY);
            
            CHECK(SentinelHandler::is_sentinel(def.apply_flags(nan_val).as_float()));
            CHECK(SentinelHandler::is_sentinel(def.apply_flags(inf_val).as_float()));
            
            // Test that clamping works after sentinel
            ParamValue valid(0.5f);
            CHECK(def.apply_flags(valid).as_float() == 0.5f);
        }

        SUBCASE("WRAP with sentinel values") {
            ParamDef def = PARAM_ANGLE("test", 0.0f, Flags::WRAP, "");
            
            // Test NaN/Inf handling with WRAP
            ParamValue nan_val(NAN);
            ParamValue inf_val(INFINITY);
            
            CHECK(SentinelHandler::is_sentinel(def.apply_flags(nan_val).as_float()));
            CHECK(SentinelHandler::is_sentinel(def.apply_flags(inf_val).as_float()));
            
            // Test wrapping with large values
            ParamValue large_val(10.0f * Constants::PT_TWO_PI);  // 10 full rotations
            float wrapped = def.apply_flags(large_val).as_float();
            CHECK(wrapped >= 0.0f);
            CHECK(wrapped < Constants::PT_TWO_PI);
        }

        SUBCASE("CLAMP and WRAP interaction") {
            ParamDef def = PARAM_RATIO("test", 0.5f, Flags::CLAMP | Flags::WRAP, "");
            
            // Test values above range
            ParamValue over(1.5f);
            CHECK(def.apply_flags(over).as_float() == 1.0f);  // Should clamp to max
            
            // Test values below range
            ParamValue under(-0.5f);
            CHECK(def.apply_flags(under).as_float() == 0.0f);  // Should clamp to min
            
            // Test with angle parameter
            ParamDef angle_def = PARAM_ANGLE("angle", 0.0f, Flags::CLAMP | Flags::WRAP, "");
            
            // Large value that would wrap to middle of range if wrapping
            ParamValue large_angle(10.0f * Constants::PT_TWO_PI + Constants::PT_HALF_PI);
            CHECK(angle_def.apply_flags(large_angle).as_float() == Constants::PT_PI);  // Should clamp to max
        }
    }

    TEST_CASE("Parameter ranges use constants") {
        SUBCASE("Ratio parameters") {
            ParamDef def = PARAM_RATIO("test_ratio", 0.5f, Flags::NONE, "Test ratio");
            CHECK(def.get_min() == Constants::RATIO_MIN);
            CHECK(def.get_max() == Constants::RATIO_MAX);
        }

        SUBCASE("Angle parameters") {
            ParamDef def = PARAM_ANGLE("test", Constants::PT_HALF_PI, Flags::NONE, "");
            CHECK(def.get_min() == Constants::ANGLE_MIN);
            CHECK(def.get_max() == Constants::ANGLE_MAX);
        }

        SUBCASE("Unsupported types return sentinel values") {
            ParamDef def("test", ParamType::palette, "default", "");  // Palette type has no min/max
            
            CHECK(SentinelHandler::is_sentinel(def.get_min()));
            CHECK(SentinelHandler::is_sentinel(def.get_max()));
            CHECK(SentinelHandler::is_sentinel(def.get_default()));
        }
    }

    TEST_CASE("Parameter validation returns sentinel") {
        SUBCASE("Invalid values return sentinel without flags") {
            ParamDef def = PARAM_RATIO("test", 0.5f, Flags::NONE, "");
            ParamValue param(1.5f);
            ParamValue result = def.apply_flags(param);
            CHECK(SentinelHandler::is_sentinel(result.as_float()));
        }
    }
}

TEST_SUITE("Parameter System") {
    TEST_CASE("ParamValue type safety") {
        SUBCASE("Construction assigns correct type") {
            ParamValue float_val(0.5f);
            ParamValue int_val(42);
            ParamValue bool_val(true);

            CHECK(float_val.type() == ParamType::range);
            CHECK(int_val.type() == ParamType::count);
            CHECK(bool_val.type() == ParamType::switch_type);
        }

        SUBCASE("Type-safe access returns sentinels for invalid types") {
            ParamValue float_val(0.5f);
            ParamValue int_val(42);
            ParamValue bool_val(true);

            // Float value
            CHECK(float_val.as_float() == doctest::Approx(0.5f));  // Valid
            CHECK(float_val.as_int() == SentinelHandler::get_sentinel<int>());  // Invalid
            CHECK(float_val.as_bool() == SentinelHandler::get_sentinel<bool>());  // Invalid

            // Int value
            CHECK(int_val.as_float() == SentinelHandler::get_sentinel<float>());  // Invalid
            CHECK(int_val.as_int() == 42);  // Valid
            CHECK(int_val.as_bool() == SentinelHandler::get_sentinel<bool>());  // Invalid

            // Bool value
            CHECK(bool_val.as_float() == SentinelHandler::get_sentinel<float>());  // Invalid
            CHECK(bool_val.as_int() == SentinelHandler::get_sentinel<int>());  // Invalid
            CHECK(bool_val.as_bool() == true);  // Valid
        }

        SUBCASE("Type conversion compatibility") {
            ParamValue ratio(0.5f);
            CHECK(ratio.can_convert_to(ParamType::ratio));
            CHECK(ratio.can_convert_to(ParamType::signed_ratio));
            CHECK_FALSE(ratio.can_convert_to(ParamType::switch_type));

            ParamValue count(42);
            CHECK(count.can_convert_to(ParamType::count));
            CHECK(count.can_convert_to(ParamType::select));
            CHECK_FALSE(count.can_convert_to(ParamType::ratio));
        }

        SUBCASE("Invalid float conversion returns sentinel") {
            ParamValue int_val(42);  // COUNT type
            ParamValue bool_val(true);  // SWITCH type
            
            CHECK(int_val.as_float() == SentinelHandler::get_sentinel<float>());
            CHECK(bool_val.as_float() == SentinelHandler::get_sentinel<float>());
            CHECK(SentinelHandler::is_sentinel(int_val.as_float()));
        }

        SUBCASE("Invalid int conversion returns sentinel") {
            ParamValue float_val(0.5f);  // RANGE type
            ParamValue bool_val(true);   // SWITCH type
            
            CHECK(float_val.as_int() == SentinelHandler::get_sentinel<int>());
            CHECK(bool_val.as_int() == SentinelHandler::get_sentinel<int>());
            CHECK(SentinelHandler::is_sentinel(float_val.as_int()));
        }

        SUBCASE("Invalid string conversion returns sentinel") {
            ParamValue float_val(0.5f);
            ParamValue int_val(42);
            ParamValue bool_val(true);
            ParamValue str_val("test");  // PALETTE type
            
            CHECK(str_val.as_string() != nullptr);  // Valid
            CHECK(float_val.as_string() == nullptr);  // Invalid
            CHECK(int_val.as_string() == nullptr);  // Invalid
            CHECK(bool_val.as_string() == nullptr);  // Invalid
        }

        SUBCASE("Invalid type conversion returns sentinel") {
            ParamDef def = PARAM_RATIO("test", 0.5f, Flags::NONE, "");
            
            // Try to apply float parameter to bool type
            ParamValue bool_val(true);
            ParamValue result = def.apply_flags(bool_val);
            
            CHECK(SentinelHandler::is_sentinel(result.as_float()));
        }

        SUBCASE("Invalid float values use sentinel") {
            ParamValue nan_val(NAN);
            ParamValue inf_val(INFINITY);
            
            CHECK(SentinelHandler::is_sentinel(nan_val.as_float()));
            CHECK(SentinelHandler::is_sentinel(inf_val.as_float()));
        }
    }

    TEST_CASE("ParamDef validation") {
        SUBCASE("Range parameters") {
            ParamDef def = PARAM_RANGE("test", -1.0f, 1.0f, 0.0f, Flags::NONE, "");
            
            ParamValue valid(0.5f);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(1.5f);
            ParamValue result = def.apply_flags(invalid);
            CHECK(SentinelHandler::is_sentinel(result.as_float()));
        }

        SUBCASE("Count parameters") {
            ParamDef def = PARAM_COUNT("test", 0, 10, 5, Flags::NONE, "");
            
            ParamValue valid(5);
            CHECK(def.apply_flags(valid).as_int() == valid.as_int());
            
            ParamValue invalid(11);
            ParamValue result = def.apply_flags(invalid);
            CHECK(SentinelHandler::is_sentinel(result.as_int()));
        }

        SUBCASE("Switch parameters") {
            ParamDef def = PARAM_SWITCH("test", true, "");
            
            ParamValue valid(true);
            CHECK(def.apply_flags(valid).as_bool() == valid.as_bool());
            
            // All bool values are valid
            ParamValue also_valid(false);
            CHECK(def.apply_flags(also_valid).as_bool() == also_valid.as_bool());
        }
    }

    TEST_CASE("Parameter values can be set") {
        ParamValue param(0.0f);
        param = 0.5f;
        CHECK(param.as_float() == 0.5f);
    }
}