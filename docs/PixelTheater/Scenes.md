---
author: Jeremy Seitz - somebox.com
generated: 2025-02-04 19:17
project: DodecaRGB Firmware
repository: https://github.com/somebox/DodecaRGB-firmware
title: '[4] Scenes'
version: 2.8.0
---

<div style="display: flex; justify-content: space-between; align-items: center;">
            <div>
                <p style="font-size: 1.0em; color: #888;">Documentation for <a href="https://github.com/somebox/DodecaRGB-firmware">DodecaRGB Firmware</a></p>
            </div>
            <div style="text-align: right; font-size: 0.7em; color: #888;">
                <p>Version 2.8.0<br/>
                Generated: 2025-02-04 19:17</p>
            </div>
          </div>

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

Each scene implements these lifecycle methods that are called by the Director:

```cpp
void setup() override {
    // Initialize scene state
    // Called once when scene becomes active
    _particles.resize(100);
    calculate_initial_positions();
}

void tick() override {
    // Update animation state
    // Called every frame (50fps+)
    float speed = settings("speed");
    update_particles(speed);
}

void status() override {
    // Optional: Report scene state
} 

void reset() override {
    // Optional: Reset to initial state
}
```

The Director manages scene transitions and ensures proper lifecycle method calls.


### [4.4] Settings

#### Using Parameters in Code

Parameters are accessed in your scene code using the `settings[]` helper:

```cpp
// Get settings via proxy objects
auto speed = settings["speed"];         // Returns a proxy object
float speed_val = float(speed);         // Convert to float [-1.0, 1.0]
speed = 0.5f;                          // Assign new value

// Type-safe conversions
float f = float(settings["ratio_param"]);
int i = int(settings["count_param"]);
bool b = bool(settings["switch_param"]);
```

The settings[] helper:

- Provides type-safe access to parameters
- Handles range validation
- Applies parameter flags (clamp, wrap)
- Returns default values if not set
- Returns a proxy object that provides type-safe access
- Handles parameter validation and type conversion
- Provides access to parameter metadata and flags
- Manages parameter lifetime and access

You can inspect parameter configuration at runtime for use in animations:

```cpp
// Get parameter metadata
auto speed = settings["speed"];
float max_speed = speed.max();
float default_speed = speed.default_value();
const char* desc = speed.description();
bool has_clamp = speed.has_flag(Flags::CLAMP);

// For select/enum parameters
auto pattern = settings["pattern"];
const ParamDef& meta = pattern.metadata();  // Access full parameter definition
```

### [4.5] Example Scene

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
            p.velocity = speed;
        }
    }
    
    void tick() override {

    }
};
```