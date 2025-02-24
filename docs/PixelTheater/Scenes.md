---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# [4] Scenes

Scenes are the main building blocks of PixelTheater. They define the animation, parameters, and other settings for a single animation.

## [4.1] Creating Scenes

Scenes are created through the Stage's type-safe creation API:

```cpp
// In main.cpp or setup code
auto* scene = stage->addScene<MyScene<ModelDef>>(*stage);
stage->setScene(scene);
```

### What is a Scene?

A Scene defines an animation that runs on a Stage with a specific Model. Scenes are called frequently (50fps+) to update LEDs based on their parameters and internal state.

Example Scene:
```cpp
template<typename ModelDef>
class SpaceScene : public Scene<ModelDef> {
    void setup() override {
        // Define parameters
        param("speed", "ratio", 0.5f, "clamp");
    }
    
    void tick() override {
        Scene<ModelDef>::tick();  // Call base to increment counter
        
        // Get parameters
        float speed = settings["speed"];
        
        // Direct LED access
        this->stage.leds[0] = CRGB::Red;
        
        // Face-based access
        auto& face = this->stage.model.faces[0];
        face.leds[0] = CRGB::Blue;
        
        // FastLED operations
        fadeToBlackBy(this->stage.leds[1], 128);
        nscale8(this->stage.leds[2], 192);
        
        // Fill operations
        fill_solid(face.leds, CRGB::Green);
        fill_rainbow(face.leds, face.led_count(), 0, 32);
    }
};
```

### [4.2] LED Access Patterns

Scenes can access and modify LEDs in several ways:

1. Direct LED Array Access:
```cpp
// Single LED
this->stage.leds[0] = CRGB::Red;

// Range-based iteration
for(auto& led : this->stage.leds) {
    led = CRGB::Blue;
}
```

2. Model Face Access:
```cpp
// Single LED on face
this->stage.model.faces[0].leds[1] = CRGB::Green;

// All LEDs on face
for(auto& face : this->stage.model.faces) {
    for(auto& led : face.leds) {
        led = CRGB::Blue;
    }
}
```

3. FastLED Operations:
```cpp
// Color operations
fadeToBlackBy(led, 128);  // Fade to black by amount
nscale8(led, 192);       // Scale brightness
nblend(led1, led2, 128); // Blend between colors

// Fill operations
fill_solid(leds, CRGB::Red);
fill_rainbow(leds, num_leds, start_hue, delta_hue);
fill_gradient_RGB(leds, start_pos, start_color, end_pos, end_color);
```

4. Color Types:
```cpp
// RGB colors
CRGB color(255, 0, 0);     // Red
CRGB::Red                  // Predefined color
CRGB::Blue                 // Predefined color

// HSV colors
CHSV hsv(0, 255, 255);    // Red in HSV
hsv2rgb_rainbow(hsv, rgb); // Convert to RGB
```

Key Features:
- Automatic parameter management
- Safe LED access through Stage
- Model geometry helpers
- Platform-agnostic animation code

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

Parameters can be defined in three ways:

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
void setup() override {
    param("speed", "ratio", 0.5f, "clamp");
}
```

3. Inheritance from Base Scene:

```cpp
class BaseScene : public Scene {
    void setup() override {
        param("global_speed", "ratio", 1.0f);
    }
};

class DerivedScene : public BaseScene {
    void setup() override {
        BaseScene::setup();  // Inherit base parameters
        param("local_speed", "ratio", 0.5f);
    }
};
```

The Director manages scene transitions and ensures proper lifecycle method calls.

### [4.3] Accessing Scene Information

The following methods and properties are available to all scenes:

- `name()`: Returns the name of the scene
- `description()`: Returns the description of the scene
- `status()`: Returns a string describing the scene's current state
- `tick_count()`: Returns the number of times the scene has been ticked
- `settings[]`: Returns a proxy object (`SettingsProxy`) for accessing parameters. This provides a clean and type-safe way to get and set parameter values.

### [4.5] Parameter System Architecture

Parameters in PixelTheater use a four-layer architecture for type safety and validation:

1. **Scene Layer** (User Interface)
   - Provides clean parameter access via `settings[]`
   - Handles parameter definition in `setup()`
   - Manages parameter lifecycle

2. **Settings Layer** (Parameter Management)
   - Stores parameter definitions and values
   - Manages validation chains
   - Handles parameter inheritance

3. **Parameter Layer** (Type System)
   - `ParamDef`: Parameter definitions and metadata
   - `ParamValue`: Type-safe value container
   - Validation and conversion rules

4. **Handler Layer** (Core Logic)
   - Type validation and conversion
   - Range validation and operations (clamp/wrap)
   - Flag handling and validation
   - Error handling via sentinels

Example usage:

```cpp
void MyScene::setup() override {
    // Parameter definition
    param("speed", "ratio", 0.5f, "clamp");
    
    // Safe to use after definition
    float speed = settings["speed"];
    initialize(speed);
}

void MyScene::tick() override {
    // Type-safe access with validation
    float speed = settings["speed"];
    if (speed > 0.0f) {  // Sentinel values handled safely
        update(speed);
    }
}
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

### [4.9] Error Handling

The parameter system uses sentinel values instead of exceptions for error handling:

```cpp
// Reading an invalid parameter returns a sentinel
float speed = settings["nonexistent"];  // Returns 0.0f
int count = settings["bad_count"];      // Returns -1
bool flag = settings["missing"];        // Returns false

// Invalid assignments are logged and clamped/wrapped based on flags
settings["speed"] = 1.5f;  // With CLAMP: becomes 1.0f
                          // With WRAP: wraps to 0.5f
```

Validation occurs in this order:

1. Type validation
2. Range validation
3. Flag application

Each validation failure:

1. Logs a warning message
2. Returns an appropriate sentinel value
3. Maintains type safety
4. Allows scene code to recover
