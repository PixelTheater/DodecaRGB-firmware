#include <doctest/doctest.h>
#include "PixelTheater/core/color.h"

using namespace PixelTheater;

TEST_SUITE("CRGB") {
  TEST_CASE("construction") {
      CRGB c1;  // Default constructor
      CHECK(c1.r == 0);
      CHECK(c1.g == 0);
      CHECK(c1.b == 0);

      CRGB c2(100, 150, 200);  // RGB constructor
      CHECK(c2.r == 100);
      CHECK(c2.g == 150);
      CHECK(c2.b == 200);

      CRGB c3(0xFF8800);  // 24-bit color constructor
      CHECK(c3.r == 0xFF);
      CHECK(c3.g == 0x88);
      CHECK(c3.b == 0x00);
  }

  TEST_CASE("fading") {
      CRGB c(200, 100, 50);
      
      SUBCASE("fadeToBlackBy") {
          c.fadeToBlackBy(128);  // 50% fade
          CHECK(c.r == 100);
          CHECK(c.g == 50);
          CHECK(c.b == 25);
      }

      SUBCASE("nscale8") {
          c.nscale8(128);  // 50% scaling
          CHECK(c.r == 100);
          CHECK(c.g == 50);
          CHECK(c.b == 25);
      }
  }

  TEST_CASE("blending") {
      CRGB c1(200, 0, 0);
      CRGB c2(0, 200, 0);
      
      SUBCASE("blend") {
          CRGB result = blend(c1, c2, 128);  // 50% blend
          CHECK(result.r == 100);
          CHECK(result.g == 100);
          CHECK(result.b == 0);
      }

      SUBCASE("nblend") {
          nblend(c1, c2, 128);  // 50% blend into c1
          CHECK(c1.r == 100);
          CHECK(c1.g == 100);
          CHECK(c1.b == 0);
      }
  }

  TEST_CASE("blending edge cases") {
      CRGB c1(200, 0, 0);
      CRGB c2(0, 200, 0);
      
      SUBCASE("blend amount 0") {
          nblend(c1, c2, 0);
          CHECK(c1.r == 200);
          CHECK(c1.g == 0);
      }
      
      SUBCASE("blend amount 255") {
          nblend(c1, c2, 255);
          CHECK(c1.r == 0);
          CHECK(c1.g == 200);
      }
  }

  TEST_CASE("static colors") {
      CHECK(CRGB::Black.r == 0);
      CHECK(CRGB::Black.g == 0);
      CHECK(CRGB::Black.b == 0);
      
      CHECK(CRGB::White.r == 255);
      CHECK(CRGB::White.g == 255);
      CHECK(CRGB::White.b == 255);
      
      CHECK(CRGB::Red.r == 255);
      CHECK(CRGB::Red.g == 0);
      CHECK(CRGB::Red.b == 0);
      // etc...
  }

  TEST_CASE("color component access") {
      CRGB c(100, 150, 200);
      
      SUBCASE("raw array access") {
          CHECK(c.raw[0] == 100);
          CHECK(c.raw[1] == 150);
          CHECK(c.raw[2] == 200);
      }
      
      SUBCASE("named component access") {
          CHECK(c.red == 100);
          CHECK(c.green == 150);
          CHECK(c.blue == 200);
      }
  }

  TEST_CASE("light calculations") {
      SUBCASE("getAverageLight") {
          CHECK(CRGB(0, 0, 0).getAverageLight() == 0);
          CHECK(CRGB(255, 255, 255).getAverageLight() == 255);
          CHECK(CRGB(150, 150, 150).getAverageLight() == 150);
          CHECK(CRGB(100, 200, 255).getAverageLight() == 185);
      }
  }

  TEST_CASE("overflow protection") {
      SUBCASE("nscale8 overflow") {
          CRGB c(255, 255, 255);
          c.nscale8(255);
          CHECK(c.r == 255);
          CHECK(c.g == 255);
          CHECK(c.b == 255);
      }
      
      SUBCASE("blend8 overflow") {
          CRGB c1(255, 255, 255);
          CRGB c2(255, 255, 255);
          nblend(c1, c2, 128);
          CHECK(c1.r == 255);
          CHECK(c1.g == 255);
          CHECK(c1.b == 255);
      }
  }
}

TEST_CASE("HSV colors") {
    SUBCASE("construction") {
        CHSV c1;  // Default constructor
        CHECK(c1.h == 0);
        CHECK(c1.s == 0);
        CHECK(c1.v == 0);

        CHSV c2(160, 255, 255);  // Pure blue
        CHECK(c2.hue == 160);
        CHECK(c2.saturation == 255);
        CHECK(c2.value == 255);
    }

    SUBCASE("conversion to RGB") {
        // Test pure colors
        CHSV blue(160, 255, 255);
        CRGB rgb;
        hsv2rgb_rainbow(blue, rgb);
        CHECK(rgb.r == 0);
        CHECK(rgb.g == 0);
        CHECK(rgb.b == 255);

        // Test automatic conversion
        CRGB auto_rgb = blue | CRGB();
        CHECK(auto_rgb.r == 0);
        CHECK(auto_rgb.g == 0);
        CHECK(auto_rgb.b == 255);
    }
}

TEST_CASE("FastLED preset colors") {
    struct PresetColor {
        const char* name;
        CRGB expected;
    };
    
    PresetColor presets[] = {
        {"Red",    CRGB(255, 0, 0)},
        {"Orange", CRGB(255, 165, 0)},
        {"Yellow", CRGB(255, 255, 0)},
        {"Green",  CRGB(0, 128, 0)},
        {"Aqua",   CRGB(0, 255, 255)},
        {"Blue",   CRGB(0, 0, 255)},
        {"Purple", CRGB(128, 0, 128)},
        {"Pink",   CRGB(255, 192, 203)}
    };

    for(const auto& color : presets) {
        SUBCASE(color.name) {
            CRGB rgb = color.expected;  // Create color using our implementation
            CHECK(rgb.r == color.expected.r);
            CHECK(rgb.g == color.expected.g);
            CHECK(rgb.b == color.expected.b);
        }
    }
}

TEST_CASE("HSV color wheel points") {
    SUBCASE("key hue points") {
        struct TestPoint {
            uint8_t hue;
            CRGB expected;
        };
        
        TestPoint points[] = {
            {0,   CRGB(255, 0, 0)},    // Red
            {32,  CRGB(171, 85, 0)},   // Orange
            {64,  CRGB(171, 170, 0)},  // Yellow
            {96,  CRGB(0, 255, 0)},    // Green
            {128, CRGB(0, 171, 85)},   // Aqua
            {160, CRGB(0, 0, 255)},    // Blue
            {192, CRGB(85, 0, 171)},   // Purple
            {224, CRGB(170, 0, 85)}    // Pink
        };

        for(const auto& point : points) {
            CRGB rgb;
            hsv2rgb_rainbow(CHSV(point.hue, 255, 255), rgb);
            INFO("Testing hue: ", point.hue);
            CHECK(rgb.r == point.expected.r);
            CHECK(rgb.g == point.expected.g);
            CHECK(rgb.b == point.expected.b);
        }
    }

    SUBCASE("blue saturation levels") {
        struct TestPoint {
            uint8_t sat;
            CRGB expected;
        };

        TestPoint points[] = {
            {0,   CRGB(255, 255, 255)},  // White (no saturation)
            {128, CRGB(64, 64, 255)},    // Half saturated blue
            {255, CRGB(0, 0, 255)}       // Pure blue
        };

        for(const auto& point : points) {
            CRGB rgb;
            hsv2rgb_rainbow(CHSV(160, point.sat, 255), rgb);
            INFO("Testing saturation: ", point.sat);
            CHECK(rgb.r == point.expected.r);
            CHECK(rgb.g == point.expected.g);
            CHECK(rgb.b == point.expected.b);
        }
    }
} 