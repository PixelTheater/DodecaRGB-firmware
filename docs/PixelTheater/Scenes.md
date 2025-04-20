---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Scene API Reference

This document provides a quick reference to the API available to authors creating custom animations by inheriting from `PixelTheater::Scene`.

- For a tutorial on creating scenes, see the [Creating Animations Guide](../creating_animations.md).
- For details on Parameters, see the [Parameters Guide](Parameters.md).
- For Color/Palette details, see [Color System](Color.md) and [Palettes API](Palettes.md).
- For Model/Geometry details, see [Model System](Model.md).

## Basic Structure & Example

All scenes must inherit from the `Scene` base class provided by `PixelTheater/SceneKit.h` and implement the `setup()` and `tick()` methods.

```cpp
#pragma once

// Include SceneKit for core types and helpers
#include "PixelTheater/SceneKit.h"

namespace Scenes {

// Note: Inherit PUBLICLY from the aliased `Scene` type
class MyScene : public Scene {
public:
    // --- Required ---
    // Constructor (usually default is fine)
    MyScene() = default;
    // Virtual destructor is important for proper cleanup
    ~MyScene() override = default;

    // Called once when scene added or reset. Define metadata/params here.
    void setup() override;
    // Called every frame. Implement animation logic here.
    void tick() override;

    // --- Optional Overrides ---

    // Called when switching back to this scene after it was inactive.
    // Default resets tick_count and parameter values to defaults.
    // Call Scene::reset() if overriding.
    void reset() override {
        Scene::reset(); // Call base implementation first
        // ... custom reset logic ...
    }

    // If parameter definitions become too numerous for setup(),
    // you can optionally define them here instead.
    void config() override {
       param(...);
       // ... other params ...
    }

    // Provide a concise status string for debugging/logging.
    // (Currently not used by default logging).
    std::string status() const override {
        return "Mode: " + std::to_string(static_cast<int>(settings["mode"]));
    }

private:
    // Scene-specific state variables
    // float position = 0.0f;
    // float velocity = 0.0f;
};

} // namespace Scenes
```

**`setup()` Implementation Example:**

```cpp
void MyScene::setup() {
    // 1. Define Metadata (Used by UI/logging)
    set_name("My Awesome Scene");
    set_author("Your Name");
    set_description("Briefly describe what the scene does.");
    // set_version("1.0"); // Optional
    // meta("CustomKey", "CustomValue"); // Optional key-value metadata

    // 2. Define Parameters (Controls) - See Parameters.md for details
    param("speed", "ratio", 0.5f, "clamp", "Animation speed (0-1)");
    param("color_hue", "ratio", 0.0f, "wrap", "Base color hue (0-1 maps to 0-255)");
    param("enabled", "switch", true, "", "Enable the main effect");
    param("mode", "select", {"A", "B", "C"}, "A", "", "Select operation mode");

    // 3. Initialize any internal state variables if needed
    // position = 0.0f;
    // velocity = randomFloat(-10.0f, 10.0f); // Use member random
}
```

**`tick()` Implementation Example:**

```cpp
void MyScene::tick() {
    Scene::tick(); // Recommended: Call base tick to increment tick_count()

    // Access parameter values via settings proxy
    float current_speed = settings["speed"];
    float current_hue_norm = settings["color_hue"];
    bool is_enabled = settings["enabled"];
    int selected_mode_index = settings["mode"]; // select returns index

    // Get time delta for frame-rate independent movement
    float dt = deltaTime();

    // --- Animation logic based on parameters and time ---
    if (!is_enabled) {
        fill_solid(leds, CRGB::Black); // Use aliases from SceneKit
        return;
    }

    // Example: Update position based on velocity and deltaTime
    // position += velocity * current_speed * dt;

    // Convert normalized hue to 0-255
    uint8_t current_hue = current_hue_norm * 255.0f;

    for (size_t i = 0; i < ledCount(); ++i) {
        // Example: Simple traveling sine wave using time (millis) and index
        uint32_t time_ms = millis();
        uint8_t phase = (time_ms / 20) + i * 10; // Combine time and position
        // Note: sin8 requires PixelTheater:: qualifier
        uint8_t val = PixelTheater::sin8(static_cast<uint8_t>(phase * current_speed)); // Use speed param
        leds[i] = CHSV(current_hue, 255, val); // Use hue param (SceneKit provides CHSV)
    }
}
```

## Available API within Scene Subclass

### Lifecycle Methods (Override)

*   `virtual void setup()`: **(Required)** Called once when the scene is added or reset. Define metadata and parameters here. Initialize internal state.
*   `virtual void tick()`: **(Required)** Called every frame. Implement animation logic here. Access parameters via `settings` to make the animation react to runtime changes.
*   `virtual void reset()`: **(Optional)** Called when scene becomes active after being inactive. Default implementation resets `tick_count` and parameter values to their defaults. Call `Scene::reset()` if overriding.
*   `virtual void config()`: **(Optional)** Alternative place to define parameters using `param(...)` if `setup()` becomes too complex. Called after the constructor.
*   `virtual ~Scene()`: **(Required override, usually `= default`)** Virtual destructor ensures proper cleanup if subclass adds members needing destruction.

### Metadata Definition (in `setup()` or `config()`)

Define these to identify your scene. They are used by logging systems and UIs.

*   `set_name(const std::string& name)`
*   `set_description(const std::string& desc)`
*   `set_version(const std::string& ver)`
*   `set_author(const std::string& author)`
*   `meta(const std::string& key, const std::string& value)`: For custom key-value metadata (e.g., `meta("Source", "URL")`).

### Parameter Definition & Access

Parameters define the user-controllable aspects of your scene. Define them in `setup()` or `config()` using `param()`. Read the current values in `tick()` using `settings["param_name"]`.

*   `param(...)` (Protected method): Define parameters. Multiple overloads exist for different types (`ratio`, `count`, `range`, `switch`, `select`, `color`, `gradient`). The `description` argument is important for UI labels/tooltips. See the [Parameters Guide](Parameters.md) for signatures and options.
*   `settings["param_name"]` (`SettingsProxy` member): Access/modify current parameter values. Provides implicit type conversion.
    *   Example: `float speed = settings["speed"];`
    *   Supports iteration: `for (const auto& pair : settings) { ... }`
    *   See [Parameters Guide](Parameters.md) for details.
*   `has_parameter(const std::string& name) const`: Check if a parameter exists.
*   `get_parameter_value(const std::string& name) const`: Get raw parameter value (variant).
*   `get_parameter_metadata(...)`: Access metadata associated with a parameter.

### Metadata Accessors (Read-only)

*   `name()` (`const std::string&`)
*   `description()` (`const std::string&`)
*   `version()` (`const std::string&`)
*   `author()` (`const std::string&`)
*   `get_meta(const std::string& key) const`: Retrieve value set by `meta()`.

### LED Access

*   `leds[index]` (`LedsProxy` member): Provides `CRGB&`. Index is bounds-clamped.
*   `led(index)` (`CRGB&` method): Helper access. Index is bounds-clamped.
*   `ledCount()` (`size_t` method): Returns total number of LEDs.

### Model Geometry Access

*   `model()` (`const IModel&` method): Returns reference to the model interface.
*   `model().point(index)` (`const Point&`): Get point data for LED `index`. Index is bounds-clamped.
*   `model().face(index)` (`const Face&`): Get face data for face `index`. Index is bounds-clamped.
*   `model().pointCount()` (`size_t`): Total number of points (usually == `ledCount()`).
*   `model().faceCount()` (`size_t`): Total number of faces.

### Timing Utilities

*   `millis()` (`uint32_t`): Milliseconds since program start (provided by Platform).
*   `deltaTime()` (`float`): Time elapsed since the last frame (in seconds, provided by Platform). Essential for frame-rate independent physics/movement.
*   `tick_count()` (`size_t`): Number of `tick()` calls since the scene was last activated/reset. Incremented by `Scene::tick()`.

### Math/Random Utilities (provided by PixelTheater, some aliased by SceneKit)

*   **Random Functions:**
    *   *Member Functions (Recommended):* The `Scene` base class provides convenient member functions like `random(max)`, `random(min, max)`, `randomFloat()`, `randomFloat(min, max)`. Use these directly without a qualifier (e.g., `float speed = randomFloat(-1.0f, 1.0f);`).
    *   *Global Functions:* `PixelTheater` also provides global random functions like `PixelTheater::random8()`, `PixelTheater::random16()`, `PixelTheater::randomFloat()`. These require the `PixelTheater::` qualifier and are generally less convenient within a scene than the member functions.
*   **Global Math/Color Utilities:** `map()`, `nblend()`, `fadeToBlackBy()`, `fill_solid()`, `colorFromPalette()`, `CRGB`, `CHSV`, etc., are available via `PixelTheater/SceneKit.h` within `namespace Scenes`. Functions *not* explicitly aliased by `SceneKit` (e.g., `PixelTheater::sin8`, `PixelTheater::cos8`) require the `PixelTheater::` qualifier. See [Color System](Color.md), [Palettes API](Palettes.md), and the [Creating Animations Guide](../creating_animations.md).
*   **Constants:** `PT_PI`, `PT_TWO_PI`, etc., are aliased by `SceneKit.h` within `namespace Scenes`. Direct use (e.g., `PT_PI`) is recommended.

### Logging Utilities

Requires a `LogProvider` configured in the `Platform`. Format string support is basic (like `printf`).

*   `logInfo(const char* format, ...)`
*   `logWarning(const char* format, ...)`
*   `logError(const char* format, ...)`

### Other Utility Methods

*   `virtual std::string status() const`: **(Optional)** Override to return a concise string representing the scene's internal state for debugging/logging.

Refer to the source code headers (`scene.h`, `imodel.h`, `platform.h`, `parameters.h`, etc.) for precise signatures and implementation details.
