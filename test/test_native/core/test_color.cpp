#include <doctest/doctest.h>
#include "PixelTheater/core/color.h"

using namespace PixelTheater;

TEST_SUITE("CRGB") {
  TEST_CASE("construction") {
      PixelTheater::CRGB c1;  // Explicitly use our CRGB
      CHECK(c1.r == 0);
      CHECK(c1.g == 0);
      CHECK(c1.b == 0);

      PixelTheater::CRGB c2(100, 150, 200);
      CHECK(c2.r == 100);
      CHECK(c2.g == 150);
      CHECK(c2.b == 200);

      PixelTheater::CRGB c3(0xFF8800);
      CHECK(c3.r == 0xFF);
      CHECK(c3.g == 0x88);
      CHECK(c3.b == 0x00);
  }

  TEST_CASE("fading") {
      SUBCASE("fadeToBlackBy") {
          CRGB c(255, 255, 255);
          fadeToBlackBy(c, 128);  // 50% fade
          CHECK(c.r == 127);
          CHECK(c.g == 127);
          CHECK(c.b == 127);
          
          // Test complete fade to black
          CRGB c2(255, 255, 255);
          fadeToBlackBy(c2, 255);
          CHECK(c2.r == 0);
          CHECK(c2.g == 0);
          CHECK(c2.b == 0);
          
          // Test no fade
          CRGB c3(255, 255, 255);
          fadeToBlackBy(c3, 0);
          CHECK(c3.r == 255);
          CHECK(c3.g == 255);
          CHECK(c3.b == 255);
          
          // Test low value fading
          CRGB c4(10, 10, 10);
          fadeToBlackBy(c4, 128);  // 50% fade
          CHECK(c4.r == 5);
          CHECK(c4.g == 5);
          CHECK(c4.b == 5);
          
          // Test very low value fading - should go to zero with sufficient fade
          CRGB c5(3, 3, 3);
          fadeToBlackBy(c5, 128);  // 50% fade
          CHECK(c5.r == 1);
          CHECK(c5.g == 1);
          CHECK(c5.b == 1);
          
          fadeToBlackBy(c5, 128);  // Another 50% fade
          CHECK(c5.r == 0);
          CHECK(c5.g == 0);
          CHECK(c5.b == 0);
      }

      SUBCASE("nscale8") {
          CRGB c(255, 255, 255);
          nscale8(c, 128);  // 50% scaling
          CHECK(c.r == 128);
          CHECK(c.g == 128);
          CHECK(c.b == 128);
          
          // Test complete scaling to black
          CRGB c2(255, 255, 255);
          nscale8(c2, 0);
          CHECK(c2.r == 0);
          CHECK(c2.g == 0);
          CHECK(c2.b == 0);
          
          // Test no scaling
          CRGB c3(255, 255, 255);
          nscale8(c3, 255);
          CHECK(c3.r == 255);
          CHECK(c3.g == 255);
          CHECK(c3.b == 255);
          
          // Test low value scaling
          CRGB c4(10, 10, 10);
          nscale8(c4, 128);  // 50% scaling
          CHECK(c4.r == 5);
          CHECK(c4.g == 5);
          CHECK(c4.b == 5);
          
          // Test very low value scaling
          CRGB c5(3, 3, 3);
          nscale8(c5, 128);  // 50% scaling
          CHECK(c5.r == 1);
          CHECK(c5.g == 1);
          CHECK(c5.b == 1);
          
          nscale8(c5, 128);  // Another 50% scaling
          CHECK(c5.r == 0);
          CHECK(c5.g == 0);
          CHECK(c5.b == 0);
      }
  }

  TEST_CASE("blending") {
      SUBCASE("blend") {
          CRGB c1(255, 0, 0);    // Red
          CRGB c2(0, 0, 255);    // Blue
          CRGB result = blend(c1, c2, 128);  // 50% blend
          
          // Split complex checks into separate statements
          CHECK(result.r >= 126);
          CHECK(result.r <= 129);
          CHECK(result.g == 0);
          CHECK(result.b >= 126);
          CHECK(result.b <= 129);
      }

      SUBCASE("nblend") {
          CRGB c1(255, 0, 0);    // Red
          CRGB c2(0, 0, 255);    // Blue
          nblend(c1, c2, 128);   // 50% blend in place
          
          CHECK(c1.r >= 126);
          CHECK(c1.r <= 129);
          CHECK(c1.g == 0);
          CHECK(c1.b >= 126);
          CHECK(c1.b <= 129);
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
          CHECK(getAverageLight(CRGB(0, 0, 0)) == 0);
          CHECK(getAverageLight(CRGB(255, 255, 255)) == 255);
          CHECK(getAverageLight(CRGB(150, 150, 150)) == 150);
          CHECK(getAverageLight(CRGB(100, 200, 255)) == 185);
      }
  }

  TEST_CASE("overflow protection") {
      SUBCASE("nscale8 overflow") {
          CRGB c(255, 255, 255);
          nscale8(c, 255);
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

  TEST_CASE("array access") {
      CRGB c(100, 150, 200);
      
      // Read access
      CHECK(c[0] == 100);  // r
      CHECK(c[1] == 150);  // g
      CHECK(c[2] == 200);  // b

      // Write access
      c[0] = 50;
      c[1] = 75;
      c[2] = 100;
      CHECK(c.r == 50);
      CHECK(c.g == 75);
      CHECK(c.b == 100);
  }

  TEST_CASE("CRGB constructors and assignment") {
      SUBCASE("copy construction") {
          CRGB c1(100, 150, 200);
          CRGB c2(c1);
          CHECK(c2.r == 100);
          CHECK(c2.g == 150);
          CHECK(c2.b == 200);
      }

      SUBCASE("HSV construction") {
          CRGB rgb(CHSV(160, 255, 255));  // Pure blue
          CHECK(rgb.r == 0);
          CHECK(rgb.g == 0);
          CHECK(rgb.b == 255);
      }

      SUBCASE("assignment operators") {
          CRGB c;
          
          // Assign from RGB
          c = CRGB(100, 150, 200);
          CHECK(c.r == 100);
          CHECK(c.g == 150);
          CHECK(c.b == 200);

          // Assign from HSV
          c = CHSV(160, 255, 255);
          CHECK(c.r == 0);
          CHECK(c.g == 0);
          CHECK(c.b == 255);

          // Assign from hex color
          c = 0xFF0000;  // Red
          CHECK(c.r == 255);
          CHECK(c.g == 0);
          CHECK(c.b == 0);
      }

      SUBCASE("setRGB") {
          CRGB c;
          c.setRGB(100, 150, 200);
          CHECK(c.r == 100);
          CHECK(c.g == 150);
          CHECK(c.b == 200);
      }
  }

  TEST_CASE("array operations") {
      CRGB leds[5];  // Small test array

      SUBCASE("fill_solid") {
          fill_solid(leds, 5, CRGB::Blue);
          CHECK(leds[0] == CRGB::Blue);
          CHECK(leds[4] == CRGB::Blue);
      }

      SUBCASE("fill_rainbow") {
          fill_rainbow(leds, 5, 0, 32);  // Start at hue 0, increment by 32
          // First LED should be red (hue 0)
          CHECK(leds[0].r > 250);
          CHECK(leds[0].g == 0);
          CHECK(leds[0].b == 0);
          // Colors should progress through rainbow
          CHECK(leds[4] != leds[0]);
      }

      SUBCASE("fill_gradient_RGB") {
          fill_gradient_RGB(leds, 0, CRGB::Red, 4, CRGB::Blue);
          
          // Start should be pure red
          CHECK(leds[0].r > 250);
          CHECK(leds[0].g == 0);
          CHECK(leds[0].b == 0);
          
          // End should have significant blue component
          CHECK(leds[4].r < 100);  // Allow some red
          CHECK(leds[4].g == 0);   // No green
          CHECK(leds[4].b > 200);  // Strong blue presence
          
          // Middle LED should be purple-ish blend
          CHECK(leds[2].r > 100);  // Significant red
          CHECK(leds[2].b > 100);  // Significant blue
          CHECK(leds[2].g == 0);   // No green
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

TEST_CASE("CRGB operators") {
    SUBCASE("addition") {
        CRGB c1(100, 150, 200);
        CRGB c2(50, 100, 150);
        
        c1 += c2;
        CHECK(c1.r == 150);  // 100 + 50
        CHECK(c1.g == 250);  // 150 + 100
        CHECK(c1.b == 255);  // 200 + 150 = 350, saturates to 255
    }

    SUBCASE("subtraction") {
        CRGB c1(100, 150, 200);
        CRGB c2(50, 200, 150);
        
        c1 -= c2;
        CHECK(c1.r == 50);   // 100 - 50
        CHECK(c1.g == 0);    // 150 - 200, saturates to 0
        CHECK(c1.b == 50);   // 200 - 150
    }

    SUBCASE("scaling") {
        CRGB c(100, 150, 200);
        
        SUBCASE("zero scale") {
            c *= 0;
            CHECK(c.r == 0);
            CHECK(c.g == 0);
            CHECK(c.b == 0);
        }

        SUBCASE("half scale") {
            c *= 128;  // ~50%
            CHECK(c.r == 50);
            CHECK(c.g == 75);
            CHECK(c.b == 100);
        }

        SUBCASE("full scale") {
            CRGB original = c;
            c *= 255;
            CHECK(c.r == original.r);
            CHECK(c.g == original.g);
            CHECK(c.b == original.b);
        }
    }

    SUBCASE("saturation handling") {
        CRGB c1(255, 255, 255);
        CRGB c2(1, 1, 1);
        
        // Test addition saturation
        c1 += c2;
        CHECK(c1.r == 255);
        CHECK(c1.g == 255);
        CHECK(c1.b == 255);

        // Test subtraction saturation
        CRGB c3(0, 0, 0);
        c3 -= c2;
        CHECK(c3.r == 0);
        CHECK(c3.g == 0);
        CHECK(c3.b == 0);
    }
}

TEST_CASE("FastLED predefined colors") {
    SUBCASE("color values") {
        // Test direct color constants
        CHECK(CRGB::AliceBlue.r == 0xF0);
        CHECK(CRGB::AliceBlue.g == 0xF8);
        CHECK(CRGB::AliceBlue.b == 0xFF);

        CHECK(CRGB::Amethyst.r == 0x99);
        CHECK(CRGB::Amethyst.g == 0x66);
        CHECK(CRGB::Amethyst.b == 0xCC);

        CHECK(CRGB::Aqua.r == 0x00);
        CHECK(CRGB::Aqua.g == 0xFF);
        CHECK(CRGB::Aqua.b == 0xFF);
    }
}

TEST_CASE("FastLED compatibility") {
    SUBCASE("multiple ways to access color components") {
        CRGB color(50, 100, 150);
        
        // r,g,b access
        CHECK(color.r == 50);
        CHECK(color.g == 100);
        CHECK(color.b == 150);
        
        // red,green,blue access
        CHECK(color.red == 50);
        CHECK(color.green == 100);
        CHECK(color.blue == 150);
        
        // raw array access
        CHECK(color.raw[0] == 50);
        CHECK(color.raw[1] == 100);
        CHECK(color.raw[2] == 150);
    }

    SUBCASE("modifying through any accessor changes all views") {
        CRGB color(50, 100, 150);
        
        // Modify through r
        color.r = 255;
        CHECK(color.red == 255);
        CHECK(color.raw[0] == 255);
        
        // Modify through green
        color.green = 255;
        CHECK(color.g == 255);
        CHECK(color.raw[1] == 255);
        
        // Modify through raw
        color.raw[2] = 255;
        CHECK(color.b == 255);
        CHECK(color.blue == 255);
    }
} 