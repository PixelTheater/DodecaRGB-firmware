---
category: Guide, Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Creating Animations Guide

This guide covers techniques and best practices for creating animation Scenes using the PixelTheater library. It assumes familiarity with the basic [Scene structure](PixelTheater/SceneAuthorGuide.md) and the [development environment](development.md).

*   **Platform Independent:** Code written following this guide, using only the `PixelTheater` namespace API, will run correctly on Teensy, native, and web simulator platforms.

## LED Management

Scenes access and modify LEDs through the `leds` proxy object, which behaves like an array.

*   **Access:** Use `leds[index]` to get/set the `PixelTheater::CRGB` value of an LED. Access is bounds-checked.
*   **Count:** Use `ledCount()` to get the total number of LEDs.
*   **Updates:** The framework handles calling the underlying platform's `show()` method after your `tick()` finishes.

### Color Modification

Use `PixelTheater::CRGB` methods or `PixelTheater` utility functions:

```cpp
#include "PixelTheater.h"
using namespace PixelTheater; // Optional, for brevity

void MyScene::tick() {
    Scene::tick();

    // Get total LED count
    size_t numLeds = ledCount();

    // Direct assignment
    leds[0] = CRGB::Red;
    leds[1] = CHSV(160, 200, 150); // Assign HSV

    // Fade an LED towards black
    if (numLeds > 2) {
        leds[2].fadeToBlackBy(32);
    }

    // Scale brightness of an LED
    if (numLeds > 3) {
        leds[3].nscale8(128); // Scale to 50% brightness
    }

    // Blend between two colors
    if (numLeds > 4) {
        CRGB targetColor = CRGB::Blue;
        // Blend LED 4 25% towards blue
        leds[4] = PixelTheater::blend(leds[4], targetColor, 64);
    }

    // Fill a range (e.g., first 10 LEDs)
    // Note: LedsProxy doesn't support std::fill directly
    CRGB fillColor = CRGB::Green;
    for (size_t i = 5; i < 15 && i < numLeds; ++i) {
        leds[i] = fillColor;
    }
    // Or use the template function if available for the range type
    // PixelTheater::fill_solid(some_led_range, fillColor);
}
```

## Using Color Palettes

Use predefined palettes or sample colors from them using `PixelTheater::colorFromPalette`.

```cpp
#include "PixelTheater.h"
using namespace PixelTheater;

void MyScene::tick() {
    Scene::tick();
    uint8_t index = tickCount(); // 0-255 animation index
    size_t numLeds = ledCount();

    // Get the Rainbow palette constant
    const CRGBPalette16& rainbow = Palettes::RainbowColors;

    // Fill LEDs using the palette
    for (size_t i = 0; i < numLeds; ++i) {
        // Calculate index for this LED
        uint8_t ledIndex = index + (i * 10); // Offset index per LED

        // Get color from palette with brightness 200, linear blending
        leds[i] = colorFromPalette(rainbow, ledIndex, 200, LINEARBLEND);
    }
}
```

*   Include `PixelTheater/palettes.h` (usually via `PixelTheater.h`).
*   Access palettes via `PixelTheater::Palettes::PaletteName`.
*   Use `PixelTheater::colorFromPalette` to sample the palette.
*   Refer to the [Palette API docs](PixelTheater/Palettes.md) for available palettes and function details.

## Random Effects

The framework seeds random number generators (`random8`, `random16`, `randomFloat`, etc.) available directly within the Scene class.

```cpp
#include "PixelTheater.h"
using namespace PixelTheater;

void MyScene::tick() {
    Scene::tick();
    size_t numLeds = ledCount();

    // --- Sparkle Example ---
    // Add random white sparkles
    uint8_t sparkleChance = 20; // Lower is less frequent
    if (random8() < sparkleChance) {
        size_t randomLed = random16(numLeds); // Pick random LED index
        leds[randomLed] = CRGB::White;
    }

    // Fade all LEDs slowly
    for (size_t i = 0; i < numLeds; ++i) {
        leds[i].fadeToBlackBy(10); // Use CRGB method
    }
}
```

## Coordinate Systems

Access LED positions and model geometry via the `model()` helper.

### Linear Addressing (Index-Based)

Access LEDs sequentially using their index (0 to `ledCount()-1`).

```cpp
// Example: Simple rainbow across all LEDs
for(size_t i = 0; i < ledCount(); i++) {
    uint8_t hue = (i * 256) / ledCount();
    leds[i] = CHSV(hue, 255, 255);
}
```

### Face-Based Rendering

Iterate through model faces and access LEDs belonging to each face.

```cpp
#include "PixelTheater.h"
using namespace PixelTheater;

void MyScene::tick() {
    Scene::tick();
    const IModel& geom = model(); // Get model reference

    for (size_t faceIdx = 0; faceIdx < geom.faceCount(); ++faceIdx) {
        const Face& face = geom.face(faceIdx); // Get face data

        // Example: Color face based on index
        CRGB faceColor = (faceIdx % 2 == 0) ? CRGB::Aqua : CRGB::Magenta;

        // Iterate through LEDs ON THIS FACE using its local indices
        for (size_t localLedIdx = 0; localLedIdx < face.ledCount(); ++localLedIdx) {
            size_t globalLedIndex = face.ledIndex(localLedIdx); // Get global index
            leds[globalLedIndex] = faceColor;
        }
    }
}

```

### 3D Coordinates

Access pre-calculated Cartesian (`x, y, z`) coordinates for each LED via `model().point(index)`.

```cpp
#include "PixelTheater.h"
using namespace PixelTheater;

void MyScene::tick() {
    Scene::tick();
    const IModel& geom = model();
    float time = millis() / 1000.0f; // Time in seconds

    // Example: Sine wave brightness based on Z height
    for (size_t i = 0; i < ledCount(); ++i) {
        const Point& p = geom.point(i); // Get point data for LED i

        // Map z-coordinate (approx -1 to 1) to 0-255 range for sine input
        uint8_t z_mapped = map(p.z(), -1.0f, 1.0f, 0.0f, 255.0f);
        // Calculate sine wave offset by time
        uint8_t sin_val = sin8(z_mapped + (uint8_t)(time * 50));

        leds[i] = CHSV(100, 200, sin_val); // Greenish color, brightness from sine
    }
}
```

*   `Point` objects contain `x(), y(), z()` methods.
*   They also provide neighbour information and distance calculations. See `imodel.h` and `point.h`.

### Spherical Coordinates

Calculate spherical coordinates (azimuth, elevation, radius) from Cartesian coordinates if needed for effects like orbits. The model's overall radius can be obtained using `model().getSphereRadius()`.

```cpp
#include "PixelTheater.h"
#include <cmath> // For atan2, acos, sqrt
using namespace PixelTheater;
using namespace PixelTheater::Constants; // For PT_PI

void MyScene::tick() {
    // ... setup ...
    const IModel& geom = model();
    for (size_t i = 0; i < ledCount(); ++i) {
        const Point& p = geom.point(i);
        float x = p.x(), y = p.y(), z = p.z();

        // Calculate spherical coords (example)
        // Note: For effects relative to the overall model radius, use model().getSphereRadius()
        // For the individual point's distance from origin:
        float point_radius = std::sqrt(x*x + y*y + z*z); 
        float azimuth = std::atan2(y, x); // Angle in XY plane (-PI to PI)
        // Use point_radius here if needed for normalization to individual point
        float elevation = (point_radius > 1e-6f) ? std::acos(z / point_radius) : 0.0f; // Angle from Z+ axis (0 to PI)

        // Use spherical coords for animation...
        // Example: Brightness based on angle from X+ axis
        uint8_t brightness = map(azimuth, -PT_PI, PT_PI, 0.0f, 255.0f);
        leds[i].setHue(0); // Red hue
        leds[i].setSaturation(255);
        leds[i].setValue(brightness); // Set brightness (value)
    }
}
```

## Animation Flow & Best Practices

*   **Time:** Use `millis()` for absolute time or `deltaTime()` for time since the last frame. Access the frame count via `tickCount()`.
*   **Avoid `delay()`:** Never use `delay()` inside `tick()`, as it blocks the animation loop.
*   **Parameters:** Use parameters (`param()`, `settings[]`) for configurable values like speed, intensity, color choices.
*   **State:** Store animation state in Scene member variables.
*   **Performance:** Pre-calculate complex values in `setup()` or `reset()` if they don't change per frame. Be mindful of calculations within loops in `tick()`.
