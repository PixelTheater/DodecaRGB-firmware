# PixelTheater Animation System

## [1] Overview

The animation system provides a type-safe, flexible framework for creating LED animations on three-dimensional objects. Whether you're building a dodecahedron, sphere, cube, or any other LED-covered shape, this library makes it easy to:

- Create modular, reusable animations (scenes)
- Define configurable parameters for each animation
- Switch between animations smoothly
- Define animation parameters and presets
- Integrate with sensors and user input
- Debug and monitor animation performance

### [2] Architecture and Class Structure

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
 │       │  ┌─────────────┐  ┌───────┐  ┌───────┐
 │       ├─▶│Settings     │◀─┤Presets│◀─┤Actors │
 │       │  └─────────────┘  └───────┘  └───────┘
 │       │  ┌────────┐   ┌───────────┐
 │       └─▶│Controls│◀──┤Controllers│
 │          └────────┘   └───────────┘
┌┴───────┐    ╔════════════════╗      
│ Stage  │───▶║ current scene  ║      
└┬───────┘    ╚════════════════╝      
 │  ┌────────────────┐                   
 └─▶│ VenueDevice    │                   
    └┬───────────────┘                   
     │  ┌────────────┐                
     ├─▶│ LEDSurface │                
     │  └────────────┘                
     │  ┌────────────┐                
     └─▶│ HWDevices  │                
        └────────────┘                             
```

### [2.1] Key Concepts

- **Stage**: The virtual spherical display where the scene is rendered
- **Director**: Manages the animation system, including scene selection and transitions
- **Show**: A list of scenes to play
- **Scene**: A single animation, including its parameters and behavior
- **Controls**: A set of controls that can be used to interact with the scene
- **Presets**: A snapshot of settings for the controls, props and scenes
- **Props**: Chunks of data like color palettes, bitmaps, geometry used by the scene
- **Actors**: Animation objects (classes) used in a scene
- **Controllers**: An external interface to drive scene controls in real time
- **Settings**: The configuration of controls, props and presets

### [3] Parameters

#### [3.1] What are Parameters For?

Parameters allow for more flexibility and easier configuration. They:

- Define an interface contract between the scene and the Director
- Enable preset support for saving and loading configurations
- Provide type safety and range validation
- Allow remote control and real-time adjustment
- Make scenes more reusable and configurable

> Parameters define constraints and behaviors that help prevent errors and
> make animations more robust. They also enable features like presets and
> remote control that would be difficult to implement with raw variables.

#### [3.2] Defining Parameters

Parameters are defined in YAML files, which provide a clear and maintainable way to
configure your scene. The YAML is converted to C++ code during build.

```yaml
# Scene Header
name: Space Animation
description: "A smooth particle-based space animation with configurable patterns"

# Parameters
parameters:
  speed:
    type: ratio
    description: "Controls animation speed"
    default: 0.5
    flags: [clamp]

  brightness:
    type: ratio
    default: 0.8
    flags: [wrap]
    description: Overall LED brightness

  pattern:
    type: select
    values: ["sphere", "fountain", "cascade"]
    default: "sphere"
```

### [3.3] Parameter Types and Ranges

Parameters serve as the foundation of our animation control system, providing a bridge between the raw code and user-friendly controls. They're designed to be both intuitive for creators and safe for runtime execution.

**Core Concept**: At their heart, parameters are strongly-typed values that control how animations behave. While internally they use enums for type safety (ParamType and ParamFlag), you'll work with more semantic, human-readable types in your configurations.

> 💡 Important: When writing YAML or using config(), always use lowercase with underscores (like signed_ratio). The system will automatically convert these to the appropriate internal enum types.

Scene parameters allow runtime control of animations. They define the interface to your scene
that allows it to be customized and controlled.

Parameters can be defined in two ways:

1. Using YAML files (recommended)
2. Using manual code in config() method

Important: You can only use one of these methods. If config() is overridden,
YAML parameter definitions will be ignored to prevent confusion about parameter source.

**What Parameters Provide**:

Think of parameters as smart variables that understand their purpose. They know their valid ranges, how to handle edge cases, and how to communicate with the broader animation system. They ensure type safety, manage value constraints, and enable features like animation presets and remote control capabilities.

> 🎨 For example, a "brightness" parameter knows it should only accept values between 0 and 1, and can automatically handle cases where values might try to exceed these bounds.

#### Semantic Types (Fixed Ranges)

- ratio: 0.0 .. 1.0 (normalized value)
  - For intensities, sizes, and other normalized values
- signed_ratio: -1.0 .. 1.0 (bidirectional normalized value)
  - For speeds, factors, and bidirectional controls
- angle: 0.0 .. PI (radians)
  - For rotations and angular measurements
- signed_angle: -PI .. PI (radians)
  - For relative angles and bidirectional rotations
Note: Semantic types have fixed ranges that cannot be overridden. Any attempt to specify custom min/max values for these types will result in a validation error.

#### Basic Types (Custom Ranges)

- range: min .. max (float, requires range)
- count: min .. max (integer, requires range)
Note: These types require explicit min/max values to be specified.

#### Choice Types

- select: Named options that map to integer indices
- Requires `values` field with either:
  - Simple list: `values: ["sphere", "fountain", "cascade"]`
    - Like HTML <select> with sequential values (0,1,2...)
  - Value mapping: `values: {forward: 1, reverse: -1, oscillate: 0}`
    - Like HTML <select> with explicit value="..." attributes
- Requires `default` that matches one of the values
- Example: Animation patterns, modes, directions
- switch: Boolean true/false

#### Resource Types

- palette: Reference to a color palette
- bitmap: Reference to an image resource

### [3.4] Parameter Flags

Parameter flags control how values are handled when they change. They help create
smoother animations and prevent unwanted behavior. Multiple flags can be combined
to achieve the desired effect.

Flags are simple bit flags (up to 32) that can be present or not on a parameter.
The system only validates that flags are known - their actual behavior is implemented
by controllers and settings processors later.

Example flags:

• `clamp`: Suggests values should be limited to their range
• `wrap`: Suggests values should wrap around their range
• `slew`: Suggests value changes should be rate-limited

Example:

```yaml
speed:
  type: ratio
  default: 0.5
  flags: [clamp]  # Suggest range limiting
  description: Animation speed
```


### Parameter Types

Parameters use semantic types to make common animation controls more intuitive and safer.
For example, instead of defining float ranges manually, use semantic types like `ratio`
for values like brightness or opacity. The ranges will be handled automatically.

> Note: Parameter types and flags in YAML and config() use lowercase with underscores.
> The generated code will use the appropriate ParamType and ParamFlag enums.

Common parameter types and their uses:

```yaml
speed:
  type: signed_ratio  # Float [-1.0, 1.0] for bidirectional motion
  default: 0.5
  flags: [clamp]

brightness:
  type: ratio        # Float [0.0, 1.0] for intensities
  default: 0.8
  flags: [wrap]
```

## [4] Scenes and Animations

### What is a Scene?

A Scene defines an animation written in C++ that runs on the teensy and controls the animation. Scenes are called frequently (50fps+) to update the LEDs based on their parameters and internal state.

#### Structure and Files

Each scene lives in its own directory:

```bash
scenes/space/               # Scene root directory
├── space.yaml             # Parameter configuration
├── space.cpp              # Scene implementation 
├── scene_params.h         # Generated parameters (auto-included)
├── README.md              # Scene documentation
└── props/                 # Scene-specific assets
    ├── nebula.bmp         # Bitmap resource
    └── deep_space.pal     # Palette resource
```

### [4.1] Scene Lifecycle

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

## [5] Directing Scenes

The Director is responsible for selecting and transitioning between scenes. It can place animations on the stage (run them), and manage playlists and activate presets. The Director puts on the show.

## [6] Props System

Props are binary assets (palettes, bitmaps) that can be:

- Scene-specific: `/scenes/<scene>/props/`
- Global: defined in props.yaml

## [8] Build System

The project uses PlatformIO to manage builds, dependencies and testing. The build process includes code generation steps that convert scene configurations and prop data into C++ code.

## [8.1] Code Generation

### scene_generator.py: YAML to C++

The build process generates C++ code from YAML that defines parameters, presets, and props for a scene. The following shows how the files are organized and generated:

```text
scenes/
├── fireworks/
│   ├── fireworks.yaml        # Scene definition
│   ├── fireworks.cpp         # Scene implementation (setup, tick, status, reset)
│   ├── fireworks_params.h    # _Generated_ parameter structs
│   ├── README.md            # User documentation
│   └── props/               # Scene-specific props
│       └── sparks.pal.json
└── space/
    └── space.yaml
    └── space.cpp
```

### [8.2] LED Coordinates

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

### [8.3] Environments

- **teensy41**: Builds firmware with Arduino framework, generates LED points
- **native**: Builds test suite with doctest, generates LED points and test fixtures



### [11] Settings

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

### [12] Example Scene

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
    std::vector<Particle> _particles;
    
public:
    void setup() override {
        // Initialize with parameter values
        _particles.resize(100);
        
        float speed = settings<float>("speed");
        for(auto& p : _particles) {
            p.velocity = speed;
        }
    }
    
    void tick() override {

    }
};
```

## [13] Advanced Configuration

### Build Process

The build system processes scene YAML files to generate C++ code:

1. During build, `generate_scenes.py` is called for each scene YAML file
2. The generator creates a header file in the same directory as the scene YAML file named `_params.h`.
3. At compile time, the scene automatically includes `_params.h` to get the parameter definitions.

The header file uses macros to define the parameters with:

- Type mapping (signed_ratio → float with -1..1 range)
- Flag conversion (clamp → ParamFlag::Clamp)
- Range validation
- Test fixture generation
- Descriptions preserved

(see [#advanced-configuration])

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

### Manual Configuration

While YAML is recommended, parameters can also be defined in code by overriding
the config() method:

```cpp
void config() override {
    // Define parameters during initialization
    param("speed", "signed_ratio", 0.5f, "clamp");
    param("brightness", "ratio", 0.8f, "wrap");
    
    // Select with sequential values (0,1,2)
    param("pattern", "select", {
        "sphere",    // Value = 0
        "fountain",  // Value = 1
        "cascade"    // Value = 2
    }, "sphere");
}
```

> Note: If config() is overridden, YAML parameter definitions will be ignored
> to prevent confusion about parameter source.

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
    // Test semantic type ranges
    CHECK(params.speed.get() == 0.5f);          // signed_ratio [-1.0, 1.0]
    CHECK(params.brightness.set(0.8f) == true); // ratio [0.0, 1.0]
    
    // Test select with mapped values
    CHECK(params.direction.get() == -1);        // "reverse" maps to -1
}
```

The doctest framework and PlatformIO's toolchains are used for testing. The native test environment only tests the library code, not the hardware. That means the arduino framework and FastLED are mocked out.

```cpp
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
