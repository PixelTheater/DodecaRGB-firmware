# Easing Functions

Easing functions allow you to create smoother, more natural-looking animations by controlling the rate of change of a value over time. Instead of moving linearly from a start value to an end value, easing functions introduce acceleration and deceleration, following specific mathematical curves.

The PixelTheater library provides a set of common easing functions accessible via `#include "PixelTheater/SceneKit.h"`, which brings them into the `Scenes` namespace.

## Usage

There are two main ways to use the provided easing functions:

1.  **Fractional Easing Functions (`...F`)**:
    *   These functions take a single `float` input `t` representing the progress through the animation, typically normalized between 0.0 (start) and 1.0 (end).
    *   They return a `float` representing the *eased* progress, also between 0.0 and 1.0.
    *   You then use this eased fraction to interpolate between your start and end values manually.
    *   **Example:** `float eased_t = outQuadF(t); float currentValue = startValue + (endValue - startValue) * eased_t;`

2.  **Interpolating Easing Functions**:
    *   These functions take three `float` inputs: `start`, `end`, and the progress `t` (0.0 to 1.0).
    *   They directly return the interpolated value between `start` and `end`, calculated using the corresponding fractional easing function internally.
    *   **Example:** `float currentValue = outQuad(startValue, endValue, t);`

Generally, the fractional functions (`...F`) offer more flexibility if you need the eased progress value itself for other calculations, while the interpolating versions are slightly more convenient if you just need the final eased value. Remember to clamp your input `t` to the range [0.0, 1.0] before passing it to an easing function if it might go outside that range. The interpolating functions handle this clamping internally via the generic `ease` template.

## Available Functions

The following easing functions are available (both fractional `...F` versions and interpolating versions are typically provided):

*   **`linearF(t)` / `linear(start, end, t)`**:
    *   No easing, constant rate of change. Useful as a baseline or when no curve is desired.

*   **`inSineF(t)` / `inSine(start, end, t)`**:
    *   Starts slow and accelerates (Ease In). Curve resembles the first quarter of a sine wave.

*   **`outSineF(t)` / `outSine(start, end, t)`**:
    *   Starts fast and decelerates (Ease Out). Curve resembles the second quarter of a sine wave.

*   **`inOutSineF(t)` / `inOutSine(start, end, t)`**:
    *   Starts slow, accelerates through the middle, then decelerates (Ease In/Out). Uses a full sine wave curve shape.

*   **`inQuadF(t)` / `inQuad(start, end, t)`**:
    *   Quadratic easing in (\(t^2\)). Starts slow, accelerates more sharply than Sine.

*   **`outQuadF(t)` / `outQuad(start, end, t)`**:
    *   Quadratic easing out (\(1 - (1-t)^2\)). Starts fast, decelerates more sharply than Sine.

*   **`inOutQuadF(t)` / `inOutQuad(start, end, t)`**:
    *   Quadratic easing in/out. Combines `inQuad` and `outQuad`.

*   *(Note: More functions like Cubic, Quart, Quint, Expo, Circ, Back, Elastic, Bounce might be available in `PixelTheater/easing.h` but may not all be aliased by `SceneKit.h` by default. Check `SceneKit.h` for the exact list of readily available aliases.)*

*(You can find visual representations of these curves on websites like [easings.net](https://easings.net/))*

## Examples

```cpp
#include "PixelTheater/SceneKit.h"
#include <cmath> // For std::fmod

namespace Scenes {

class EasingExampleScene : public Scene {
    float progress = 0.0f;
    float startX = 10.0f;
    float endX = 110.0f;

public:
    void setup() override {
        set_name("Easing Example");
        param("speed", "ratio", 0.2f, "clamp", "Animation Speed");
    }

    void tick() override {
        Scene::tick();
        float speed = settings["speed"];
        float dt = deltaTime();

        // Update progress (0.0 to 1.0 and wrap around)
        progress = std::fmod(progress + speed * dt * 0.5f, 1.0f); // Wraps every ~2s at speed=1

        // --- Fractional Easing Example ---
        // Use outQuadF to ease the progress value
        float easedProgress = outQuadF(progress);
        // Manually interpolate using the eased progress
        float currentX_fractional = startX + (endX - startX) * easedProgress;
        // Example: Map eased X position to brightness for LED 0
        if (ledCount() > 0) {
             uint8_t brightness = map(static_cast<long>(currentX_fractional), static_cast<long>(startX), static_cast<long>(endX), 0, 255);
             leds[0] = CHSV(0, 0, brightness); // White pulse using outQuad ease
        }


        // --- Interpolating Easing Example ---
        // Use inOutSine directly to get the eased value
        float currentX_interpolating = inOutSine(startX, endX, progress);
        // Example: Map eased X position to hue for LED 1
        if (ledCount() > 1) {
             uint8_t hue = map(static_cast<long>(currentX_interpolating), static_cast<long>(startX), static_cast<long>(endX), 0, 255);
             leds[1] = CHSV(hue, 255, 255); // Hue shift using inOutSine ease
        }

        // --- Linear Example (for comparison) ---
         float currentX_linear = linear(startX, endX, progress);
        // Example: Map linear X position to hue for LED 2
        if (ledCount() > 2) {
             uint8_t hue = map(static_cast<long>(currentX_linear), static_cast<long>(startX), static_cast<long>(endX), 0, 255);
             leds[2] = CHSV(hue, 255, 255); // Hue shift using linear ease
        }
    }
};

} // namespace Scenes
``` 