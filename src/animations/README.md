# DodecaRGB Animation System

## Overview

The animation system provides a type-safe, flexible way to create and manage LED animations. Each animation can define parameters that control its behavior, store those parameters in settings, and save collections of settings as presets.

## Core Concepts

### Parameters, Settings, and Presets

The DodecaRGB animation system uses three key components to manage LED animations: Parameters, Settings, and Presets. 

- **Parameters** define the interface for each animation by specifying which values can be changed, their valid ranges, and ensuring type safety through validation. 
- **Settings** maintain the current state of these parameters, providing type-safe access and automatic range validation to ensure values remain valid. 
- **Presets** act as named collections of settings that can be saved and restored, allowing quick application of predefined configurations which can also be partially overridden with custom values.

### Animation Manager

The **Animation Manager** is the main class that manages all animations. It provides methods to add, remove, and switch between animations, as well as to control playback and apply presets.

## Parameters

Built-in Parameter Types:

- Built-in ranges (`Ratio`, `SignedRatio`, `Percent`, `Angle`)
- Custom numeric ranges (e.g. `-2.0 to 15.0`, `PI to 4*PI`)
- Boolean values
- Instance types (like `CRGBPalette16`)

### Parameter Types

#### Built-in Ranges

- `Ratio` - 0.0 to 1.0 (intensities, sizes, blending factors)
- `SignedRatio` - -1.0 to 1.0 (speeds, directions)
- `Percent` - 0 to 100 (percentage values)
- `Angle` - 0 to TWO_PI (rotations)
- `SignedAngle` - -PI to PI (relative angles)

#### Custom Types

The parameter system supports custom types using the `.as<T>()` syntax:

```cpp
// In defineParams():
param("colors").as<CRGBPalette16>().with(SpaceColors_p);

// In tick():
const CRGBPalette16& palette = settings["colors"];  // Type deduced automatically
CRGB color = ColorFromPalette(palette, index, brightness);
```

## Creating Animations

### Basic Structure

Animations define their parameters in the `defineParams()` method. 

```cpp
class MyAnimation : public Animation {
protected:
    void defineParams() override {
        // Common ranges using semantic names
        param("speed").range(SignedRatio).set(0.0f);    // -1.0 to 1.0
        param("size").range(Ratio).set(0.5f);          // 0.0 to 1.0
        param("angle").range(SignedAngle).set(0.0f);   // -PI to PI
        param("brightness").range(Percent).set(50);     // 0 to 100
        
        // Custom ranges
        param("spawn-chance").range(-5.0f, 5.0f).set(0.0f);
        param("num_circles").range(1, 100).set(50);
        param("random-chaos").range(0.0f, 1.0f).randomize();  // Random initial value
        
        // Other types
        param("jedi-colors").as<CRGBPalette16>().set(SpaceColors_p);
        param("enabled").boolean().set(true);
    }

    void tick() override {
        // Access settings using subscript operator
        float speed = settings["speed"];
        int count = settings["count"];
        const CRGBPalette16& palette = settings["colors"];  // Type deduced from parameter
        bool enabled = settings["enabled"];
        
        // Use the values to update the animation...
    }
};
```

## Using AnimationManager

### Basic Usage

```cpp
// Create the animation manager
AnimationManager animations(leds, NUM_LEDS, NUM_SIDES);  // accepts FastLED array

// Add new animations (still uses .add since it's creation)
animations.add<MyAnimation>("basic");
animations.add<MyAnimation>("custom")
    .set("speed", 0.5f)
    .set("colors", OceanColors_p);

// Access and modify existing animations
animations["custom"]
    .set("speed", 0.8f)
    .set("colors", RainbowColors_p);
```

### Presets

Presets allow you to define named collections of settings within an animation, which can then be applied at runtime:

```cpp
// in myAnimation.cpp:
void definePresets() override {
    preset("black-hole")
        .set("speed", -0.5f)
        .set("size", 1.0f)
        .set("colors", SpaceColors_p);
}

// Apply preset
animations["custom"].applyPreset("black-hole");

// Apply preset and override some values
animations["custom"]
    .applyPreset("black-hole")
    .set("speed", 0.1f);
```

### Runtime Control

Many animations can be added to the AnimationManager at runtime, and can be switched between, advanced, or randomized.

```cpp
// Switch between animations
animations.play("black-hole");
animations.next();
animations.random();

// Control playback mode
animations.setPlaybackMode(PlaybackMode::HOLD);      // Stay on current
animations.setPlaybackMode(PlaybackMode::ADVANCE, 15.0f);  // Auto-advance every 15s
animations.setPlaybackMode(PlaybackMode::RANDOM, 30.0f);   // Random every 30s
```

## Key Features

1. Type-safe parameter definitions with built-in ranges
2. Simple access to settings using subscript operator
3. Support for custom types
4. Preset system for storing and applying configurations
5. Runtime parameter validation

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
