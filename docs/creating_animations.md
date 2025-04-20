# Creating Animations (Scenes)

This guide walks you through creating a basic animation (called a "Scene") using the PixelTheater library.

## 1. Prerequisites

*   Familiarity with C++.
*   PlatformIO environment set up for the project.
*   A generated Model header file (e.g., `models/MyModel/model.h`) defining your LED geometry.

## 2. Create the Scene File

Create a new header file for your scene, typically within the `src/scenes/` directory (e.g., `src/scenes/my_scene/my_scene.h`).

## 3. Basic Scene Structure

Start with the basic class structure, inheriting from the `Scene` alias provided by `SceneKit.h`:

```cpp
#pragma once

#include "PixelTheater/SceneKit.h" // Include SceneKit for core types and helpers

namespace Scenes {

class MyScene : public Scene { // Inherit from the aliased Scene type
public:
    // Constructor (usually default is fine)
    MyScene() = default;

    // Required lifecycle methods
    void setup() override {
        // Initialization code goes here
        set_name("My First Scene"); // Set basic metadata
        // position = 0.0f; // Initialize state if needed
    }

    void tick() override {
        // Animation logic goes here (runs every frame)
        Scene::tick(); // Recommended to call base tick
    }

private:
    // Example state variable
    // float position = 0.0f;
};

} // namespace Scenes
```

*   Include `PixelTheater/SceneKit.h`. This provides convenient access to core types (`Scene`, `CRGB`, `CHSV`, `CRGBPalette16`), constants (`PT_PI`, `PT_TWO_PI`), and utility functions (`map`, `nblend`, `fadeToBlackBy`, `colorFromPalette`) directly within the `Scenes` namespace.
*   Use a namespace (e.g., `Scenes`).
*   Inherit publicly from the aliased `Scene` type.
*   Override the pure virtual `setup()` method.
*   Override the `tick()` method for animation logic.

### Including Headers

Including `PixelTheater/SceneKit.h` is the recommended approach for scene development. It automatically pulls in the necessary parts of `PixelTheater.h` and provides aliases for common symbols within the `Scenes` namespace, reducing the need for `PixelTheater::` qualifiers.

Avoid `using namespace PixelTheater;` or `using namespace PixelTheater::Constants;` when using `SceneKit.h`, as it provides the aliases directly.

## 4. Defining Parameters

Make your scene configurable by adding parameters in `setup()` using the `param()` helper:

```cpp
void setup() override {
    set_name("My Scene Name");
    set_description("A description of what this scene does");

    // Simple float parameter (0.0 to 1.0)
    param("speed", "ratio", 0.5f, "clamp", "Animation speed");

    // Integer parameter with range [1, 10]
    param("count", "count", 1, 10, 5, "", "Number of items");

    // Boolean switch
    param("enabled", "switch", true, "", "Enable the effect");
}
```

See `Parameters.md` for all available parameter types and flags.

## 5. Implementing Animation Logic (`tick()`)

Use the `tick()` method to update the animation state and set LED colors. Access LEDs, geometry, parameters, and utilities via the `Scene` base class helpers and the aliases provided by `SceneKit.h`:

```cpp
void tick() override {
    Scene::tick(); // Track frame count

    // Get parameter values
    float speed = settings["speed"];
    int count = settings["count"];
    bool enabled = settings["enabled"];

    // Get time delta for frame-rate independent movement
    float dt = deltaTime();

    if (!enabled) {
        // Optionally clear LEDs if disabled
        for(size_t i=0; i < ledCount(); ++i) leds[i] = CRGB::Black; // Use aliased CRGB
        return; // Skip rest of animation
    }

    // Fade existing colors slightly
    for(size_t i=0; i < ledCount(); ++i) {
        leds[i].fadeToBlackBy(10); // Use aliased fadeToBlackBy
    }

    // Example: Move a conceptual position using deltaTime
    // float current_pos = 0.0f; // Assume this is a member variable
    // current_pos += speed * dt * 100.0f; // Move 100 units/sec at speed=1.0
    // current_pos = fmodf(current_pos, 200.0f); // Wrap around

    // Animate based on parameters and geometry
    for (size_t i = 0; i < ledCount(); ++i) {
        const auto& p = model().point(i);

        // Example: pulse brightness based on Z coordinate and time (millis)
        uint32_t time_ms = millis();
        float phase = (p.z() * 0.1f) + (time_ms / 1000.0f * speed * PT_TWO_PI);
        // Note: sin8/cos8 require PixelTheater:: qualifier
        uint8_t brightness = PixelTheater::sin8(static_cast<uint8_t>(phase * 255 / PT_TWO_PI));

        // Set LED color (only if brightness > 0)
        if (brightness > 10) {
           leds[i] = CHSV(160, 255, brightness); // Use aliased CHSV
        }
    }
}
```

**Key Helpers Used (via `SceneKit.h` unless noted):**
*   `settings["param_name"]`: Access parameter values.
*   `ledCount()`: Get the number of LEDs.
*   `leds[i]`: Access individual LEDs (returns `CRGB&`).
*   `model().point(i)`: Get the 3D point (`PixelTheater::Point`) for LED `i`.
*   `millis()`: Get current time in milliseconds.
*   `deltaTime()`: Get time since last frame in seconds (crucial for physics/smooth movement).
*   `PixelTheater::sin8()`: Fast trig function (requires `PixelTheater::` qualifier).
*   `CHSV`, `CRGB`: Color structs (aliased).
*   `PT_PI`, `PT_TWO_PI`: Constants (aliased).
*   `map`, `fadeToBlackBy`, `nblend`, `colorFromPalette`: Utility functions (aliased).

## 5a. Practical Math & Geometry Usage

While `Scenes.md` lists basic math utilities, understanding how to combine them with geometry (`model().point(i)`) and potentially external libraries like Eigen (often available via includes) unlocks powerful animation techniques. Here are common applications:

*   **Time-based Animation:** Use `millis()`, `deltaTime()`, or `tickCount()` combined with parameters (like `speed`) to drive changes over time. `millis()` is good for simple periodic effects, while `deltaTime()` is essential for frame-rate independent physics and smooth motion (e.g., `position += velocity * deltaTime();`).

*   **Smooth Oscillations:** Use `PixelTheater::sin8(phase)` and `PixelTheater::cos8(phase)` (with qualifier) for smooth pulses or movements. The `phase` typically changes over time (e.g., `millis() / 20`), scaled by parameters. Remember the 0-255 input range corresponds to 0-360 degrees.

*   **Mapping Values:** Use `map()` (aliased by SceneKit) like Arduino's map for converting ranges, e.g., mapping a coordinate (`p.z()`) or a time value to a brightness or hue range.

*   **Using Geometry & Coordinates:** Access LED positions via `model().point(i).x()`, `.y()`, `.z()`. These coordinates are essential for making animations react to the physical layout.
    *   **Cartesian:** Use `x, y, z` directly for effects based on position (e.g., brightness based on height).
    *   **Spherical:** Convert Cartesian coordinates to spherical (azimuth, elevation) using `std::atan2(y, x)` and `std::acos(z / radius)`. This is useful for effects that wrap around a sphere, like latitude/longitude grids (`OrientationGridScene`) or mapping angles to colors.

*   **Rotations:** To rotate the entire model or parts of it:
    *   Define rotation angles (e.g., `spin += speed * deltaTime();`).
    *   Use `std::cos(angle)` and `std::sin(angle)` from `<cmath>` to construct 3D rotation matrices manually. You can use `Eigen::Matrix3f` for the matrix type, as it's typically available.
    *   Multiply the original point `Vector3f(p.x(), p.y(), p.z())` by the rotation matrix (or sequence of matrices) to get the rotated position.
    *   Use the rotated position in your animation logic.
    *   *See `OrientationGridScene` for a detailed example of manual matrix construction and combination.*

    ```cpp
    // Conceptual Rotation Example (Manual Matrix Construction)
    // Assumes Vector3f and Matrix3f are available via PixelTheater includes
    #include <cmath> // For std::cos, std::sin

    // In tick():
    spin_angle += rotation_speed * deltaTime();
    float cosSpin = std::cos(spin_angle);
    float sinSpin = std::sin(spin_angle);

    Matrix3f rot_z; // Rotation around Z axis
    rot_z << cosSpin, -sinSpin, 0,
             sinSpin,  cosSpin, 0,
                   0,        0, 1;

    for (size_t i = 0; i < ledCount(); ++i) {
        const auto& p = model().point(i);
        Vector3f original_pos(p.x(), p.y(), p.z());
        Vector3f rotated_pos = rot_z * original_pos; // Apply rotation
        // Use rotated_pos.x(), rotated_pos.y(), rotated_pos.z() for animation logic
        // ... e.g., map rotated_pos.z() to brightness ...
        leds[i] = /* color based on rotated_pos */ ; // Use aliased CRGB here
    }
    ```

*   **Distance Calculations:**
    *   **Euclidean:** `sqrt(dx*dx + dy*dy + dz*dz)` or use Eigen's `(vec1 - vec2).norm()`. Useful for effects based on proximity to a point.
    *   **Spherical/Angular:** For points on a sphere, calculate the angle between their normalized direction vectors using `std::acos(vec1.normalized().dot(vec2.normalized()))`. Useful for interactions based on angular separation (`BoidsScene`).

*   **Simulation (e.g., Physics, Agents):** For effects like particle systems or flocking (`BoidsScene`):
    *   Represent state using vectors (e.g., `Eigen::Vector3f pos`, `vel`).
    *   Calculate forces or influences as vectors (e.g., `separation_force`, `gravity`).
    *   Update velocity: `vel += force * deltaTime();`. Handle accumulation of multiple forces.
    *   Update position: `pos += vel * deltaTime()`.
    *   Apply constraints: Limit speed (`vel = vel.normalized() * max_speed`), keep objects on a surface (e.g., project velocity onto tangent plane using dot products), handle boundaries.

*   **Mathematical Models:** Drive animations using mathematical formulas or systems, like the Lorenz attractor in `GeographyScene`, where `x, y, z` evolve over time and are mapped to colors or positions.

*   **Randomness:** Use `random...()` functions for adding variation. While global functions like `PixelTheater::random8()` or `PixelTheater::randomFloat()` exist (requiring the qualifier), it's often more convenient to use the *member functions* inherited from `Scene`, like `random(max)` or `randomFloat(min, max)`, which don't need a qualifier.

Remember to include necessary headers like `<cmath>` when using standard math functions. Types like `Vector3f` and `Matrix3f` are usually available via the `PixelTheater/SceneKit.h` include (which includes `PixelTheater.h`), but avoid including specific Eigen headers directly unless absolutely necessary and you understand the build implications. Always use the `PixelTheater::` qualifier for library functions not explicitly aliased by `SceneKit.h` (like `sin8`, `cos8`).

## 5b. Practical Color Techniques

`Color.md` and `Palettes.md` detail the color structures and palette functions. Here's how to apply them effectively using `SceneKit.h` aliases:

*   **`CRGB` vs `CHSV`:**
    *   Use `CHSV` when you want to easily manipulate Hue (cycling colors), Saturation, or Value independently. It's great for rainbow effects or smoothly changing color properties.
    *   Use `CRGB` for direct color setting, mixing (blending), and fading, especially when performance is critical. Conversions between them are available but have a small cost.
    *   Remember `leds[...]` stores `CRGB`, so assigning a `CHSV` performs an implicit conversion.

*   **Effective Palette Use:**
    *   **Cycling:** Use `tickCount()` or a time-based variable as the `index` for `colorFromPalette` to cycle through the palette over time.
    *   **Mapping:** Map spatial properties (like `p.z()`, `p.x()`, or distance from a point) to the 0-255 `index` to paint the geometry with the palette.
    *   **Blending Type:** `LINEARBLEND` gives smoother transitions but is slightly slower. `NOBLEND` is faster and useful for distinct color bands or effects where blending isn't desired.

    ```cpp
    // Example: Mapping Z coordinate to a palette index
    const auto& p = model().point(i);
    float normalizedZ = (p.z() + 1.0f) / 2.0f; // Normalize 0.0 - 1.0
    uint8_t paletteIndex = normalizedZ * 255;
    // Use aliased colorFromPalette and CRGB
    CRGB color = colorFromPalette(
        PixelTheater::Palettes::LavaColors, // Choose palette (namespace needed)
        paletteIndex,
        200, // Brightness
        PixelTheater::LINEARBLEND // Blending mode (namespace needed)
    );
    leds[i] = color;
    ```

*   **Fading:** The `leds[i].fadeToBlackBy(amount)` method (aliased) is fundamental for many animations, creating trails or decaying effects. Apply it at the start of `tick()` before setting new pixel values.

*   **Blending:** Use `blend(currentColor, targetColor, amount)` (aliased) for smoothly transitioning a pixel towards a new color over several frames.

*   **Color Measurement:** Functions in `PixelTheater::ColorUtils` (like `get_hue_distance`, `get_perceived_brightness`) require the full `PixelTheater::` qualifier. They can be useful for more advanced logic, like ensuring color contrast or selecting colors based on perceived brightness. See `Color.md` for details.

## 6. Adding the Scene to the Show

In your main application (`src/main.cpp`), include your scene's header and add it to the `Theater` during setup:

```cpp
// In src/main.cpp
#include "PixelTheater.h" // Main app still uses this
#include "models/MyModel/model.h" // Your model
#include "scenes/my_scene/my_scene.h" // Your new scene (uses SceneKit.h internally)

PixelTheater::Theater theater;

void setup() {
    // ... hardware setup ...

    theater.useFastLEDPlatform<PixelTheater::Models::MyModel>(...); // Initialize Theater

    theater.addScene<Scenes::MyScene>(); // Add your scene
    // Add other scenes...

    theater.start(); // Start the show
}

void loop() {
    theater.update(); // Run the current scene
    // ... handle input ...
}
```

Now, when you build and run, your scene will be part of the sequence managed by the `Theater`!

Refer to `Scenes.md` for a more detailed API reference.

## 7. Web Build Considerations

The WebGL simulator (`build_web.sh`) compiles a specific list of PixelTheater library source files (`.cpp`). If your new scene uses library functions (e.g., from `color/measurement.cpp`, `core/other_utils.cpp`, etc.) whose source files were not previously included in `build_web.sh`, you might encounter linker errors (like `undefined symbol`) when building for the web.

If this happens, you'll need to edit `build_web.sh` and add the required `.cpp` file(s) to the list in the `emcc`