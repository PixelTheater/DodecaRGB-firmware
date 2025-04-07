---
category: Development
generated: 2025-02-10 00:32
version: 2.8.2
---

# PixelTheater Animation System

## Overview

The PixelTheater library provides a flexible framework for creating LED animations (Scenes) on three-dimensional objects. It simplifies hardware interaction and scene management through a central `Theater` facade.

Key features:
- Define complex 3D models from configuration files.
- Create modular, reusable animation Scenes inheriting from `PixelTheater::Scene`.
- Access LEDs and geometry easily via Scene helper methods (`leds[i]`, `model().point(i)`).
- Use common utilities (`millis()`, `random8()`, etc.) directly within Scenes.
- Configure scenes with runtime parameters.
- Integrate with different hardware platforms (FastLED, native testing) via the `Theater`.

### Architecture

```text
                           ┌───────────┐
                           │ User Code │
                           │ (main.cpp)│
                           └─────┬─────┘
                                 │ Uses
                                 ▼
                           ┌───────────┐
                           │  Theater  │ (Facade)
                           └─────┬─────┘
 Manages / Provides Access To    │
       ┌─────────────────────────┼──────────────────────────┐
       │                         │                          │
       ▼                         ▼                          ▼
┌──────────────┐      ┌───────────────────┐       ┌────────────────────┐
│   Platform   │      │ IModel/ILedBuffer │       │ Scene N            │
│ (Native/Hdw) │◀─────│    (Interfaces)   │◀──────│ (Your Animation)   │
└──────────────┘      └───────────────────┘       └────────────────────┘
       ▲                         ▲                          ▲
       │ Implemented By          │ Implemented By           │ Inherits From
       │                         │                          │
┌──────┴─────────┐  ┌───────────┴──────────┐      ┌───────┴──────┐
│ NativePlatform │  │ ModelWrapper         │      │ Scene (Base) │
│ FastLEDPlatform│  │ LedBufferWrapper     │      └──────────────┘
└────────────────┘  └──────────────────────┘             │
                                                           │ Provides Helpers
                                                           │ (leds[], model(),
                                                           │  millis(), etc.)
```

### Key Concepts 

- **Theater**: The main entry point. Initializes the system (`useNativePlatform`, `useFastLEDPlatform`), adds scenes (`addScene`), and runs the animation loop (`update`).
- **Scene**: Base class for all animations. Provides helpers to access LEDs (`leds[i]`), geometry (`model().point(i)`), utilities (`millis()`), and parameters (`settings[]`). You inherit from this to create your animation.
- **Platform**: Abstract base class for hardware/environment interaction (e.g., `NativePlatform`, `FastLEDPlatform`). Managed by `Theater`.
- **IModel/ILedBuffer**: Interfaces providing access to model geometry and LED data buffers. Managed by `Theater`, accessed via `Scene` helpers.
- **ModelWrapper/LedBufferWrapper**: Internal classes implementing the interfaces, wrapping the concrete `Model` and LED buffer. Managed by `Theater`.
- **Model**: Defines the LED geometry (points, faces). Generated from config files.
- **Parameters/Settings**: Mechanism for runtime configuration of scenes.

## Getting Started

1.  **Include Header:** Add `#include "PixelTheater.h"` to your main file (`src/main.cpp`).
2.  **Define Model:** Ensure your model definition header (e.g., `models/MyModel/model.h`) exists.
3.  **Create Theater:** Instantiate `PixelTheater::Theater theater;` globally.
4.  **Initialize Theater:** In `setup()`, call the appropriate method, e.g.:
    ```cpp
    // For Teensy/FastLED:
    #include "models/DodecaRGBv2/model.h" // Include your specific model
    extern ::CRGB leds[]; // Assuming global FastLED array
    extern const size_t NUM_LEDS;
    theater.useFastLEDPlatform<PixelTheater::Models::DodecaRGBv2>(leds, NUM_LEDS);

    // For Native testing:
    // #include "fixtures/models/basic_pentagon_model.h"
    // theater.useNativePlatform<PixelTheater::Fixtures::BasicPentagonModel>(/*led count*/);
    ```
5.  **Create Scenes:** Define classes inheriting from `PixelTheater::Scene` in separate header files (e.g., `src/scenes/my_scene.h`). Implement `setup()` and `tick()`.
6.  **Add Scenes:** In `setup()`, include your scene headers and add instances to the theater:
    ```cpp
    #include "scenes/my_scene.h"
    #include "scenes/another_scene.h"
    // ... 
    theater.addScene<Scenes::MyScene>(); 
    theater.addScene<Scenes::AnotherScene>();
    ```
7.  **Start Theater:** Call `theater.start();` after adding scenes.
8.  **Update Loop:** In `loop()`, call `theater.update();`.

See `creating_animations.md` and `SceneAuthorGuide.md` for more details.

## Parameter System

Parameters allow scenes to be configured at runtime. They are defined in the scene's `setup()` method using `param()`:

```cpp
// Inside MyScene::setup()
param("speed", "ratio", 0.5f); // Float 0.0-1.0
param("count", "count", 1, 10, 5); // Integer 1-10, default 5
```

Access values in `tick()` using the `settings` proxy:

```cpp
float speed = settings["speed"];
int count = settings["count"];
```

See `Parameters.md` for full details.

## Advanced Configuration & Features

*   **Models:** Define custom LED geometry. See `Model.md`.
*   **Palettes:** Use predefined or custom color palettes. See `Palettes.md`.
*   **Build System:** Understand how models and code are compiled. See `build-system.md`.
*   **Logging:** Use `logInfo()`, `logWarning()`, `logError()` within scenes.
*   **Utilities:** Leverage math functions (`map`, `sin8`, `blend8`), color functions (`hsv2rgb_rainbow`), and constants (`Constants::PT_PI`) provided via `PixelTheater.h`.

### Build Process

The build system compiles scenes and models into the firmware:

1. Each scene is compiled as a separate class
2. Models are generated from their definitions into C++ header files
3. The firmware links everything together at compile time

### Customization

PixelTheater can be customized in several ways:

1. Creating new scenes
2. Defining new models
3. Extending the core library with new features

See the individual documentation pages for more details on each aspect of the system.

### [14] Palettes

Palettes define color schemes that can be used in animations. See [Palettes.md](Palettes.md)
for detailed documentation on:

- Available built-in palettes
- Creating custom palettes
- Using palettes in animations
- Memory and performance considerations

### Bitmaps

Bitmap resources can be used for textures, masks, or lookup tables:

- Supported formats: 8-bit grayscale, 24-bit RGB
- Images are converted to binary data at build time
- Access via resource manager to save RAM
- Consider memory limits when using large images
