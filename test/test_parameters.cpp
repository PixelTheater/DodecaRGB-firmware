#include <doctest/doctest.h>
#include "PixelTheater/parameter.h"
#include "fixtures/parameter_test_params.h"

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
            // Check that value is unchanged when no flags
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(1.5f);
            // Should throw for out of range without CLAMP/WRAP
            CHECK_THROWS_AS(def.apply_flags(invalid), std::out_of_range);
        }

        SUBCASE("Signed ratio parameters") {
            ParamDef def = PARAM_SIGNED_RATIO("test", 0.0f, Flags::NONE, "");
            
            ParamValue valid(0.0f);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(-1.1f);
            CHECK_THROWS_AS(def.apply_flags(invalid), std::out_of_range);
        }

        SUBCASE("Angle parameters") {
            ParamDef def = PARAM_ANGLE("test", Constants::HALF_PI, Flags::NONE, "");
            
            ParamValue valid(Constants::HALF_PI);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(Constants::TWO_PI);
            CHECK_THROWS_AS(def.apply_flags(invalid), std::out_of_range);
        }

        SUBCASE("Signed angle parameters") {
            ParamDef def = PARAM_SIGNED_ANGLE("test", 0.0f, Flags::NONE, "");
            
            ParamValue valid(0.0f);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(-Constants::TWO_PI);
            CHECK_THROWS_AS(def.apply_flags(invalid), std::out_of_range);
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
            
            ParamValue param(Constants::TWO_PI);
            // With WRAP, should wrap to 0.0
            CHECK(def.apply_flags(param).as_float() == 0.0f);
            
            param = ParamValue(Constants::HALF_PI);
            CHECK(def.apply_flags(param).as_float() == Constants::HALF_PI);
        }
    }

    TEST_CASE("Parameter ranges use constants") {
        SUBCASE("Ratio parameters") {
            ParamDef def = PARAM_RATIO("test_ratio", 0.5f, Flags::NONE, "Test ratio");
            CHECK(def.get_min() == Constants::RATIO_MIN);
            CHECK(def.get_max() == Constants::RATIO_MAX);
        }

        SUBCASE("Angle parameters") {
            ParamDef def = PARAM_ANGLE("test", Constants::HALF_PI, Flags::NONE, "");
            
            CHECK(def.get_min() == Constants::ANGLE_MIN);
            CHECK(def.get_max() == Constants::ANGLE_MAX);
        }
    }

    TEST_CASE("Parameter validation throws appropriately") {
        SUBCASE("Invalid values throw without flags") {
            ParamDef def = PARAM_RATIO("test", 0.5f, Flags::NONE, "");
            ParamValue param(1.5f);
            CHECK_THROWS_AS(def.apply_flags(param), std::out_of_range);
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

        SUBCASE("Type-safe access") {
            ParamValue val(0.5f);
            CHECK(val.as_float() == doctest::Approx(0.5f));
            CHECK_THROWS_AS(val.as_int(), std::bad_cast);
            CHECK_THROWS_AS(val.as_bool(), std::bad_cast);
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
    }

    TEST_CASE("ParamDef validation") {
        SUBCASE("Range parameters") {
            ParamDef def = PARAM_RANGE("test", -1.0f, 1.0f, 0.0f, Flags::NONE, "");
            
            ParamValue valid(0.5f);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(1.5f);
            CHECK_THROWS_AS(def.apply_flags(invalid), std::out_of_range);
        }

        SUBCASE("Count parameters") {
            ParamDef def = PARAM_COUNT("test", 0, 10, 5, Flags::NONE, "");
            
            ParamValue valid(5);
            CHECK(def.apply_flags(valid).as_int() == valid.as_int());
            
            ParamValue invalid(11);
            CHECK_THROWS_AS(def.apply_flags(invalid), std::out_of_range);
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

    TEST_CASE("Parameter definitions have correct ranges") {
        ParamDef def = PARAM_ANGLE("test", 0.0f, Flags::NONE, "");
        CHECK(def.get_min() == Constants::ANGLE_MIN);
        CHECK(def.get_max() == Constants::ANGLE_MAX);
    }
}