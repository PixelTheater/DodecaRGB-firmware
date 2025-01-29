# PixelTheater Animation System

## Overview

The animation system provides a type-safe, flexible framework for creating LED animations on three-dimensional objects. Whether you're building a dodecahedron, sphere, cube, or any other LED-covered shape, this library makes it easy to:

- Create modular, reusable animations (scenes)
- Define configurable parameters for each animation
- Switch between animations smoothly
- Define animation parameters and presets and apply them quickly
- Integrate with sensors and user input
- Debug and monitor animation performance

Each animation exists as a "scene" that can define its own behavior, parameters, and interaction with the LED array. User code is called as frequently as possible (50fps+) and can update all the leds at once. The system handles all the complexity of managing animations, transitioning between them, and providing a consistent interface for development.

The library was designed to work with Arduino frameworks and FastLED, but it could be adapted to other use cases.

## Concepts and Nomenclature

- PixelStage: The virtual spherical display where the scene is rendered
- Director: Manages the animation system, including scene selection and transition logic
- Show: A list of scenes to play
- Scene: A single animation, including its parameters and behavior
- Controls: A set of controls that can be used to interact with the scene
- Presets: A snapshot of settings for the controls, props and scenes
- Props: Chunks of data like color palettes, bitmaps, geometry, etc. that are used by the scene
- Actors: Animation objects (classes) used in a scene
- Controllers: An external interface to drive scene controls in real time
- Settings: The configuration of controls, props and presets that can be used to configure the scene

The PixelStage abstracts an addressable LED string into a 3D spherical display.

## Architecture

```text
┌──────────┐                          
│ Director │                          
└┬─────────┘                          
 │  ┌────────┐                        
 ├─▶│ Show   │                        
 │  └┬───────┘                        
 │   │  ┌────────┐┌────────┐┌────────┐
 │   └─▶│Scene 1 ││Scene 2 ││Scene N │
 │      └┬───────┘└────────┘└────────┘
 │       │  ┌─────────────┐  ┌───────┐
 │       ├─▶│Settings     │◀─┤Presets│
 │       │  └─────────────┘  └───────┘
 │       │  ┌────────┐   ┌───────────┐
 │       └─▶│Controls│◀──┤Controllers│
 │          └────────┘   └───────────┘
┌┴───────┐    ╔════════════════╗      
│ Stage  │───▶║ current scene  ║      
└┬───────┘    ╚════════════════╝      
 │  ┌─────────────┐                   
 └─▶│PixelSurface │                   
    └┬────────────┘                   
     │  ┌────────────┐                
     ├─▶│ LEDSurface │                
     │  └────────────┘                
     │  ┌────────────┐                
     └─▶│ HWDevices  │                
        └────────────┘                             
```

## Design Specification

### Scene Design

A Scene defines an animation written in C++ that runs on the teensy and controls the animation. Scenes are called frequently (50fps+) to update the LEDs based on their parameters and internal state.

#### Scene Structure

Each scene consists of:

1. A C++ class implementing behavior
2. Parameters that control the animation
3. Optional binary assets (props)

#### Scene Organization

Each scene lives in its own directory:

```bash
/scenes/space/               # Scene root directory
  space.cpp                  # Scene implementation
  space.yaml                 # Parameter configuration
  README.md                  # Scene documentation
  props/                     # Scene-specific assets
    nebula.bmp              # Bitmap resource
    deep_space.pal          # Palette resource
```

> _Why use parameters instead of just setting variables in scene code?_
>
> Parameters allow for more flexibility and easier configuration. They can be used to define **presets**, which are sets of parameter values that can be applied to a scene. They can define constraints on the parameter values, such as a **range** or a step size. They can be configured in **YAML files**, which are easier to maintain than C++ code. Parameters define an interface contract between the scene and the Director, which opens the door to more complex behaviors and interactions.

#### Scene Configuration

Parameters can be defined in two ways:

```cpp
// 1. YAML Configuration (default)
// Parameters defined in space.yaml, loaded at build time
class SpaceScene : public Scene {
    void setup() override {
        // Initialize scene state only
        _particles.resize(100);
        calculate_initial_positions();
    }
    
    void tick() override {
        float speed = settings("speed");
        update_particles(speed);
    }
};

// 2. Manual Configuration
// All parameters must be defined in config()
class ManualScene : public Scene {
    void config() override {
        // Define all parameters
        param("speed", SignedRatio, 0.0f, Clamp);
        param("brightness", Ratio, 0.8f);
        
        // Define presets
        preset("black_hole", {
            {"speed", -0.5f},
            {"brightness", 0.2f}
        });
    }
};
```

Key points:

- setup() is always available for scene initialization
- If config() is defined, YAML configuration is disabled
- Parameters defined in YAML are validated at compile time
- Parameter access is type-safe and checked at compile time

#### YAML Configuration

Scenes are configured in YAML files that define parameters, presets, and resources. The configuration is converted to C++ code at build time.

```yaml
parameters:
  speed:
    type: SignedRatio    # Built-in type (-1..1)
    default: 0.0
    flags: [clamp, slew] # Optional behaviors
    description: "Animation speed"

  brightness:
    type: Ratio         # Built-in type (0..1)
    default: 0.8
    flags: [clamp]
    description: "Overall brightness"

  palette:
    type: Palette      # Resource type
    default: "rainbow" # Built-in palette name

presets:
  black_hole:
    speed: -0.5
    palette: "deep_space"

  hubble:
    speed: 0.5
    palette: "galaxy"
```

Props can be defined at two levels:

```yaml
# In scene.yaml - scene-specific props
props:
  nebula:
    file: props/nebula.bmp    # Relative to scene directory
    format: RGB565

# In props.yaml - global props available to all scenes
props:
  palettes:
    rainbow:
      file: props/rainbow.pal
    deep_space:
      file: props/deep_space.pal
```

#### Built-in Parameter Types

Standard parameter types with their ranges:

```cpp
// Value Types
Ratio       // float 0..1 (intensities, sizes)
SignedRatio // float -1..1 (speeds, factors)
Angle       // float 0..PI (rotations)
SignedAngle // float -PI..PI (relative angles)
Count       // int 0..max (quantities)
Range       // float/int min..max (custom ranges)
Switch      // bool false/true (toggles)
Select      // int with named values (choices)

// Resource Types
Palette     // string (name of palette resource)
Bitmap      // string (name of bitmap resource)
```

Ranges have a minimum and maximum value, and a default value. If no default value is provided, the range's minimum value is used (or 0.0 for signed ranges).

Switch parameters are simple boolean toggles that default to false. Select parameters map named values to integers, useful for choosing between different modes or behaviors:

```yaml
parameters:
  auto_rotate:  # Switch example
    type: Switch
    default: false
    description: "Enable auto-rotation"

  chaos:        # Select example
    type: Select
    values:     # Maps names to values
      - none      # Position 0 = no particles
      - mild      # Position 1 = few particles
      - wild      # Position 2 = many particles
    default: none
    description: "Chaos level affects particle behavior"

  direction:    # Select with explicit values
    type: Select
    values:
      clockwise: 1     # Explicit value mapping
      counter: -1
      random: 0
    default: clockwise
    description: "Rotation direction and speed"
```

Manual configuration in scene code:

```cpp
void config() override {
    // Switch - simple boolean toggle
    param("auto_rotate", Switch, false);  // name, type, default

    // Select - sequential positions (0,1,2)
    param("chaos", Select, {
        "none",     // Position 0
        "mild",     // Position 1
        "wild"      // Position 2
    }, "none");    // default selection

    // Select - with explicit values
    param("direction", Select, {
        {"clockwise", 1},   // Name maps to value
        {"counter", -1},
        {"random", 0}
    }, "clockwise");       // default selection
}
```

Usage in scene code:

```cpp
void tick() override {
    // Switch usage
    if (settings<bool>("auto_rotate")) {
        // Auto-rotation enabled
    }

    // Select usage - get position (0,1,2)
    int chaos_level = settings<int>("chaos");
    
    // Select usage - get mapped value (-1,0,1)
    int direction = settings<int>("direction");
}
```

Parameter flags modify behavior:

```cpp
Clamp      // Values limited to range
Wrap       // Values wrap around range
Slew       // Smooth transitions between values
```

#### Scene Lifecycle

A Scene implements these key methods:

```cpp
void setup() override {
    // Initialize scene state
    _particles.resize(100);
    calculate_initial_positions();
}

void tick() override {
    // Update animation state
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

### Directing Scenes

The Director is responsible for selecting and transitioning between scenes. It can place animations on the stage (run them), and manage playlists and activate presets. The Director puts on the show.

### Props System

Props are binary assets (palettes, bitmaps) that can be:

- Scene-specific: `/scenes/<scene>/props/`
- Global: defined in props.yaml

### LED Coordinates

LED positions are calculated from PCB pick-and-place data:

1. Each PCB face has 104 LEDs in a pentagon shape
2. 12 faces form a dodecahedron ~13cm in diameter
3. Coordinates are generated in several formats:
   - Cartesian (x,y,z)
   - Spherical (radius, theta, phi)
   - Face/index pairs

The build process:

1. Reads pick-and-place CSV files
2. Applies face rotations and transformations
3. Generates C++ point data
4. Creates neighbor lookup tables

### Build System

The project uses PlatformIO to manage builds, dependencies and testing. The build process includes code generation steps that convert scene configurations and prop data into C++ code.

### Environments

- **teensy41**: Builds firmware with Arduino framework, generates LED points
- **native**: Builds test suite with doctest, generates LED points and test fixtures

### Code Generation

The build process generates C++ code from:

1. **Scene Parameters**:

Scene YAML files are converted to C++ parameter definitions:

```cpp
// Parameter definitions - one line per param for easy diffing
constexpr ParamDef SPACE_SCENE_PARAMS[] = {
    {"speed",      PARAM_SIGNED_RATIO, -1.0f, 1.0f,  0.5f,  PARAM_CLAMP},
    {"brightness", PARAM_RATIO,         0.0f, 1.0f,  0.8f,  PARAM_WRAP},
    {"angle",      PARAM_RANGE,        -PI,   PI,    0.0f,  PARAM_WRAP | PARAM_SLEW},
    {"palette",    PARAM_PALETTE,       0,     0,    0,     PARAM_NONE, "rainbow"},
};
```

2. **LED Point Data**:

- Processes PCB pick-and-place files for LED positions
- Generates coordinate data and neighbor tables
- Creates visualization data for testing

### Testing

Tests are organized into two environments:

```bash
# Run native tests
pio test -e native

# Run specific test file
pio test -e native -f test_parameters
```

Test fixtures are generated from scene YAML files to enable testing with real scene configurations:

```cpp
// Generated fixture provides scene parameters
struct SpaceSceneFixture {
    PixelTheater::SpaceSceneParameters params;
};

// Use fixture in tests
TEST_CASE_FIXTURE(SpaceSceneFixture, "Scene parameters work") {
    CHECK(params.speed.get() == 0.5f);
    CHECK(params.brightness.set(0.8f) == true);
}
```

The doctest framework and PlatformIO's toolchains are used for testing. The native test environment only tests the library code, not the hardware. That means the arduino framework and FastLED are mocked out. 

```
// Generated palette_data.h
namespace PixelTheater {
    // Each palette is a separate const struct
    constexpr struct {
        const uint8_t data[12] = {
            0,   255, 0,   0,    // red
            128, 0,   255, 0,    // green
            255, 0,   0,   255   // blue
        };
    } PALETTE_RAINBOW;

    // Simple lookup returns pointer to palette data
    const uint8_t* get_palette(const char* name);
} 
```
