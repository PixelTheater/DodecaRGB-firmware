---
category: Development
generated: false
version: N/A
---

# Using Color in PixelTheater Scenes

PixelTheater provides a flexible and platform-independent way to work with colors in your animation Scenes. This guide focuses on how Scene authors can leverage the color system.

**Core Goal:** Write your color logic once using the `PixelTheater` API, and have it run correctly on Teensy (using FastLED), native C++ simulators, and the Web simulator.

**Main Include:** Simply include the main library header:

```cpp
#include "PixelTheater.h"
```

This brings in all the necessary color types, functions, and constants.

## Color Representation: `CRGB` and `CHSV`

PixelTheater uses color structures designed to be familiar to FastLED users.

* **`PixelTheater::CRGB`**: Represents a 24-bit color with Red, Green, and Blue channels (0-255 each).
  * **Compatibility:** Structurally similar to FastLED's `CRGB`. On Teensy, conversions to/from `::CRGB` are handled efficiently behind the scenes.
  * **Access:** `color.r`, `color.g`, `color.b`, or `color[0]`, `color[1]`, `color[2]`.
  * **Creating:**

```cpp
PixelTheater::CRGB myRed = PixelTheater::CRGB(255, 0, 0);
PixelTheater::CRGB myPurple = PixelTheater::CRGB(0xFF00FF); // Hex code
PixelTheater::CRGB darkBlue = PixelTheater::CRGB(0, 0, 50); // From test_scene.h
```

* **`PixelTheater::CHSV`**: Represents color using Hue (0-255), Saturation (0-255), and Value (Brightness, 0-255).
  * **Compatibility:** Structurally similar to FastLED's `CHSV`.
  * **Access:** `hsv.h`, `hsv.s`, `hsv.v`.
  * **Creating:**

```cpp
PixelTheater::CHSV brightGreen = PixelTheater::CHSV(96, 255, 255); // FastLED Green hue is 96
PixelTheater::CHSV dimOrange = PixelTheater::CHSV(32, 200, 100);
```

* **Conversion:** You can assign or construct `CRGB` from `CHSV` and vice-versa using standard functions or operators.

    ```cpp
    #include "PixelTheater.h" // Includes necessary headers

    // HSV -> CRGB
    PixelTheater::CHSV hsvColor(160, 255, 255); // Blue hue
    PixelTheater::CRGB rgbFromHsv = hsvColor; // Implicit conversion via constructor/assignment

    // Alternative HSV -> CRGB function call
    PixelTheater::CRGB anotherRgb;
    PixelTheater::hsv2rgb_rainbow(hsvColor, anotherRgb); // Explicit function call

    // CRGB -> HSV
    PixelTheater::CRGB rgbColor = PixelTheater::CRGB::Yellow;
    PixelTheater::CHSV hsvFromRgb = PixelTheater::rgb2hsv_approximate(rgbColor);
    // hsvFromRgb.h will be ~42

    // Assigning HSV directly to an LED (which stores CRGB) performs the conversion
    leds[0] = PixelTheater::CHSV(0, 255, 255); // Assign Red using HSV
    ```

## Using Predefined Colors

For convenience, many standard color names are available as `static const PixelTheater::CRGB` constants.

* **Compatibility:** Includes standard HTML/CSS color names, similar to FastLED's predefined colors.
* **Access:** Use `PixelTheater::CRGB::ColorName`.

```cpp
#include "PixelTheater.h"

leds[0] = PixelTheater::CRGB::Red;
leds[1] = PixelTheater::CRGB::Aqua;
leds[2] = PixelTheater::CRGB::Orange;
leds[3] = PixelTheater::CRGB::Black;
```

## Using Palettes

PixelTheater provides a robust way to work with 16-entry color palettes.

* **Type:** `PixelTheater::CRGBPalette16` (an alias for `std::array<PixelTheater::CRGB, 16>`).
* **Predefined Palettes:** The `PixelTheater::Palettes` namespace contains constants for standard FastLED palettes (like `RainbowColors`, `PartyColors`, `OceanColors`, `ForestColors`) and custom project palettes (`basePalette`).
* **Sampling Function:** `PixelTheater::colorFromPalette()` is the primary way to get colors. It maps a 0-255 index onto the 16 palette entries, optionally blending between them.
  * **Compatibility:** Similar in concept to FastLED's `ColorFromPalette` function.

```cpp
#include "PixelTheater.h" // Includes palettes.h and palette_api.h

void MyScene::tick() {
    Scene::tick();
    uint8_t animationIndex = tickCount(); // Simple 0-255 index

    // Example 1: Color boids from Ocean palette (from boids_scene.h)
    PixelTheater::CRGB boidColor = PixelTheater::colorFromPalette(
        PixelTheater::Palettes::OceanColors,
        animationIndex // Use animation index to cycle through palette
    );
    // Assign boidColor to LEDs representing the boid...

    // Example 2: Fill LEDs using Rainbow palette
    const PixelTheater::CRGBPalette16& rainbow = PixelTheater::Palettes::RainbowColors;
    for (size_t i = 0; i < ledCount(); ++i) {
        uint8_t ledIndex = animationIndex + (i * 10); // Offset index per LED
        // Get color with brightness 200, linear blending
        leds[i] = PixelTheater::colorFromPalette(rainbow, ledIndex, 200, PixelTheater::LINEARBLEND);
    }

    // Example 3: Using NOBLEND
    PixelTheater::CRGB colorSnapped = PixelTheater::colorFromPalette(
        PixelTheater::Palettes::PartyColors,
        animationIndex,
        255, // Full brightness
        PixelTheater::NOBLEND // Snap to nearest entry
    );
}
```

* See the [Palette API documentation](Palettes.md) for more details on `colorFromPalette` options and available palettes.

## Modifying and Blending Colors

Several methods and functions allow you to manipulate colors.

* **`CRGB::fadeToBlackBy(uint8_t fade)`**: Reduces the brightness of a `CRGB` color. `fade=255` is black, `fade=0` is no change.
  * **Compatibility:** Identical method signature to FastLED's `CRGB::fadeToBlackBy`.
* **`CRGB::nscale8(uint8_t scale)`**: Scales the brightness of a `CRGB` color. `scale=255` is full brightness, `scale=128` is half, `scale=0` is black.
  * **Compatibility:** Identical method signature to FastLED's `CRGB::nscale8`.
* **`PixelTheater::blend(CRGB c1, CRGB c2, uint8_t amount)`**: Blends color `c1` towards `c2`. `amount=0` gives `c1`, `amount=255` gives `c2`, `amount=128` is a 50% mix.
  * **Compatibility:** Similar in concept and usage to FastLED's `blend` function.

```cpp
#include "PixelTheater.h"

void MyScene::tick() {
    Scene::tick();
    size_t numLeds = ledCount();

    // --- Fading/Scaling ---
    // Fade all LEDs slightly each frame (very common effect)
    for (size_t i = 0; i < numLeds; ++i) {
        leds[i].fadeToBlackBy(10); // Make slightly dimmer
    }

    // Scale brightness of one LED (from test_scene.h)
    if (numLeds > 0) {
        uint8_t brightness = 128; // 50%
        leds[0].nscale8(brightness);
    }

    // --- Blending ---
    // Blend LED 1 towards Blue (from creating_animations.md example)
    if (numLeds > 1) {
        PixelTheater::CRGB targetColor = PixelTheater::CRGB::Blue;
        uint8_t blendAmount = 64; // 25% towards target
        leds[1] = PixelTheater::blend(leds[1], targetColor, blendAmount);
    }

    // Blend between two defined colors (from test_scene.h)
    PixelTheater::CRGB startColor = PixelTheater::CRGB(0, 0, 50);
    PixelTheater::CRGB endColor = PixelTheater::CRGB::White;
    uint8_t blend_amount = 192; // 75% towards endColor
    PixelTheater::CRGB blendedColor = PixelTheater::blend(startColor, endColor, blend_amount);
    // Use blendedColor...
}
```

## Fill Functions

Convenience functions for filling ranges of LEDs. These work on the `LedsProxy` object provided in scenes, or raw `CRGB*` arrays.

* `PixelTheater::fill_solid(leds, color)`
* `PixelTheater::fill_gradient_RGB(leds, startpos, startcolor, endpos, endcolor)`
* `PixelTheater::fill_rainbow(leds, initialhue, deltahue)`

```cpp
#include "PixelTheater.h"

void MyScene::tick() {
    Scene::tick();

    // Fill all LEDs black (from test_scene.h)
    PixelTheater::fill_solid(leds, PixelTheater::CRGB::Black);

    // Fill first 50 LEDs green
    // Need to check bounds if not filling the whole range
    size_t fillCount = std::min((size_t)50, ledCount());
    PixelTheater::fill_solid(leds.slice(0, fillCount), PixelTheater::CRGB::Green);

    // Fill next 50 with a rainbow (if enough LEDs exist)
    if (ledCount() > 100) {
       PixelTheater::fill_rainbow(leds.slice(50, 50), tickCount(), 5); // Start hue, delta hue per LED
    }
}
```

*(Note: `leds.slice(start, count)` returns a temporary object representing the range)*

## Advanced Utilities (ColorUtils)

The `PixelTheater::ColorUtils` namespace contains less frequently used helper functions for color measurement and identity.

* `ColorUtils::colorDistance(c1, c2)`
* `ColorUtils::getClosestColorName(rgb)`
* `ColorUtils::getAnsiColorString(rgb, char)`
* `ColorUtils::get_perceived_brightness(hsv)`
* `ColorUtils::get_contrast_ratio(hsv1, hsv2)`
* `ColorUtils::get_hue_distance(hsv1, hsv2)`

```cpp
#include "PixelTheater.h"
#include <string> // For std::string

void MyScene::config() {
    Scene::config();
    set_name("Color Utils Example");
}

void MyScene::tick() {
    Scene::tick();
    if (ledCount() > 1) {
        PixelTheater::CRGB c1 = leds[0];
        PixelTheater::CRGB c2 = leds[1];

        uint32_t dist = PixelTheater::ColorUtils::colorDistance(c1, c2);
        std::string name = PixelTheater::ColorUtils::getClosestColorName(c1);

        // Log the information (requires Log provider)
        // PixelTheater::Log::printf("LED 0 is closest to %s. Distance to LED 1: %u\n", name.c_str(), dist);
    }
}
```
