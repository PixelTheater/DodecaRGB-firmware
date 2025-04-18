---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Scene API Reference

This document provides a quick reference to the API available to authors creating custom animations by inheriting from `PixelTheater::Scene`.

For a tutorial on creating scenes, see `docs/creating_animations.md`.
For a more detailed guide, see `docs/PixelTheater/SceneAuthorGuide.md`.

## Basic Structure & `setup()` Example

All scenes must inherit from `PixelTheater::Scene` and implement the `setup()` and `tick()` methods.

```cpp
#pragma once

#include "PixelTheater.h" 

// Optional: Bring namespaces into scope for convenience,
// but explicit qualification (e.g., PixelTheater::CRGB) is often preferred.
// using namespace PixelTheater;
// using namespace PixelTheater::Constants;

namespace Scenes {

// Note: Inherit from PixelTheater::Scene explicitly
class MyScene : public PixelTheater::Scene {
public:
    // Constructor (usually default is fine)
    MyScene() = default;

    // Setup is called once when the scene is added to the Theater.
    // Use it to define metadata and parameters.
    void setup() override {
        // 1. Define Metadata (Name, Author, etc.)
        set_name("My Awesome Scene");
        set_author("Your Name");
        set_description("Briefly describe what the scene does.");
        // set_version("1.0"); // Optional

        // 2. Define Parameters (Controls)
        // See Parameters.md for detailed options
        param("speed", "ratio", 0.5f, "clamp", "Animation speed (0-1)");
        param("color_hue", "ratio", 0.0f, "wrap", "Base color hue (0-1 maps to 0-255)");
        param("enabled", "switch", true, "", "Enable the main effect");
        param("mode", "select", {"A", "B", "C"}, "A", "", "Select operation mode");

        // 3. Initialize any internal state variables if needed
        // my_internal_state = 0;
    }

    // Tick is called repeatedly for each frame.
    // Implement animation logic here.
    void tick() override {
        Scene::tick(); // Recommended: Call base tick to increment tick_count()

        // Access parameter values via settings proxy
        float current_speed = settings["speed"];
        float current_hue_norm = settings["color_hue"];
        bool is_enabled = settings["enabled"];
        int selected_mode_index = settings["mode"]; // select returns index

        // Convert normalized hue to 0-255
        uint8_t current_hue = current_hue_norm * 255.0f;

        // --- Animation logic based on parameters and time ---
        if (!is_enabled) {
            fill_solid(leds, CRGB::Black);
            return;
        }

        for (size_t i = 0; i < ledCount(); ++i) {
            // Example: Simple traveling sine wave
            uint8_t phase = tickCount() * 5 + i * 10;
            uint8_t val = sin8(phase * current_speed);
            leds[i] = CHSV(current_hue, 255, val);
        }
    }

    // Optional: Override reset() if custom logic needed when scene reactivates
    // void reset() override { 
    //     Scene::reset(); // Calls base reset (resets tick_count, parameters)
    //     // Add custom reset logic here
    // }
    
    // Optional: Provide a status string for debugging/logging
    // std::string status() const override {
    //     return "Mode: " + std::to_string(static_cast<int>(settings["mode"])) + "; Speed: " + std::to_string(static_cast<float>(settings["speed"]));
    // }
};

} // namespace Scenes
```

## Available API within Scene Subclass

### Lifecycle Methods (Override)

*   `virtual void setup()`: (Pure Virtual) Called once when the scene is added or reset. **Define metadata and parameters here.** Initialize internal state. This configures how the scene appears in UIs and what controls are available.
*   `virtual void tick()`: (Pure Virtual) Called every frame. **Implement animation logic here.** Access parameters via `settings` to make the animation react to runtime changes.
*   `virtual void reset()`: Optional override. Called when scene becomes active after being inactive. Default implementation resets `tick_count` and parameter values to their defaults. Call `Scene::reset()` if overriding.

### Metadata Definition (in `setup()`)

Define these in `setup()` to identify your scene. They are used by logging systems (`main.cpp`) and UIs (like the Web Simulator) to display information about the scene.

*   `set_name(const std::string& name)`
*   `set_description(const std::string& desc)`
*   `set_version(const std::string& ver)`
*   `set_author(const std::string& author)`
*   `meta(const std::string& key, const std::string& value)`: For custom key-value metadata.

### Parameter Definition & Access (in `setup()` / `tick()`)

Parameters define the user-controllable aspects of your scene. Define them in `setup()` using `param()`. The definitions (type, range, description) are used by UIs (like the Web Simulator) to automatically generate controls. Read the current values in `tick()` using `settings["param_name"]` to make your animation respond. These values can be changed *while the scene is running* via the UI or other control mechanisms.

*   `param(...)` (Protected method in `setup()`): Define parameters. Multiple overloads exist for different types (`ratio`, `count`, `range`, `switch`, `select`). The `description` argument is important as it often appears as a label or tooltip in UIs. See the example above and the detailed [Parameters Guide](Parameters.md) for signatures and options.
*   `settings["param_name"]` (`SettingsProxy` member, typically used in `tick()`): Access/modify current parameter values. Provides implicit type conversion.
    *   Example: `float speed = settings["speed"];`
    *   See [Parameters Guide](Parameters.md) for details on accessing metadata and iterating parameters.

### Metadata Accessors (Read-only)

*   `name()` (`const std::string&`)
*   `description()` (`const std::string&`)
*   `version()` (`const std::string&`)
*   `author()` (`const std::string&`)

### LED Access

*   `leds[index]` (`LedsProxy` member): Provides `CRGB&`. Bounds-clamped.
*   `led(index)` (`CRGB&` method): Helper access. Bounds-clamped.
*   `ledCount()` (`size_t` method): Returns total number of LEDs.

### Model Geometry Access

*   `model()` (`const IModel&` method): Returns reference to the model interface.
*   `model().point(index)` (`const Point&`): Get point data for LED `index`. Bounds-clamped.
*   `model().face(index)` (`const Face&`): Get face data for face `index`. Bounds-clamped.
*   `model().pointCount()` (`size_t`): Total number of points (usually == `ledCount()`).
*   `model().faceCount()` (`size_t`): Total number of faces.

### Timing Utilities

*   `millis()` (`uint32_t`): Milliseconds since program start.
*   `deltaTime()` (`float`): Time elapsed since the last frame (in seconds).
*   `tick_count()` (`size_t`): Number of `tick()` calls since the scene was last activated/reset.

### Math/Random Utilities

*   `random8()`
*   `random16()`
*   `random(max)`
*   `random(min, max)`
*   `randomFloat()` (0.0-1.0)
*   `randomFloat(max)` (0.0-max)
*   `randomFloat(min, max)`
*   *(Global utilities like `map`, `nblend`, `fadeToBlackBy`, `CHSV`, `fill_solid`, etc., are available via `PixelTheater.h` and `using namespace PixelTheater;`. See `Color.md` and `creating_animations.md`.)*
*   *(Constants like `PT_PI`, `PT_TWO_PI` are available via the `PixelTheater::Constants` namespace. Use them directly (e.g., `PixelTheater::Constants::PT_PI`) or bring them into scope locally (e.g., `using PixelTheater::Constants::PT_PI;` within a function).)*
*   *(Note: Using certain library utilities might require ensuring their corresponding `.cpp` source file is included in platform-specific build scripts, like `build_web.sh`.)*

### Other Utility Methods

*   `virtual std::string status() const`: Optional override. Intended to return a concise string representing the current internal state of the scene for debugging or detailed logging. (Note: Currently not used by the default logging in `main.cpp`.)
*   `logInfo(const char* format, ...)`: Log informational messages.
*   `logWarning(const char* format, ...)`: Log warning messages.
*   `logError(const char* format, ...)`: Log error messages.
*   *(Note: Logging requires a `LogProvider` to be configured in the `Platform`. Format string support is basic; avoid complex specifiers.)*

Refer to the source code headers (`scene.h`, `imodel.h`, `platform.h`, `parameters.h`, etc.) for precise signatures and implementation details.
