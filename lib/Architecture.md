# Animation Library

Core library for creating and managing LED animations for the DodecaRGB project.

## Architecture

The library uses the `Animation` namespace to organize all components:

### Core Classes

- `Scene` - Base class for all animations
  - Defines interface for parameters, animation loop, and optional features
  - Each scene has its own display and settings instance
  - Pure virtual methods: `defineParams()`, `tick()`

- `Display` - Interface for LED control and geometry
  - Handles pixel colors and 3D coordinates
  - Implementations:
    - `HardwareDisplay` - Controls physical LEDs via FastLED
    - `MockDisplay` - For testing without hardware
  - Includes palette management for colors

- `Settings` - Parameter management and persistence
  - Manages parameters, ranges, and presets
  - Provides type-safe parameter access
  - Handles parameter validation and serialization

- `AnimationManager` - Orchestrates scenes
  - Manages scene lifecycle and transitions
  - Handles playback modes (hold, advance, random)
  - Provides scene access by name

### Support Classes

- `Point` - 3D geometry primitives
  - Represents LED positions in space
  - Provides coordinate transformations

- `Param` - Parameter definition system
  - Defines valid ranges and types
  - Supports numeric and instance parameters

- `Preset` - Parameter preset system
  - Stores and loads parameter configurations
  - Supports both numeric and instance values

### File Organization

```txt
lib/Animation/
├── include/           # Public headers
│   ├── scene.h       
│   ├── display.h     
│   ├── settings.h    
│   └── animation_manager.h
├── src/              # Implementation files
│   ├── scene.cpp     
│   ├── display.cpp   
│   ├── settings.cpp  
│   └── param.cpp     
...
/include        # Project-specific implementation of core classes
/src            # Project firmware using Animation library
└── scenes/     # Project-specific animations
...
/test           # All tests following PlatformIO conventions
├── helpers/    # Test helpers and mocks
└── test_*.cpp  # Test files
/util           # Utilities for generating and simulating animations (python, not part of the library)
```

For best practices and where to put files and tests, see [PlatformIO Project Structure](https://docs.platformio.org/en/latest/advanced/unit-testing/structure/hierarchy.html)

## Testing

We are using [doctest in PlatformIO](https://docs.platformio.org/en/latest/advanced/unit-testing/frameworks/doctest.html#) for unit testing. We will follow the advice an best practices from the PlatformIO documentation.

In general we follow these principles:
- Tests should be as independent as possible, tessting their primpary concerns and dependencies.
- We want to avoid cross-dependencies between tests that complicate the test setup.
- When fixing a broken test, also check that the test assumptions are correct, by checking design specifications

## Usage

### Scene Interface

Scenes inherit from the Scene base class and must implement:

- `defineParams()` - Declare parameters and their ranges
- `tick()` - Update animation state each frame

Optional methods:

- `setup()` - Called during initialization
- `definePresets()` - Define parameter presets
- `status()` - Return current state as string

Each scene has access to:

- `settings()` - Parameter management
- `_display` - LED control (managed by AnimationManager)

The AnimationManager handles scene registration and playback control.

### User Animations

User animations should be placed in the `scenes/` directory. Each animation should be a separate file, and should include the following:

- `#include "scene.h"`
- Implement the `Scene` interface
- Define parameters using `Param`
- Implement `tick()` to update the animation state

If there are additional classes needed for an animation, they should also be placed in the `scenes/` directory and should be included in the animation file.

## Namespace Organization

All components use the `Animation` namespace:

```cpp
namespace Animation {
    class Scene { ... }
    class Display { ... }
    // etc.
}
```

User animations should also be placed within the Animation namespace:

```cpp
// in src/scenes/my_scene.h
namespace Animation {
    class MyScene : public Scene {
        // Implementation
    };
}
```

## Hardware Abstraction

The animation system is designed to be hardware-independent through a layered abstraction:

### Core Types

```cpp
// Basic RGB color type (matches FastLED's CRGB)
struct RGB {
    uint8_t r, g, b;
};

// Physical LED point with neighbors
struct LED_Point : public Point {
    std::vector<uint16_t> neighbors;  // Indices of neighboring LEDs
    uint8_t side;                     // Which face this LED belongs to
};

// Complete hardware configuration
struct HardwareConfig {
    RGB* leds;                  // LED array
    const LED_Point* points;    // Physical layout
    size_t num_leds;           // Total number of LEDs
    uint8_t num_sides;         // Number of faces
    uint16_t leds_per_side;    // LEDs per face
};
```

### Testing Without Hardware

For testing, use the base AnimationManager which doesn't require hardware:

```cpp
// In tests:
AnimationManager animations;  // No hardware needed
animations.add<MyScene>("test");
animations.update();  // Safe to call without hardware
```

### Using with Real Hardware

For actual hardware, use HardwareAnimationManager with FastLED:

```cpp
// In main.cc:
#include <FastLED.h>
#include "hardware_animation_manager.h"

CRGB leds[NUM_LEDS];
LED_Point points[NUM_LEDS];  // Defined in points.cpp

void setup() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    
    // Create hardware configuration
    HardwareConfig config {
        .leds = leds,
        .points = points,
        .num_leds = NUM_LEDS,
        .num_sides = 12,
        .leds_per_side = NUM_LEDS/12
    };
    
    // Create manager with hardware support
    HardwareAnimationManager animations(config);
    
    // Add animations as normal
    animations.add<MyScene>("basic");
}

void loop() {
    animations.update();  // Updates both animation and LEDs
    FastLED.show();
}
```

### Hardware Display

The HardwareDisplay class bridges between animations and physical LEDs:

```cpp
class HardwareDisplay : public Display {
public:
    explicit HardwareDisplay(const HardwareConfig& config);
    
    // Core interface used by animations
    void setPixel(int i, RGB c) override;
    RGB getPixel(int i) const override;
    const Point& getPoint(int i) const override;
    
    // Hardware-specific info
    size_t numSides() const;
    size_t ledsPerSide() const;
};
```

This layered approach allows:
- Testing without hardware dependencies
- Clean separation between animation logic and hardware control
- Easy adaptation to different LED hardware
- Hardware-specific optimizations in HardwareDisplay