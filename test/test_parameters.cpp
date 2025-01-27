#include <doctest/doctest.h>
#include "PixelTheater/parameter.h"
#include "PixelTheater/param_range.h"
#include "PixelTheater/param_types.h"

using namespace PixelTheater;

TEST_SUITE("Parameters") {
    TEST_CASE("Parameter ranges can be validated") {
        SUBCASE("Float parameters respect their ranges") {
            ParamRange<float> ratio_range(0.0f, 1.0f);
            
            CHECK(ratio_range.validate(0.5f) == true);
            CHECK(ratio_range.validate(0.0f) == true);
            CHECK(ratio_range.validate(1.0f) == true);
            CHECK(ratio_range.validate(-0.1f) == false);
            CHECK(ratio_range.validate(1.1f) == false);
        }

        SUBCASE("Integer parameters respect their ranges") {
            ParamRange<int> count_range(0, 100);
            
            CHECK(count_range.validate(50) == true);
            CHECK(count_range.validate(0) == true);
            CHECK(count_range.validate(100) == true);
            CHECK(count_range.validate(-1) == false);
            CHECK(count_range.validate(101) == false);
        }
    }

    TEST_CASE("Parameters can be created with ranges") {
        SUBCASE("Float parameter with range") {
            Parameter<float> speed("speed", -1.0f, 1.0f);
            
            CHECK(speed.name() == "speed");
            CHECK(speed.set(0.5f) == true);
            CHECK(speed.get() == 0.5f);
            CHECK(speed.set(2.0f) == false);  // Outside range
        }

        SUBCASE("Parameters have default values") {
            Parameter<float> brightness("brightness", 0.0f, 1.0f, 0.8f);
            
            CHECK(brightness.get() == 0.8f);
            CHECK(brightness.default_value() == 0.8f);

            brightness.set(0.5f);
            CHECK(brightness.get() == 0.5f);

            brightness.reset();
            CHECK(brightness.get() == 0.8f);
        }

        SUBCASE("Default values must be in range") {
            CHECK_THROWS_AS(
                Parameter<float>("invalid", 0.0f, 1.0f, 2.0f),
                std::invalid_argument
            );
        }
    }

    TEST_SUITE("Parameter Types") {
        TEST_CASE("Parameter types have correct defaults") {
            SUBCASE("Ratio defaults to min (0.0)") {
                Parameter<float> p("test", 0.0f, 1.0f);
                CHECK(p.get() == 0.0f);
            }

            SUBCASE("SignedRatio defaults to 0.0") {
                Parameter<float> p("test", -1.0f, 1.0f);
                CHECK(p.get() == 0.0f);  // -n..n range defaults to 0
            }

            SUBCASE("Angle defaults to min (0.0)") {
                Parameter<float> p("test", 0.0f, Angle::PI);
                CHECK(p.get() == 0.0f);
            }

            SUBCASE("SignedAngle defaults to 0.0") {
                Parameter<float> p("test", -SignedAngle::PI, SignedAngle::PI);
                CHECK(p.get() == 0.0f);  // -n..n range defaults to 0
            }

            SUBCASE("Count defaults to min (0)") {
                Parameter<int> p("test", 0, 100);
                CHECK(p.get() == 0);
            }

            SUBCASE("Range defaults to min value") {
                Parameter<float> p("test", -5.0f, 5.0f);
                CHECK(p.get() == 0.0f);  // -n..n range defaults to 0
            }
        }

        TEST_CASE("Switch parameters behave like booleans") {
            SUBCASE("Basic Switch behavior") {
                Switch sw;
                CHECK(sw.validate(true) == true);
                CHECK(sw.validate(false) == true);
                CHECK(sw.DEFAULT == false);  // Default should be false
            }

            SUBCASE("Switch parameter defaults to false") {
                Parameter<bool> auto_rotate("auto_rotate", false, true);  // No default provided
                CHECK(auto_rotate.get() == false);
            }

            SUBCASE("Switch parameter respects explicit default") {
                Parameter<bool> auto_rotate("auto_rotate", false, true, true);
                CHECK(auto_rotate.get() == true);
            }
        }

        TEST_CASE("Select parameters map names to values") {
            SUBCASE("Sequential values (0,1,2)") {
                Select sel(2);  // Max position = 2
                sel.add_value("none", 0);   // Position 0
                sel.add_value("mild", 1);   // Position 1
                sel.add_value("wild", 2);   // Position 2

                Parameter<int> chaos("chaos", 0, 2, 0);  // Default to first position
                CHECK(chaos.get() == 0);  // Default position

                chaos.set(1);  // Set to "mild"
                CHECK(chaos.get() == 1);
            }

            SUBCASE("Explicit value mapping") {
                Select sel(1);  // -1..1 range
                sel.add_value("clockwise", 1);
                sel.add_value("counter", -1);
                sel.add_value("random", 0);

                Parameter<int> direction("direction", -1, 1, 1);  // Default to clockwise
                CHECK(direction.get() == 1);

                direction.set(-1);  // Set to counter
                CHECK(direction.get() == -1);
            }

            SUBCASE("Out of range values are clamped") {
                Select sel(2);
                sel.add_value("none", 0);
                sel.add_value("mild", 1);
                sel.add_value("wild", 2);

                Parameter<int> chaos("chaos", 0, 2, 0, Clamp);
                chaos.set(3);  // Beyond max
                CHECK(chaos.get() == 2);  // Clamped to max

                chaos.set(-1);  // Below min
                CHECK(chaos.get() == 0);  // Clamped to min
            }
        }

        TEST_CASE("Standard parameter types have correct ranges") {
            SUBCASE("Ratio is 0..1") {
                Ratio r;
                CHECK(r.validate(0.0f) == true);
                CHECK(r.validate(0.5f) == true);
                CHECK(r.validate(1.0f) == true);
                CHECK(r.validate(-0.1f) == false);
                CHECK(r.validate(1.1f) == false);
            }

            SUBCASE("SignedRatio is -1..1") {
                SignedRatio sr;
                CHECK(sr.validate(-1.0f) == true);
                CHECK(sr.validate(0.0f) == true);
                CHECK(sr.validate(1.0f) == true);
                CHECK(sr.validate(-1.1f) == false);
                CHECK(sr.validate(1.1f) == false);
            }

            SUBCASE("Angle is 0..PI") {
                Angle a;
                CHECK(a.validate(0.0f) == true);
                CHECK(a.validate(Angle::PI/2) == true);
                CHECK(a.validate(Angle::PI) == true);
                CHECK(a.validate(-0.1f) == false);
                CHECK(a.validate(Angle::PI + 0.1f) == false);
            }

            SUBCASE("SignedAngle is -PI..PI") {
                SignedAngle sa;
                CHECK(sa.validate(-SignedAngle::PI) == true);
                CHECK(sa.validate(0.0f) == true);
                CHECK(sa.validate(SignedAngle::PI) == true);
                CHECK(sa.validate(-SignedAngle::PI - 0.1f) == false);
                CHECK(sa.validate(SignedAngle::PI + 0.1f) == false);
            }

            SUBCASE("Count is 0..max") {
                Count c(10);
                CHECK(c.validate(0) == true);
                CHECK(c.validate(5) == true);
                CHECK(c.validate(10) == true);
                CHECK(c.validate(-1) == false);
                CHECK(c.validate(11) == false);
            }

            SUBCASE("Range allows custom ranges") {
                Range<float> custom(-5.0f, 5.0f);
                CHECK(custom.validate(-5.0f) == true);
                CHECK(custom.validate(0.0f) == true);
                CHECK(custom.validate(5.0f) == true);
                CHECK(custom.validate(-5.1f) == false);
                CHECK(custom.validate(5.1f) == false);
            }
        }

        TEST_CASE("Parameter flags modify behavior") {
            SUBCASE("Clamp limits values to range") {
                Parameter<float> p("test", 0.0f, 1.0f, 0.5f, Clamp);
                
                p.set(-0.5f);
                CHECK(p.get() == 0.0f);  // Clamped to min
                
                p.set(1.5f);
                CHECK(p.get() == 1.0f);  // Clamped to max
                
                p.set(0.7f);
                CHECK(p.get() == 0.7f);  // Within range
            }

            SUBCASE("Wrap wraps values around range") {
                // TODO: Implement wrap behavior and test
            }

            SUBCASE("Slew smooths transitions") {
                // TODO: Implement after control system
                // Requires time-based parameter updates
            }
        }
    }
} 