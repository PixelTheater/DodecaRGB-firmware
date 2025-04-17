---
category: Development
generated: 2025-02-10 00:32
version: 2.8.2
---

# PixelTheater Animation System

![PixelTheater Logo](../../images/pixeltheater-logo.png)

## Overview

PixelTheater is a C++ library designed for creating interactive, 3D LED animations. It provides a platform-independent framework, allowing animations (Scenes) to run on different hardware (like Teensy/FastLED) or simulators (native C++, WebAssembly) with minimal code changes.

**Key Features:**

*   **Platform-Independent Scenes:** Write animation logic once using the `PixelTheater` API; run on multiple platforms.
*   **3D Model Abstraction:** Define complex LED geometry and access LED positions and relationships easily.
*   **Simplified Scene API:** Create modular `PixelTheater::Scene` classes with helpers for LEDs, geometry, time, parameters, and utilities.
*   **Color & Palette API:** Unified API (`PixelTheater::CRGB`, `PixelTheater::CHSV`, `PixelTheater::Palettes`, `PixelTheater::colorFromPalette`) for handling colors and palettes across platforms.
*   **Parameter System:** Define runtime-configurable parameters for scenes.
*   **Central `Theater` Facade:** Manages platform setup, scene lifecycles, and the main animation loop.

### Architecture

The `Theater` acts as a central coordinator, connecting the chosen hardware `Platform`, the 3D `Model` geometry, and the active animation `Scene`. Scenes interact with the system exclusively through the `PixelTheater::Scene` base class helpers and the `PixelTheater` namespace API.

```text
                           ┌───────────┐
                           │ User Code │ // e.g., main.cpp
                           │ (Setup)   │
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

### Core Concepts

*   **Theater**: The main entry point (`PixelTheater::Theater`). Initializes the platform and model, adds scenes, and runs the animation loop.
*   **Scene**: Base class (`PixelTheater::Scene`) for all animations. Implement `setup()` and `tick()`. Provides platform-independent helpers to access LEDs (`leds[]`), geometry (`model()`), time (`millis()`), parameters (`settings[]`), and utilities (`random8()`).
*   **Platform**: Abstraction layer for hardware/environment interaction (e.g., `FastLEDPlatform`, `NativePlatform`). Usually configured once via `Theater`.
*   **Model**: Defines the 3D LED geometry (`IModel` interface, accessed via `Scene::model()`). Generated from configuration files. See [Model System](Model.md).
*   **Parameters/Settings**: Mechanism for runtime configuration of scenes via `param()` and `settings[]`. See [Parameters](Parameters.md).
*   **Color API**: Platform-independent types (`PixelTheater::CRGB`, `PixelTheater::CHSV`) and functions (`PixelTheater::blend`, `PixelTheater::colorFromPalette`). See [Palettes API](Palettes.md).

## Getting Started (Scene Author Focus)

1.  **Include Header:** Add `#include "PixelTheater.h"` to your scene file. This typically includes everything needed for scene development.
2.  **Create Scene Class:** Define a class inheriting from `PixelTheater::Scene`.
    ```cpp
    #pragma once
    #include "PixelTheater.h"

    namespace Scenes { // Optional, but recommended
    class MyScene : public PixelTheater::Scene {
    public:
        MyScene() = default;
        ~MyScene() override = default;

        void setup() override;
        void tick() override;
    private:
        // Scene-specific state variables
    };
    } // namespace Scenes
    ```
3.  **Implement `setup()`:** Set metadata (`set_name`, etc.) and define parameters (`param`).
4.  **Implement `tick()`:** Write your animation logic using helpers like `leds[]`, `model()`, `millis()`, `settings[]`, `PixelTheater::colorFromPalette`, etc.
5.  **Register Scene:** In your main application file (`main.cpp`), include your scene's header and add it to the `Theater` instance after initializing the platform.
    ```cpp
    // --- main.cpp ---
    #include "PixelTheater.h"
    #include "scenes/my_scene.h" // Include your scene

    PixelTheater::Theater theater;

    void setup() {
        // Platform setup (specific to project/hardware)
        // theater.useFastLEDPlatform<...>(...);
        // theater.useNativePlatform<...>(...);

        // Add scenes
        theater.addScene<Scenes::MyScene>();
        // theater.addScene<...>();

        theater.start();
    }

    void loop() {
        theater.update();
    }
    ```

*   For detailed guides, see [Scene Author Guide](SceneAuthorGuide.md) and [Creating Animations Guide](../guides/creating_animations.md).

## Key Subsystems Documentation

*   **[Scene API](SceneAuthorGuide.md):** How to structure and write animation scenes.
*   **[Color & Palettes](Palettes.md):** Using `CRGB`/`CHSV`, predefined palettes, and `colorFromPalette`.
*   **[Parameters](Parameters.md):** Defining and using runtime parameters.
*   **[Model System](Model.md):** Accessing 3D geometry (`model()`, `Point`, `Face`).
*   **[Utilities](../guides/creating_animations.md#led-management):** Overview of helpers for time, random numbers, color math (`blend`, `nscale8`), etc. available in scenes.
*   **[Build System](build-system.md):** How code and models are compiled.
