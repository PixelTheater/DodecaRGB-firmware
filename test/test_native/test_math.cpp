#include <doctest/doctest.h>
#include "PixelTheater/core/math.h"
#include "Core"  // Eigen includes
#include "Dense"

using namespace PixelTheater;

TEST_SUITE("MathProvider") {
    DefaultMathProvider math;

    TEST_CASE("map") {
        SUBCASE("integer mapping") {
            CHECK(math.map(50, 0, 100, 0, 1000) == 500);
            CHECK(math.map(75, 0, 100, 0, 200) == 150);
            CHECK(math.map(0, -100, 100, -1, 1) == 0);
        }
        
        SUBCASE("float mapping") {
            CHECK(math.map(0.5f, 0.0f, 1.0f, 0.0f, 100.0f) == doctest::Approx(50.0f));
            CHECK(math.map(0.25f, 0.0f, 1.0f, -1.0f, 1.0f) == doctest::Approx(-0.5f));
        }
    }

    TEST_CASE("constrain") {
        SUBCASE("integer constraining") {
            CHECK(math.clamp_value(50, 0, 100) == 50);
            CHECK(math.clamp_value(-10, 0, 100) == 0);
            CHECK(math.clamp_value(200, 0, 100) == 100);
        }
        
        SUBCASE("float constraining") {
            CHECK(math.clamp_value(0.5f, 0.0f, 1.0f) == doctest::Approx(0.5f));
            CHECK(math.clamp_value(-0.5f, 0.0f, 1.0f) == doctest::Approx(0.0f));
            CHECK(math.clamp_value(1.5f, 0.0f, 1.0f) == doctest::Approx(1.0f));
        }
    }

    TEST_CASE("min/max") {
        CHECK(math.min(5, 10) == 5);
        CHECK(math.max(5, 10) == 10);
        CHECK(math.min(0.5f, 1.0f) == doctest::Approx(0.5f));
        CHECK(math.max(0.5f, 1.0f) == doctest::Approx(1.0f));
    }

    TEST_CASE("abs") {
        CHECK(math.abs(-5) == 5);
        CHECK(math.abs(5) == 5);
        CHECK(math.abs(-0.5f) == doctest::Approx(0.5f));
        CHECK(math.abs(0.5f) == doctest::Approx(0.5f));
    }

    TEST_CASE("fixed point trig") {
        // Test key points
        CHECK(math.sin8(0) == 128);    // sin(0) = 0 -> 128
        CHECK(math.sin8(64) == 255);   // sin(π/2) = 1 -> 255
        CHECK(math.sin8(128) == 128);  // sin(π) = 0 -> 128
        CHECK(math.sin8(192) == 1);    // sin(3π/2) = -1 -> 1
        
        CHECK(math.cos8(0) == 255);    // cos(0) = 1 -> 255
        CHECK(math.cos8(64) == 128);   // cos(π/2) = 0 -> 128
        CHECK(math.cos8(128) == 1);    // cos(π) = -1 -> 1
        CHECK(math.cos8(192) == 128);  // cos(3π/2) = 0 -> 128
    }

    TEST_CASE("edge cases") {
        SUBCASE("map with zero range") {
            // Should handle division by zero gracefully
            CHECK(math.map(50, 100, 100, 0, 100) == 0);  // or some defined behavior
        }

        SUBCASE("map with reversed ranges") {
            CHECK(math.map(75, 100, 0, 0, 200) == 50);  // Should work with reversed input range
            CHECK(math.map(50, 0, 100, 200, 0) == 100); // Should work with reversed output range
        }
    }

    TEST_CASE("saturating arithmetic") {
        SUBCASE("qadd8") {
            CHECK(math.qadd8(100, 100) == 200);     // Normal addition
            CHECK(math.qadd8(200, 100) == 255);     // Saturates at 255
            CHECK(math.qadd8(255, 1) == 255);       // Already at max
            CHECK(math.qadd8(0, 255) == 255);       // Full range
            CHECK(math.qadd8(0, 0) == 0);           // Zero case
        }

        SUBCASE("qsub8") {
            CHECK(math.qsub8(100, 50) == 50);       // Normal subtraction
            CHECK(math.qsub8(100, 200) == 0);       // Saturates at 0
            CHECK(math.qsub8(0, 1) == 0);           // Already at min
            CHECK(math.qsub8(255, 255) == 0);       // Full range
            CHECK(math.qsub8(0, 0) == 0);           // Zero case
        }
    }

    TEST_CASE("random number generation") {
        SUBCASE("deterministic sequence") {
            math.setRandomSeed(42);
            auto first = math.random(100);
            auto second = math.random(100);
            
            math.setRandomSeed(42);  // Reset
            CHECK(math.random(100) == first);
            CHECK(math.random(100) == second);
        }

        SUBCASE("range validation") {
            for(int i = 0; i < 1000; i++) {
                auto val = math.random(0, 100);
                CHECK(val >= 0);
                CHECK(val < 100);
            }
        }

        SUBCASE("edge cases") {
            CHECK(math.random(1) == 0);
            CHECK(math.random(0, 1) == 0);
            CHECK(math.random(100, 100) == 100);  // Equal min/max
        }
    }

    TEST_CASE("eigen compatibility test") {
        SUBCASE("Vector operations") {
            Eigen::Vector3d v1(1.0, 0.0, 0.0);
            Eigen::Vector3d v2(0.0, 1.0, 0.0);
            
            auto cross = v1.cross(v2);
            CHECK(cross.isApprox(Eigen::Vector3d(0.0, 0.0, 1.0)));
            CHECK(v1.dot(v2) == doctest::Approx(0.0));
        }

        SUBCASE("Normalization") {
            Eigen::Vector3d v(2.0, 0.0, 0.0);
            v.normalize();
            CHECK(v.norm() == doctest::Approx(1.0));
        }
    }

    TEST_CASE("random number generators are independent") {
        DefaultMathProvider math;
        
        SUBCASE("different seeds give different sequences") {
            // Set both seeds to same value
            math.random16_set_seed(1337);
            math.setRandomSeed(1337);
            
            // Get first values from each
            uint16_t fastled_first = math.random16();
            int32_t arduino_first = math.random(65536);  // Same range as random16
            
            // Values should be different due to different algorithms
            CHECK(fastled_first != arduino_first);
            
            // Each should maintain its own sequence
            math.random16_set_seed(1337);
            CHECK(math.random16() == fastled_first);
            
            math.setRandomSeed(1337);
            CHECK(math.random(65536) == arduino_first);
        }
        
        SUBCASE("sequences don't affect each other") {
            math.random16_set_seed(42);
            math.setRandomSeed(1337);
            
            auto fastled_seq = std::vector<uint16_t>();
            auto arduino_seq = std::vector<int32_t>();
            
            // Generate some values
            for(int i = 0; i < 5; i++) {
                fastled_seq.push_back(math.random16());
                arduino_seq.push_back(math.random(65536));
            }
            
            // Reset seeds
            math.random16_set_seed(42);
            math.setRandomSeed(1337);
            
            // Verify sequences match when called in different order
            for(int i = 0; i < 5; i++) {
                CHECK(math.random(65536) == arduino_seq[i]);
            }
            for(int i = 0; i < 5; i++) {
                CHECK(math.random16() == fastled_seq[i]);
            }
        }
    }
} 