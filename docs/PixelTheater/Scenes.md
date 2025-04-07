---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Scene API Reference

This document provides a quick reference to the API available to authors creating custom animations by inheriting from `PixelTheater::Scene`.

For a tutorial on creating scenes, see `docs/creating_animations.md`.
For a more detailed guide, see `docs/PixelTheater/SceneAuthorGuide.md`.

## Basic Structure

```cpp
#include "PixelTheater.h"
using namespace PixelTheater;
using namespace PixelTheater::Constants;

namespace Scenes {
class MyScene : public Scene {
public:
    void setup() override { /* Init metadata, params, state */ }
    void tick() override { /* Animation logic */ }
};
} // namespace Scenes
```

## Available API within Scene Subclass

### Lifecycle Methods (Override)

*   `virtual void setup()`: (Pure Virtual) Initialize parameters, metadata, state.
*   `virtual void tick()`: Implement frame-by-frame animation logic. Call `Scene::tick()` to increment base counter.
*   `virtual void reset()`: Optional override. Called when scene becomes active again. Default resets `tick_count` and parameters.

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

### Parameters & Settings

*   `param(...)` (Protected method): Define parameters in `setup()`. Multiple overloads exist.
*   `meta(key, value)` (Protected method): Define simple string metadata in `setup()`.
*   `settings["name"]` (`SettingsProxy` member): Access/modify parameter values.

### Metadata Accessors

*   `name()` (`const std::string&`): Get scene name.
*   `description()` (`const std::string&`): Get scene description.
*   `version()` (`const std::string&`): Get scene version.
*   `author()` (`const std::string&`): Get scene author.

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
*   *(Global utilities like `map`, `nblend`, `fadeToBlackBy`, `CHSV`, etc., are available via `PixelTheater.h` and `using namespace PixelTheater;`)*
*   *(Constants like `PT_PI`, `PT_TWO_PI` are available via `PixelTheater.h` and `using namespace PixelTheater::Constants;`)*

### Logging Utilities

*   `logInfo(const char* format)`
*   `logWarning(const char* format)`
*   `logError(const char* format)`
*   *(Also `const` versions available)*
*   *(Note: Currently only supports simple format string, no variable arguments)*

Refer to the source code headers (`scene.h`, `imodel.h`, `platform.h`, etc.) for precise signatures and implementation details.
