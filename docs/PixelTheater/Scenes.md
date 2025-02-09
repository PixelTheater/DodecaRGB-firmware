---
category: Development
generated: 2025-02-10 00:32
version: 2.8.2
---

# [4] Scenes

Scenes are the main building blocks of PixelTheater. They define the animation, parameters, and other settings for a single animation.

## [4.1] Scenes and Animations

### What is a Scene?

A Scene defines an animation written in C++ that runs on the teensy display (the "stage"). Scenes are called frequently (50fps+) to update the LEDs based on their parameters and internal state.

- defines the classname (`FireworksScene`) and friendly name ("fireworks") of the animation
- will automatically load and include parameters from the generated `_params file
- provides foundation for Scene lifecycle methods (init, config, setup, tick, status, reset)
- provides helper methods from PixelTheater like `settings[]` and parameter reflection
- exposes access to LEDs (via FastLED) and hardware devices (buttons, sensors, accelerometer)

#### [4.2] Structure and Files

Each scene lives in its own directory:

```bash
scenes/space/              # Scene root directory
├── space.yaml             # Parameter configuration
├── space.cpp              # Scene implementation 
├── _params.h              # Generated parameters (auto-included)
├── README.md              # Scene documentation
└── props/                 # Scene-specific assets
    ├── nebula.bmp         # Bitmap resource
    └── deep_space.pal     # Palette resource
```

### [4.3] Scene Lifecycle

1. Constructor initializes basic state
2. If YAML-defined parameters exist (generated _params.h), they are loaded automatically
3. setup() is called to initialize the scene
   - For manually defined parameters, they must be defined here using param()
   - After parameters are defined, scene can initialize using their values
4. tick() is called every frame to update the scene
5. status() may be called to report scene state

### [4.4] Parameter Definition

Parameters can be defined in two ways:

1. YAML Definition (Recommended):

```yaml
# space.yaml
parameters:
  speed:
    type: ratio
    default: 0.5
    description: "Controls animation speed"
    flags: [clamp]
```

2. Manual Definition:

```cpp
class TestScene : public Scene {
    void setup() override {
        // Define parameters first
        param("speed", "ratio", 0.5f, "clamp");
        param("brightness", "ratio", 0.8f, "wrap");
        
        // Now safe to use parameters
        float speed = settings["speed"];
        initialize_with_speed(speed);
    }
};
```

The Director manages scene transitions and ensures proper lifecycle method calls.

### [4.3] Accessing Scene Information

The following methods are available to all scenes:

- `name()`: Returns the name of the scene
- `description()`: Returns the description of the scene
- `status()`: Returns a string describing the scene's current state
- `tick_count()`: Returns the number of times the scene has been ticked
- `settings[]`: Returns a proxy object for accessing parameters

### [4.5] Parameter System Architecture

Parameters in PixelTheater use a three-layer system for type safety and validation:

1. **ParamValue**: Type-safe value container
   - Handles type conversion and validation
   - Supports float, int, and bool values
   - Throws if incorrect type accessed

   ```cpp
   ParamValue speed(0.5f);  // Creates float parameter
   float f = speed.as_float();  // Safe
   int i = speed.as_int();   // Throws bad_cast
   ```

2. **Settings**: Parameter Storage
   - Stores parameter definitions and current values
   - Validates values against parameter definitions
   - Manages parameter lifecycle
  
   ```cpp
   // In scene setup
   settings.add_parameter(PARAM_RATIO("speed", 0.5f, Flags::CLAMP, "Animation speed"));
   
   // Later in tick()
   settings["speed"] = 0.75f;  // Valid, stored
   settings["speed"] = 1.5f;   // Throws (out of range)
   ```

3. **SettingsProxy**: User Interface
   - Provides clean syntax for parameter access
   - Handles type conversion automatically
   - Validates ranges and types
  
   ```cpp
   // Type-safe access
   float speed = settings["speed"];        // Automatic conversion
   settings["speed"] = 0.5f;               // Type checked
   bool enabled = settings["is_enabled"];   // Different type
   ```

### [4.6] Parameter Best Practices

- Use YAML for parameter definition when possible
- Access parameters through settings[] interface
- Let type system catch errors early:

  ```cpp
  float speed = settings["speed"];
  if (speed > 0.5f) { ... }
  ```

- Handle parameter changes appropriately:

  ```cpp
  void SpaceScene::tick() {
      float speed = settings["speed"];
      position += speed * delta_time;  // Use current value
  }
  ```

### [4.6] Example Scene

Here's a complete example showing how parameters and settings work together:

```yaml
# space.yaml
name: space
description: "A space-themed animation"

parameters:
  speed:
    type: signed_ratio
    default: 0.5
    description: "Controls particle motion"
  
  pattern:
    type: select
    values: ["orbit", "spiral", "chaos"]
    default: "orbit"
    description: "Particle motion pattern"
  
  palette:
    type: palette
    default: "deep_space"
    description: "Color theme"
```

```cpp
// space.cpp
class SpaceScene : public Scene {
private:
    
public:
    void setup() override {
        // Initialize with parameter values
        _particles.resize(100);
        
        for(auto& p : _particles) {
            p.velocity = settings["speed"];
        }
    }
    
    void tick() override {
        // Access parameters via settings proxy
        float speed = settings["speed"];
        bool active = settings["enabled"];
        
        // Update animation state
        update_particles(speed);
    }
};
```

### [4.7] Understanding the call stack

What Happens when accessing and setting values in a scene's parameters:

```txt
# Defining a parameter in setup()

Scene::setup() 
  -> Scene::param("speed", "ratio", 0.5f, "clamp")
    -> Scene::param(name, type, ParamValue(0.5f), flags)  // Convenience overload
      -> Settings::add_parameter_from_strings(name, type, default_val, flags)
        -> ParamDef def = create_param_def(name, type, default_val, flags)
        -> Settings::add_parameter(def)
          -> validate_definition()  // Throws if default value invalid
          -> _values[name] = def.get_default_as_param_value()

# Setting a parameter in a scene method

Scene code: settings["speed"] = 1.2f
  -> SettingsProxy::operator[]("speed")
    -> returns Parameter("speed")
      -> Parameter::operator=(1.2f)
        -> Settings::set_value("speed", ParamValue(1.2f))
          -> ParamDef::apply_flags(value)  // Applies CLAMP/WRAP
          -> _values[name] = transformed_value

# Accessing a parameter in tick()

Scene code: float current = settings["speed"]
  -> SettingsProxy::operator[]("speed")
    -> returns Parameter("speed")
      -> Parameter::operator float()
        -> Settings::get_value("speed")
        -> value.as_float()
```

### [4.8] Parameter Flags

Parameters can have flags that modify their behavior:

- `clamp`: Clamps values between 0 and 1
- `wrap`: Wraps values around the range

The are applied to the parameter value when it is set.

```cpp
settings["some_ratio"] = 1.2f;  // Clamped to 1.0
settings["some_signed_ratio"] = -1.5f; // Clamped to -1.0
settings["some_angle_wrapped"] = 7.3f;  // Wrapped to 7.3 - 2PI
```
