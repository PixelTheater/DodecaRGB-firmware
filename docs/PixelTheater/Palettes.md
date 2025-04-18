---
category: Development
generated: 2025-02-10 00:32
version: 2.8.2
---

# PixelTheater Palette API

## Overview

The PixelTheater library provides a platform-transparent API for using 16-entry color palettes within animation scenes. This document focuses specifically on the palette types, constants, and the `colorFromPalette` function. For general color handling (`CRGB`, `CHSV`, blending, fading, etc.), see the main [Using Color in PixelTheater Scenes](Color.md) guide.

Scenes interact solely with types and functions within the `PixelTheater` namespace, allowing the same code to run on Teensy (using FastLED internally) and native/web platforms (using C++ fallbacks).

## Core Types

### `PixelTheater::CRGBPalette16`

A 16-entry color palette.

```cpp
#include <array>
#include "PixelTheater/core/crgb.h" // Provides PixelTheater::CRGB

namespace PixelTheater {
    using CRGBPalette16 = std::array<PixelTheater::CRGB, 16>;
}
```

### `PixelTheater::TBlendType`

Enum defining how colors are retrieved from the palette between the 16 defined entries.

```cpp
namespace PixelTheater {
    enum TBlendType {
        LINEARBLEND = 0, ///< Linear interpolation between palette entries. High quality, slower.
        NOBLEND = 1      ///< No interpolation between palette entries. Low quality, faster.
    };
}
```

## Available Palettes (`PixelTheater::Palettes`)

The `PixelTheater::Palettes` namespace provides predefined `constexpr PixelTheater::CRGBPalette16` constants. These include standard FastLED palettes and custom ones. Include `<PixelTheater/palettes.h>` to use them.

```cpp
#include "PixelTheater/palettes.h"

// Example Access:
const PixelTheater::CRGBPalette16& myPalette = PixelTheater::Palettes::RainbowColors;
const PixelTheater::CRGBPalette16& customPalette = PixelTheater::Palettes::basePalette;
```

**Standard Palettes (Equivalent to FastLED `*_p`):**

*   `CloudColors`
*   `LavaColors`
*   `OceanColors`
*   `ForestColors`
*   `RainbowColors`
*   `RainbowStripeColors`
*   `PartyColors`
*   `HeatColors`

**Custom Palettes:**

*   `basePalette`
*   `highlightPalette`
*   `uniquePalette`
*   *(Others can be added)*

## Core Function (`PixelTheater::colorFromPalette`)

Retrieves a color from a `CRGBPalette16`, handling interpolation and brightness scaling.

```cpp
#include "PixelTheater/color_api.h" // Include this header
#include "PixelTheater/palettes.h"  // For palette constants and type

namespace PixelTheater {

/**
 * @brief Get a color from a 16-entry palette.
 *
 * Handles interpolation between entries based on blend type.
 * Note: Native/Web implementation currently only supports CRGBPalette16,
 * gradient palette support is Teensy-only via FastLED for now.
 *
 * @param pal The 16-entry palette (CRGBPalette16).
 * @param index The 8-bit index (0-255) into the virtual 256-entry palette.
 * @param brightness Optional brightness scale (0-255). Defaults to 255.
 * @param blendType How to blend between the 16 entries (LINEARBLEND or NOBLEND). Defaults to LINEARBLEND.
 * @return PixelTheater::CRGB The calculated color.
 */
CRGB colorFromPalette(const CRGBPalette16& pal,
                       uint8_t index,
                       uint8_t brightness = 255,
                       TBlendType blendType = LINEARBLEND);

} // namespace PixelTheater
```

*   `index`: Maps the full 0-255 range onto the 16 palette entries. If `blendType` is `LINEARBLEND`, it interpolates between entries `index / 16` and `(index / 16) + 1`.
*   `brightness`: Scales the final color.
*   `blendType`: Determines if interpolation occurs (`LINEARBLEND`) or if the color snaps to the nearest lower entry (`NOBLEND`).

## Usage Example

```cpp
#include "PixelTheater.h" // Includes necessary color/palette headers
#include "scenes/my_scene.h" // Your scene header

// Bring namespaces into scope (optional)
using namespace PixelTheater;
// using namespace PixelTheater::Palettes; // Can use this too

void Scenes::MyScene::tick() {
    Scene::tick(); // Base class tick

    uint8_t brightness = 180;
    uint8_t index = tickCount(); // Simple animation index (0-255 wraps)

    // Get color from the standard 'Party' palette
    CRGB color1 = PixelTheater::colorFromPalette(
        PixelTheater::Palettes::PartyColors, // Use predefined constant
        index,
        brightness,
        PixelTheater::LINEARBLEND // Use linear blending
    );

    // Get color from a custom palette without blending
    CRGB color2 = PixelTheater::colorFromPalette(
        PixelTheater::Palettes::basePalette,
        index + 64, // Offset index
        255,        // Full brightness
        PixelTheater::NOBLEND // Snap to nearest entry
    );

    // Use the colors
    if (ledCount() > 1) {
      leds[0] = color1;
      leds[1] = color2;
    }

    // Fade the rest
    for(size_t i = 2; i < ledCount(); ++i) {
        leds[i].fadeToBlackBy(10);
    }
}
```

## Defining Custom Palettes (Advanced)

While scenes *use* palettes via the API above, palettes are *defined* externally.

*   **Format:** Palettes can be defined in JSON files (`*.pal.json`) using a format compatible with [WLED Custom Palettes](https://kno.wled.ge/features/palettes/#custom-palettes). See `util/palettes/` for examples.
*   **Generation:** A Python script (`util/generate_props.py`) processes these JSON files and generates C++ definitions (currently placed in `lib/PixelTheater/src/palettes.cpp` for built-in ones, or potentially elsewhere for user-added ones).
*   **Gradient Palettes:** The JSON format defines *gradient* palettes. While these definitions are used to generate the `CRGBPalette16` constants (by sampling the gradient), the `PixelTheater::colorFromPalette` function **does not currently support runtime lookup directly from the raw gradient data** on native/web platforms. Only the pre-sampled 16-entry `CRGBPalette16` constants work universally. Full gradient support relies on FastLED (Teensy only).
