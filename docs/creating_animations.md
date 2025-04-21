# Creating Animations (Scenes): Techniques Guide

This guide focuses on practical techniques for creating dynamic and interesting visual effects using the PixelTheater framework. It assumes you understand the basic structure of a Scene.

- For the basic structure and API reference, see the [Scene API Reference Guide](../PixelTheater/Scenes.md).
- For details on Parameters, see the [Parameters Guide](../PixelTheater/Parameters.md).

## Core Concepts Refresher

Remember these key elements available within your Scene class (typically via `SceneKit.h`):

*   **`tick()`:** Your main animation loop, called every frame.
*   **`deltaTime()`:** Time (in seconds) since the last frame. Crucial for smooth, frame-rate independent movement and physics.
*   **`millis()`:** Milliseconds since program start. Useful for longer-term timing or simple oscillations.
*   **`settings["param_name"]`:** Access user-configurable parameters defined in `setup()`.
*   **`leds[]` and `ledCount()`:** Access the LED buffer.
*   **`model()`:** Access the 3D model geometry (`point()`, `face()`, `getSphereRadius()`).
*   **Base Class Call:** It's generally recommended to call `Scene::tick()` at the start of your `tick()` method to increment `tick_count()`.

## Time-Based Animation

Use timing functions to drive changes.

*   **Frame-Rate Independent Movement:** Multiply velocities or rates by `deltaTime()`.
    ```cpp
    // Example: Move a conceptual position
    position += velocity * speed * deltaTime(); // speed is a parameter
    ```
*   **Periodic Effects:** Use `millis()` with modulo or trigonometric functions. For fast 8-bit sine/cosine waves where precision isn't critical, consider `PixelTheater::sin8()` and `PixelTheater::cos8()` (requires `PixelTheater::` qualifier).
    ```cpp
    // Example: Simple pulse every 2 seconds
    uint32_t time_ms = millis();
    float phase = (time_ms % 2000) / 2000.0f; // 0.0 to 1.0 over 2 seconds
    uint8_t brightness = static_cast<uint8_t>(std::sin(phase * PT_TWO_PI) * 127.5f + 127.5f);
    ```
*   **Timed Events:** Use `tick_count()` or accumulate `deltaTime()` to trigger events after a certain duration or number of frames.

## Working with Color

*   **`CRGB` vs `CHSV`:** Use `CHSV` for easy manipulation of hue, saturation, or value (brightness). Use `CRGB` for direct color setting, blending, and performance-critical operations. Assigning `CHSV` to `leds[]` converts automatically.
*   **Using Palettes:** `colorFromPalette()` lets you sample colors from a `CRGBPalette16`.
    *   **Cycling:** Use `millis()` or `tickCount()` as the index.
    *   **Mapping:** Map spatial data (e.g., normalized `p.z()`) to the 0-255 index.
    *   **Blending Type:** `LINEARBLEND` (smooth) vs. `NOBLEND` (snapped).
    *   **Cross-Platform Note:** For compatibility between hardware and the web simulator, always use palette constants from the `PixelTheater::Palettes` namespace (e.g., `PixelTheater::Palettes::PartyColors`) instead of hardware-specific PROGMEM variables (like `PartyColors_p`).
    ```cpp
    // Example: Map Z coordinate to LavaColors palette
    const auto& p = model().point(i);
    float normalizedZ = (model().getSphereRadius() > 0) ? (p.z() / model().getSphereRadius() + 1.0f) / 2.0f : 0.5f; // Normalize -1..1 to 0..1
    uint8_t paletteIndex = normalizedZ * 255;
    CRGB color = colorFromPalette(
        PixelTheater::Palettes::LavaColors,
        paletteIndex, 200, PixelTheater::LINEARBLEND
    );
    leds[i] = color;
    ```
*   **Smooth Color Transitions:** Use `lerp8by8()` (aliased by `SceneKit.h`) for fast 8-bit interpolation of individual R, G, or B components, or `blend()` for blending whole `CRGB` colors.
    ```cpp
    // Example: Lerp red component over time
    float progress = std::fmod(millis() / 5000.0f, 1.0f); // 0.0 to 1.0 over 5s
    uint8_t redValue = lerp8by8(0, 255, static_cast<uint8_t>(progress * 255.0f));
    if (ledCount() > 0) leds[0].r = redValue;
    ```

## Fading and Blending Techniques

*   **Trails/Decay:** Use `leds[i].fadeToBlackBy(amount)` (or the global `fadeToBlackBy(leds,...)`) at the *start* of `tick()` before drawing new colors.
    ```cpp
    // At the start of tick()
    uint8_t fadeAmount = 15;
    for(size_t i=0; i < ledCount(); ++i) {
        leds[i].fadeToBlackBy(fadeAmount);
    }
    // ... draw new colors ...
    ```
*   **Layering Effects:** Use `blend()` or `nblend()` (aliased by `SceneKit.h`) to mix new colors onto existing pixel colors.
    ```cpp
    CRGB newSparkleColor = CRGB::Yellow;
    uint8_t blendAmount = 128; // 50% mix
    nblend(leds[pixelIndex], newSparkleColor, blendAmount);
    ```
*   **Clearing LEDs:** Use `fill_solid(leds, CRGB::Black);` (utility from `SceneKit.h`) for a concise way to clear the buffer.

## Leveraging Geometry

Access 3D coordinates via `model().point(i).x()`, `.y()`, `.z()`. Use `model().getSphereRadius()` for normalization if needed.

*   **Spatial Mapping:** Map coordinates directly to visual properties.
    ```cpp
    // Example: Brightness based on height (Z)
    const auto& p = model().point(i);
    float heightFactor = (model().getSphereRadius() > 0) ? (p.z() / model().getSphereRadius() + 1.0f) / 2.0f : 0.5f;
    leds[i] = CHSV(160, 255, static_cast<uint8_t>(heightFactor * 255));
    ```

## Spatial Techniques

*   **Coordinate Systems:** Understand the difference between Cartesian (`x,y,z`) and Spherical (azimuth/longitude, elevation/latitude) coordinates. Use `<cmath>` functions like `std::atan2(y, x)` and `std::acos(z / radius)` for conversions when needed (e.g., for wrapping effects on a sphere).
*   **Distance Calculations:**
    *   Euclidean: Use `model().point(i).distanceTo(model().point(j))` to get the distance between two LED points. Useful for proximity effects.
    *   Angular (on sphere): `std::acos(vec1.normalized().dot(vec2.normalized()))`. Useful for surface interactions.
*   **Neighbor-Based Effects**: You can create effects based on direct LED connections using `model().point(i).isNeighbor(model().point(j))` or by iterating through the array returned by `model().point(i).getNeighbors()`. This is useful for spreading light or simulating interactions along the model's surface mesh.
*   **Rotations:** Apply 3D rotations using trigonometric functions (`std::cos`, `std::sin`) and matrix math, ideally using Eigen types (`Eigen::Matrix3f`). Multiply point vectors (`Eigen::Vector3f`) by rotation matrices. See `OrientationGridScene` for a detailed example.

## Movement and Simulation

*   **Basic Physics:** Update position using velocity and `deltaTime()`: `position += velocity * deltaTime();`. Use `Eigen::Vector3f` for position and velocity for easier 3D math.
*   **Easing for Smooth Motion:** Apply easing functions to smooth out changes in position, brightness, size, etc. See the [Easing Functions Guide](../PixelTheater/Easing.md) for details.
    ```