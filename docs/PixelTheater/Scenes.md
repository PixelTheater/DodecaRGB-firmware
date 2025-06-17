---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Scene API Reference

This document provides a quick reference to the API available to authors creating custom animations by inheriting from `PixelTheater::Scene`.

- For a tutorial on creating scenes, see the [Creating Animations Guide](../../docs/creating_animations.md).
- For details on Parameters, see the [Parameters Guide](Parameters.md).
- For Color/Palette details, see [Color System](Color.md) and [Palettes API](Palettes.md).
- For Model/Geometry details, see [Model System](Model.md).
- For Easing function details, see [Easing Functions](Easing.md).

## Basic Structure

All scenes must inherit from the `Scene` base class provided by `PixelTheater/SceneKit.h` and implement the `setup()` and `tick()` methods.

```cpp
#pragma once

// Include SceneKit for core types and helpers
#include "PixelTheater/SceneKit.h"

namespace Scenes {

// Note: Inherit PUBLICLY from the aliased `Scene` type
class MyMinimalScene : public Scene {
public:
    MyMinimalScene() = default;
    ~MyMinimalScene() override = default;

    void setup() override {
        set_name("Minimal Example");
        param("example_param", "ratio", 0.5f);
    }

    void tick() override {
        Scene::tick(); // Recommended base call
        // Basic animation logic...
        float val = settings["example_param"];
        if (ledCount() > 0) {
            leds[0] = CHSV(0, 0, static_cast<uint8_t>(val * 255));
        }
    }
};

} // namespace Scenes
```

## Lifecycle Methods

Override these virtual methods to control scene behavior:

*   `void setup()`: **(Required)** Called once when the scene is added or reset. Define metadata and parameters here. Initialize internal state.
*   `void tick()`: **(Required)** Called every frame. Implement animation logic here.
*   `void reset()`: **(Optional)** Called when scene becomes active after being inactive. Default resets `tick_count` and parameters. Call `Scene::reset()` if overriding.
*   `void config()`: **(Optional)** Alternative place to define parameters using `param(...)` if `setup()` is complex. Called after the constructor.
*   `virtual ~Scene()`: **(Required override, usually `= default`)** Ensure proper cleanup.

## Metadata Definition (in `setup()` or `config()`)

*   `set_name(const std::string& name)`
*   `set_description(const std::string& desc)`
*   `set_version(const std::string& ver)`
*   `set_author(const std::string& author)`
*   `meta(const std::string& key, const std::string& value)`: Custom key-value metadata.

## Parameter Definition & Access

Define user-controllable parameters in `setup()` or `config()`.

*   `param(...)`: Define parameters (various overloads).
*   `settings["param_name"]`: Access/modify current parameter values.

*Note: See the [Parameters Guide](Parameters.md) for full details on types, flags, and usage.*

## Metadata Accessors (Read-only)

*   `name()` (`const std::string&`)
*   `description()` (`const std::string&`)
*   `version()` (`const std::string&`)
*   `author()` (`const std::string&`)
*   `get_meta(const std::string& key) const`

## Core Accessors

### LED Access

*   `leds[index]` (`LedsProxy` member): Access `CRGB&` for an LED. Index is bounds-clamped.
*   `led(index)` (`CRGB&` method): Alternative helper access. Index is bounds-clamped.
*   `ledCount()` (`size_t` method): Returns total number of LEDs.

### Model Geometry Access

*   `model()` (`const IModel&` method): Returns reference to the model interface.
*   `model().point(index)` (`const Point&`): Get point data for LED `index`. Index is bounds-clamped. The returned `Point` object has methods like `x()`, `y()`, `z()`, `id()`, `distanceTo(otherPoint)`, `isNeighbor(otherPoint)`, and `getNeighbors()` (returns a `const std::array<Point::Neighbor, MAX_NEIGHBORS>&`). Each `Point::Neighbor` struct contains the neighbor point `id` and `distance`.
*   `model().face(index)` (`const Face&`): Get face data for face `index`. Index is bounds-clamped.
*   `model().pointCount()` (`size_t`): Total number of points (usually == `ledCount()`).
*   `model().faceCount()` (`size_t`): Total number of faces.
*   `model().getSphereRadius()` (`float`): Returns the calculated radius of the model's bounding sphere.

### Edge Connectivity Access

Edge access uses face-centric methods to avoid confusion in models with multiple face types:

```cpp
// Get a face proxy for face-centric operations
auto face_proxy = model().face(face_id);

// Get connected face at a specific edge of this face
int8_t connected_face = face_proxy.face_at_edge(edge_index);  // Returns -1 if no connection

// Get number of edges for this face  
uint8_t edge_count = face_proxy.edge_count();

// Iterate through all edges of this face
auto edges = face_proxy.edges();
for (const auto& edge : edges) {
    if (edge.has_connection()) {
        int8_t adjacent_face = edge.connected_face_id;
        // Process edge connection...
    }
}
```

Note: Each edge connects exactly one face to another (or has no connection). Face-centric design ensures predictable behavior in models with multiple face types.

## Core Utility Methods (Base Class Members)

### Timing Utilities

*   `millis()` (`uint32_t`): Milliseconds since program start.
*   `deltaTime()` (`float`): Time elapsed since the last frame (seconds). Essential for frame-rate independence.
*   `tick_count()` (`size_t`): Number of `tick()` calls since last activation/reset. Incremented by `Scene::tick()`.

### Random Number Utilities

*   `random8()` / `random8(max)` / `random8(min, max)`
*   `random16()` / `random16(max)` / `random16(min, max)`
*   `random()` / `random(max)` / `random(min, max)` (`uint32_t`)
*   `randomFloat()` / `randomFloat(max)` / `randomFloat(min, max)`

### Logging Utilities

Requires a `LogProvider` configured in the `Platform`. Basic `printf`-style formatting.

*   `logInfo(const char* format, ...)`
*   `logWarning(const char* format, ...)`
*   `logError(const char* format, ...)`

## SceneKit Aliased Utilities (Non-Member Helpers)

These common functions are brought into the `Scenes` namespace by `SceneKit.h` for convenient use without qualification:

*   **Mapping:** `map()` (Arduino-style for int & float).
*   **Color/Palette:**
    *   `colorFromPalette()`: Sample colors from `CRGBPalette16` palettes.
    *   `lerp8by8()`: Fast 8-bit linear interpolation between two `uint8_t` values.
    *   *Note: For cross-platform compatibility (hardware and web simulator), use palette constants from the `PixelTheater::Palettes` namespace (e.g., `PixelTheater::Palettes::PartyColors`) rather than hardware-specific PROGMEM variables (like `PartyColors_p`).*
*   **Blending/Fading:** `fadeToBlackBy()`, `nblend()`, `blend8()`, `blend()`.
*   **Easing Functions:** `linearF`, `inSineF`, `outSineF`, `inOutSineF`, `inQuadF`, `outQuadF`, `inOutQuadF`, and their interpolating counterparts (`linear`, `inSine`, etc.). See the [Easing Functions Guide](Easing.md) for details.

*(Note: Check `SceneKit.h` for the full list of aliases. Functions or types not explicitly aliased require the `PixelTheater::` namespace qualifier, e.g., `PixelTheater::sin8()`, `PixelTheater::cos8()`. Other potentially useful qualified functions include `PixelTheater::qadd8()` and `PixelTheater::qsub8()` for 8-bit saturated arithmetic.)*

## Vector Math (Eigen)

PixelTheater typically includes the Eigen library (via `<PixelTheater/core/math.h>`), making types like `Eigen::Vector3f` and `Eigen::Matrix3f` available for advanced 3D calculations (position vectors, velocity, rotation matrices). Refer to [Eigen documentation](https://eigen.tuxfamily.org/dox/group__QuickRefPage.html) for usage details or check out this [helpful tutorial](https://github.com/brown-cs-224/Eigen-Tutorial).

## Other Utility Methods

*   `virtual std::string status() const`: **(Optional)** Override to return a concise string representing the scene's internal state for debugging/logging.
