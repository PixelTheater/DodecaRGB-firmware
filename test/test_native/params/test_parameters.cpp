#include <doctest/doctest.h>
#include "PixelTheater/parameter.h"
#include "../../helpers/log_capture.h"

using namespace PixelTheater;

TEST_SUITE("Parameters") {
    // Focus on ParamValue and type system
    TEST_CASE("ParamValue type system") {
        SUBCASE("Construction and type assignment") {
            ParamValue float_val(0.5f);
            ParamValue int_val(42);
            ParamValue bool_val(true);
            CHECK(float_val.type() == ParamType::range);
            CHECK(int_val.type() == ParamType::count);
            CHECK(bool_val.type() == ParamType::switch_type);
        }

        SUBCASE("Type-safe access") {
            ParamValue val(0.5f);
            CHECK(val.as_float() == 0.5f);
            CHECK(SentinelHandler::is_sentinel(val.as_int()));
            CHECK(SentinelHandler::is_sentinel(val.as_bool()));
        }
    }

    // Focus on ParamDef type definitions
    TEST_CASE("ParamDef type definitions") {
        SUBCASE("Basic type definitions") {
            // Replace PARAM_RATIO with factory method
            ParamDef ratio_def = ParamDef::create_ratio("test", 0.5f, Flags::NONE, "");
            // Replace PARAM_COUNT with factory method
            ParamDef count_def = ParamDef::create_count("test", 0, 10, 5, Flags::NONE, "");
            // Replace PARAM_SWITCH with factory method
            ParamDef switch_def = ParamDef::create_switch("test", true, "");

            CHECK(ratio_def.type == ParamType::ratio);
            CHECK(count_def.type == ParamType::count);
            CHECK(switch_def.type == ParamType::switch_type);
        }

        SUBCASE("Type-specific ranges") {
            // Replace PARAM_RATIO with factory method
            ParamDef ratio_def = ParamDef::create_ratio("test", 0.5f, Flags::NONE, "");
            CHECK(ratio_def.get_min() == Constants::RATIO_MIN);
            CHECK(ratio_def.get_max() == Constants::RATIO_MAX);
        }
    }

    // Remove validation tests that belong in settings
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

        SUBCASE("Invalid float values use sentinel") {
            ParamValue nan_val(NAN);
            ParamValue inf_val(INFINITY);
            
            CHECK(SentinelHandler::is_sentinel(nan_val.as_float()));
            CHECK(SentinelHandler::is_sentinel(inf_val.as_float()));
        }
    }

    TEST_CASE("ParamDef validation") {
        SUBCASE("Range parameters") {
            // Replace PARAM_RANGE with factory method
            ParamDef def = ParamDef::create_range("test", -1.0f, 1.0f, 0.0f, Flags::NONE, "");
            
            ParamValue valid(0.5f);
            CHECK(def.apply_flags(valid).as_float() == valid.as_float());
            
            ParamValue invalid(1.5f);
            ParamValue result = def.apply_flags(invalid);
            CHECK(SentinelHandler::is_sentinel(result.as_float()));
        }

        SUBCASE("Count parameters") {
            // Replace PARAM_COUNT with factory method
            ParamDef def = ParamDef::create_count("test", 0, 10, 5, Flags::NONE, "");
            
            ParamValue valid(5);
            CHECK(def.apply_flags(valid).as_int() == valid.as_int());
            
            ParamValue invalid(11);
            ParamValue result = def.apply_flags(invalid);
            CHECK(SentinelHandler::is_sentinel(result.as_int()));
        }

        SUBCASE("Switch parameters") {
            // Replace PARAM_SWITCH with factory method
            ParamDef def = ParamDef::create_switch("test", true, "");
            
            ParamValue valid(true);
            CHECK(def.apply_flags(valid).as_bool() == valid.as_bool());
            
            // All bool values are valid
            ParamValue also_valid(false);
            CHECK(def.apply_flags(also_valid).as_bool() == also_valid.as_bool());
        }
    }
}