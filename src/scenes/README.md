# DodecaRGB Animation System

## Overview

The animation system provides a type-safe, flexible framework for creating LED animations on three-dimensional objects. Whether you're building a dodecahedron, sphere, cube, or any other LED-covered shape, this library makes it easy to:

- Create modular, reusable animations (scenes)
- Define configurable parameters for each animation
- Switch between animations smoothly
- Define preset configurations and apply them quickly
- Integrate with sensors and user input
- Debug and monitor animation performance

Each animation exists as a "scene" that can define its own behavior, parameters, and interaction with the LED array. User code is called as frequently as possible (50fps+) and can update all the leds at once. The system handles all the complexity of managing animations, transitioning between them, and providing a consistent interface for development.

The library was designed to work with Arduino frameworks and FastLED, but it could be adapted to other use cases.

## Core Concepts

### Scenes, Settings, Parameters and Presets

The DodecaRGB animation system uses three key components to manage LED animations:

- **Scenes** define animations and their configurable parameters
- **Parameters** specify what values can be changed and their valid ranges
- **Settings** store the current state of these parameters
- **Presets** are named collections of settings that can be quickly applied

### Ownership and Responsibility

The class ownership and hierarchy:

1. **Scene** owns its Settings instance
   - The Parameters for Settings are defined in the scene's setup() method
   - Settings values can be changed externally by AnimationManager, and are read-only from inside the scene

2. **Settings** contain the values for all defined parameters in a scene
   - Owns parameter definitions
   - Provides value storage, retrieval, and final validation

3. **ParamBuilder** is a setup() helper
   - Provides the param() builder interface for defining parameters in the scene's setup() method
   - Created by Settings to define a single parameter
   - No direct ownership of data
   - Validates parameter configuration
   - Returns control to Settings when done

   - Example:
  
     ```cpp
     void MyScene::setup() {
         param("speed")
             .range(Ranges::SignedRatio)
             .set(0.0f);
             
         param("active")
             .boolean()
             .set(true);
     }

4. **Settings Access**

Settings are read-only from inside animations:

```cpp
     void MyScene::tick() {
         float speed = settings("speed");         // Read current speed
         const auto& palette = settings<CRGBPalette16>("colors");  // Get custom type
     }
```

Only AnimationManager can modify parameters externally:

```cpp
     animations["pattern"].set("speed", 0.5f);    // Change speed
     animations["pattern"].preset("fast");         // Apply preset
```

5. **Presets**

Presets are named collections of settings that can be quickly applied, they are like a "snapshot" of settings values. More detials below.

### Animation Manager

The **Animation Manager** is the main class that manages all animations. It provides methods to add, remove, and switch between animations, as well as to control playback and apply presets.

## Parameters

Parameters help define what settings can be changed for a scene. There are several built-in types:

- Built-in ranges (`Ratio`, `SignedRatio`, `Percent`, `Angle`)
- Custom numeric ranges (e.g. `-2.0 to 15.0`, `PI to 4*PI`)
- Boolean values
- Instance types (like `CRGBPalette16`)

Parameters preserve their defined type when accessed. For example, if a parameter is defined as a boolean,
it will be returned as a bool when accessed, without needing explicit casting.

### Parameter Types

#### Built-in Ranges

- `Ratio` - 0.0 to 1.0 (intensities, sizes, blending factors)
- `SignedRatio` - -1.0 to 1.0 (speeds, directions)
- `Percent` - 0 to 100 (percentage values)
- `Angle` - 0 to TWO_PI (rotations)
- `SignedAngle` - -PI to PI (relative angles)

All numeric parameters are automatically clamped to their defined ranges:

```cpp
// In setup():
param("size").range(0, 100).set(150);  // Clamped to 100

// Through AnimationManager:
animations["rollerball"].set("size", -10);  // Clamped to 0
animations["rollerball"].set("unit_velocity", 2.0f); // Clamped to 1.0 (SignedRatio)
```

Note: Clamping ensures values always stay within their defined ranges, both during setup and runtime.

#### Custom Types

The parameter system supports custom types using the `.as<T>()` syntax:

```cpp
// In Scene::setup():
param("colors").as<CRGBPalette16>().set(RainbowColors_p);

// In Scene::tick():
const auto& palette = settings<CRGBPalette16>("colors");
```

### Defining a Scene

The scene lifecycle is:

1. Scene is constructed and added to AnimationManager
2. `setup()` is called once to:
    - Define parameters using param()
    - Define presets using preset()
    Note: setup() must be implemented but can be empty if no parameters are needed
3. `reset()` is called to initialize state:
    - Get current settings values
    - Reset animation state
4. Scene begins running:
    - `tick()` is called each frame
    - `status()` reports current state
    - when settings change, the scene is notified via `onSettingsChanged()`

By default, `onSettingsChanged()` calls reset(), but scenes can override this method to handle changes differently. For example a scene could read new settings, contiue the animation, and transition to new values over time.

Note: `reset()` can be called to update the scene with new settings and restart the animation.

```cpp
class MyScene : public Scene {
protected:
    void setup() override {
        // One-time configuration

        // define parameters
        param("speed")
            .range(Ranges::SignedRatio)
            .set(0.0f);
        param("size").range(5, 30).set(10);
        param("colors").as<CRGBPalette16>().set(RainbowColors_p);
        
        // define any presets
        preset("fast")
            .set("speed", 1.0f)
            .set("size", 5);
    }

    void reset() override {
        // Update local variables with current settings
        _speed = settings("speed");
        _size = settings("size");
        _palette = settings<CRGBPalette16>("colors");
        
        // Reset animation state
        _particles.clear();
    }

    void tick() override {
        // Use current settings to update animation
        updateParticles(_speed, _size);
        drawParticles(_palette);
    }
    
    void status() override {
        output.printf("%d particles", _particles.size());
    }

private:
    // Cached settings
    float _speed;
    int _size;
    CRGBPalette16 _palette;
    
    // Animation state
    std::vector<Particle> _particles;
};
```

### Defining Parameters

Parameters are defined in the scene's `setup()` method.

Some ParameterExamples:

```cpp
    // Common ranges using semantic names
    param("speed").range(SignedRatio).set(0.0f);    // -1.0 to 1.0
    param("size").range(Ratio).set(0.5f);          // 0.0 to 1.0
    param("rotation").range(Angle).set(0.0f);      // 0.0 to 2*PI
    param("angle").range(SignedAngle).set(0.0f);   // -PI to PI
    param("brightness").range(Percent).set(50);     // 0 to 100
    
    // Custom ranges
    param("spawn-chance").range(-5.0f, 5.0f).set(0.0f);
    param("num_circles").range(1, 100).set(50);
    param("random-chaos").range(0.0f, 1.0f).randomize();  // Random initial value
    
    // Other types
    param("jedi-colors").as<CRGBPalette16>().set(SpaceColors_p);
    param("enabled").boolean().set(true);
```

### Using Settings

```cpp
// Changing settings through AnimationManager (in main.cpp):
animations["pattern"].set("speed", 0.5f);    // Change speed
animations["pattern"].set("colors", RainbowColors_p);  // Change palette

// In reset() (in scene.cpp):
void reset() override {
    _palette = settings("colors");  // Type deduced automatically
    _speed = settings("speed");
    // Reset animation state
    _particles.clear();
}

// in onSettingsChanged() (in scene.cpp):
void onSettingsChanged() override {
    _speed = settings("speed");    // respond only to speed changes
    _particle_speed_target = _speed;
}

// Not allowed - settings are read-only inside scenes
void tick() override {
    settings.set("speed", 0.5f);  // Error: no set() access
}
```

### Defining Presets

Presets allow you to define named collections of settings within an animation, which can then be applied at runtime:

```cpp
// in myScene.cpp:
void setup() override {
    // ...
    preset("black-hole")
        .set("speed", -0.5f)
        .set("size", 1.0f)
        .set("colors", SpaceColors_p);
}

// Apply preset
animations["custom"].preset("black-hole");

// Apply preset then change specific values
animations["custom"].preset("black-hole");
animations["custom"].set("speed", 0.1f);
```

## Using AnimationManager

### setup() in main.cpp

```cpp
// Create the animation manager
AnimationManager animations;  // Uses default time provider

// Add new animations with identifying names
animations.add<MyAnimation>("basic");     // default settings
animations.add<MyAnimation>("custom")     // custom settings
    .set("speed", 0.5f)
    .set("colors", OceanColors_p);

// Access animations by their assigned names
animations["custom"]
    .set("speed", 0.8f)
    .set("colors", RainbowColors_p);

// Remove animations

animations.clear();          // Removes all animations
```

### Runtime Control

Many animations can be added to the AnimationManager at runtime, and can be switched between, advanced, or randomized.

```cpp
// Switch between animations
animations.play("black-hole");  // Switch to specific animation
animations.next();             // Advance to next in sequence
animations.random();           // Switch to random animation

// Control playback mode
animations.setPlaybackMode(PlaybackMode::HOLD);      // Stay on current
animations.setPlaybackMode(PlaybackMode::ADVANCE, 15.0f);  // Auto-advance every 15s
animations.setPlaybackMode(PlaybackMode::RANDOM, 30.0f);   // Random every 30s

// Safety
// All operations are safe even with no animations:
animations.clear();      // Remove all
animations.next();       // Safe when empty
animations.random();     // Safe when empty
animations.play("any");   // Throws std::invalid_argument for unknown animations
animations.remove("basic");  // Safely removes single animation
```

### Status Reporting

The animation system provides status reporting through the AnimationManager, which can be configured to periodically output diagnostic information to the serial terminal:

```cpp
// In main.cpp
void loop() {
    animations.update();  // Update current animation

    // Check and display status (optional)
    if (animations.hasStatus()) {
        Serial.println(animations.status());  // Gets status from current animation
    }
}
```

The status output includes:

- Global information (FPS, brightness, current animation)
- Animation-specific status from the current animation

### Configuring Status Reports

```cpp
// Configure status reporting interval
animations.setStatusInterval(1000);  // Status every 1 second
animations.setStatusInterval(0);     // Disable status reporting
```

### Animation Status Implementation

Scenes can override status() to provide custom information using an Arduino-like output helper:

```cpp
class MyScene : public Scene {
protected:
    void status() override {
        output.printf("Speed: %.2f\n", _speed);
        output.print("Color: ");
        output.print(getAnsiColorString(_primary_color));
        output.println();
    }
};
```

The `output` object provides familiar Arduino-style methods:

- print() - For strings, chars, numbers
- println() - Print with newline
- printf() - Formatted output

AnimationManager will collect and format the status output from the current scene.

### Color Formatting Helpers

The animation system provides helper functions for formatting colors in status messages:

```cpp
// In animation status methods:
std::string ansi = getAnsiColorString(CRGB::Blue);    // ANSI escape codes (256 color), displays a colored space
std::string name = getClosestColorName(my_color);     // Find nearest named color
```

These helpers are available in the `color_helpers.h` utility library.

Be sure to call `animations.update()` in your loop to handle time-based transitions.

```cpp
void loop() {
    // Update current animation and handle playlist transitions
    animations.update();  // Calls current animation's tick() method
    // ... rest of loop
}
```

## Ideas for the future

```cpp
// Future: Animate settings over time
animations["custom"].settings
    .animate("speed")
    .from(0.0f).to(1.0f)
    .over(5000);  // 5 seconds


// Future: add modular controllers and modifiers
animations.controllers.add<LFOController>("slow-sine")
    .range(SignedRatio)
    .freq(0.04f)
    .default(0.0f); // 4% per second
animations.controllers.add<RandomController>("random-walk")
    .range(0, 100)
    .freq(2); // 2x per second

// Use these two time-based controllers to change settings over time:
auto& dest = animations["custom"];
dest.setting("speed").addController("slow-sine");
dest.setting("z-height").addController("random-walk");
```

### Error Handling

AnimationManager provides safe access to animations and their settings:

```cpp
try {
    // Access non-existent animation
    animations["unknown"].set("speed", 0.5f);  // Throws std::invalid_argument
    
    // Access non-existent parameter
    animations["pattern"].set("unknown", 0.5f);  // Throws std::invalid_argument
    
    // Invalid parameter value
    animations["pattern"].set("size", -1.0f);   // Throws std::invalid_argument (out of range)
    
} catch (const std::invalid_argument& e) {
    Serial.printf("Error: %s\n", e.what());
}

// Safe checks
if (animations.hasAnimation("pattern")) {
    animations["pattern"].set("speed", 0.5f);
}

// Safe operations (no exceptions)
animations.clear();      // Safe when empty
animations.next();       // Safe when empty
animations.random();     // Safe when empty

// Preset safety
if (animations["pattern"].hasPreset("fast")) {
    animations["pattern"].preset("fast");
}
```

Note: Inside Scene methods (tick, reset, etc), settings access is always safe as parameters are validated during setup.

