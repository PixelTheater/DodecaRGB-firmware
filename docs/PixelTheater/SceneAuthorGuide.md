# PixelTheater Scene Author Guide

This guide explains how to create new animation scenes for the PixelTheater library using the refactored API (featuring the `Theater` facade and non-templated `Scene` base class).

## 1. Introduction: What is a Scene?

A Scene represents a single, self-contained animation or visual effect. It manages its own state, parameters, and logic for calculating LED colors based on the model geometry and time.

Scenes are managed and run by the main `Theater` class.

## 2. Creating Your Scene File

Create a new header file for your scene, typically within the `src/scenes/your_scene_name/your_scene_name.h`.

## 3. Basic Scene Structure

Your scene class must inherit publicly from `PixelTheater::Scene`.

```cpp
#pragma once

#include "PixelTheater.h" // Include the main library header

// Optional: Bring PixelTheater namespaces into scope
using namespace PixelTheater;
using namespace PixelTheater::Constants; // For PT_PI, PT_TWO_PI etc.

namespace Scenes { // Recommended namespace for scenes

class MyScene : public Scene {
public:
    // --- Required Methods --- 
    
    MyScene() = default;
    ~MyScene() override = default; // Virtual destructor

    // Called once before the scene runs for the first time, 
    // or after reset() is called.
    // Use this to set metadata, define parameters, and init state.
    void setup() override;

    // Called repeatedly for each animation frame.
    // Implement your animation logic here.
    void tick() override;

    // --- Optional Methods --- 

    // Called when switching back to this scene after it has run before.
    // Default resets tick_count and parameters.
    // void reset() override { Scene::reset(); /* + custom reset */ }
    
    // If parameters are complex and setup() becomes cluttered,
    // you can define them in config() instead.
    // void config() override { param(...); }

private:
    // --- Member Variables --- 
    float _phase = 0.0f;
    // ... other state variables ...
};

} // namespace Scenes
```

*   Include `PixelTheater.h`.
*   Use a namespace (e.g., `Scenes`).
*   Inherit publicly from `PixelTheater::Scene`.
*   Override the pure virtual `setup()` method.
*   Override the `tick()` method for animation logic.

## 4. Setup: Metadata and Parameters

Use the `setup()` method for initialization:

```cpp
void MyScene::setup() {
    // 1. Set Metadata (Name is important for UI/logging)
    set_name("My Awesome Scene");
    set_description("Description of what it does.");
    set_version("1.0");
    set_author("Your Name");
    // Alternatively, use meta("key", "value");
    // meta("Source", "URL");

    // 2. Define Parameters
    param("speed", "ratio", 0.5f, "clamp", "Animation speed (0-1)");
    param("intensity", "range", 0.1f, 1.0f, 0.8f, "", "Effect intensity");
    param("color_hue", "count", 0, 255, 128, "wrap", "Base color hue (0-255)"); // Use count for byte range
    param("enabled", "switch", true, "", "Enable/disable feature");

    // 3. Initialize Scene State (if needed)
    _phase = randomFloat(0.0f, PT_TWO_PI); 
}
```

*   **Metadata:** Use `set_name()`, `set_description()`, etc., or the generic `meta()`.
*   **Parameters:** Use the `param()` helper. See `Parameters.md` for types and flags.

## 5. Tick: Animation Logic

Implement your animation logic in `tick()`. Access LEDs, geometry, parameters, and utilities via the `Scene` base class helpers:

```cpp
void MyScene::tick() {
    Scene::tick(); // Call base class to increment tick_count()

    // --- Get Parameter Values --- 
    float speed = settings["speed"];
    int color_hue = settings["color_hue"]; // Automatically converts from count param
    if (!settings["enabled"]) { // Check boolean switch
        // Fade out quickly if disabled
        for(size_t i=0; i < ledCount(); ++i) leds[i].fadeToBlackBy(64);
        return; // Skip rest of animation
    }

    // --- Access Time & Randomness --- 
    uint32_t time_ms = millis();
    float dt = deltaTime(); // Time since last frame
    _phase += speed * dt * 5.0f; // Update phase based on speed and delta time
    if (_phase > PT_TWO_PI) _phase -= PT_TWO_PI;

    // --- Access Geometry & LEDs --- 
    const IModel& m = model(); // Get model reference
    size_t count = ledCount();

    for (size_t i = 0; i < count; ++i) {
        const Point& p = m.point(i); // Get geometry for LED i
        
        // Example: Calculate brightness based on point height and phase
        float height_factor = map(p.y(), -1.0f, 1.0f, 0.0f, 1.0f); // Assumes map is available via namespace
        uint8_t brightness = sin8( static_cast<uint8_t>((height_factor * 128) + (_phase * 255 / PT_TWO_PI)) ); 

        // Set LED color using leds[] proxy
        leds[i] = CHSV(color_hue, 200, brightness); // Assumes CHSV is available
    }
}
```

## 6. API Helper Reference

These methods are available within your Scene class:

**LEDs:**
*   `leds[index]` (`LedsProxy`): Access `CRGB&` (bounds-clamped).
*   `led(index)` (`CRGB&`): Helper function access (bounds-clamped).
*   `ledCount()` (`size_t`): Total number of LEDs.

**Model/Geometry:**
*   `model()` (`const IModel&`): Get reference to model interface.
*   `model().point(index)` (`const Point&`): Get point data (bounds-clamped).
*   `model().face(index)` (`const Face&`): Get face data (bounds-clamped).
*   `model().pointCount()` (`size_t`): Number of points (should == ledCount).
*   `model().faceCount()` (`size_t`): Number of faces.

**Parameters/Settings:**
*   `param(...)`: Define parameters in `setup()`.
*   `meta(key, value)`: Define simple metadata in `setup()`.
*   `settings["name"]`: Access/modify parameter values (returns proxy).
*   `name()`, `description()`, `version()`, `author()`: Get scene metadata.
*   `get_parameter...()`, `has_parameter()`: Introspection methods.

**Utilities:**
*   `millis()`, `deltaTime()`, `tickCount()`: Timing.
*   `random8()`, `random16()`, `random()`, `randomFloat()`: Random numbers.
*   `logInfo()`, `logWarning()`, `logError()`: Logging (takes `const char*`).

Refer to the header files (`scene.h`, `imodel.h`, etc.) for exact signatures.

## 7. Adding Your Scene

Include your scene's header in `src/main.cpp` and add it to the `Theater` instance:

```cpp
// src/main.cpp
#include "PixelTheater.h"
#include "models/MyModel/model.h" // Your model
#include "scenes/my_scene/my_scene.h" // Your new scene

extern PixelTheater::Theater theater;

void setup() {
    // ... 
    theater.useFastLEDPlatform<PixelTheater::Models::MyModel>(...);
    theater.addScene<Scenes::MyScene>(); // Add your scene
    theater.start();
}