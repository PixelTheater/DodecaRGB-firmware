---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# [3] Parameters

## [3.1] What are Parameters For?

Parameters allow for more flexibility and easier configuration. They:

- Define an interface contract between the scene and the Director
- Enable preset support for saving and loading configurations
- Provide type safety and range validation
- Allow remote control and real-time adjustment
- Make scenes more reusable and configurable

> Parameters define constraints and behaviors that help prevent errors and
> make animations more robust. They also enable features like presets and
> remote control that would be difficult to implement with raw variables.

## [3.2] Defining Parameters

Parameters are primarily defined in YAML files. This approach offers a clear and maintainable way to configure your scenes, separating the animation logic from its adjustable settings. The YAML is converted to C++ code during the build process.

```yaml
# Example Scene Configuration (YAML)
# This file defines the parameters for a specific animation scene.
# It includes the scene's name, a brief description, and a list of
# configurable parameters.

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

In this example:

- `name` and `description` provide basic information about the scene.
- The `parameters` section lists the configurable parameters, such as `speed`, `brightness`, and `pattern`.

## [3.3] Parameter Types and Ranges

Parameters serve as the foundation of our animation control system, providing a bridge between the raw code and user-friendly controls. They're designed to be both intuitive for creators and safe for runtime execution. To achieve this, parameters are strongly-typed values that control how animations behave.

Different parameter types allow you to define various kinds of controls, from simple numeric ranges to selection menus.

For example, instead of defining float ranges manually, use semantic types like `ratio`
for values like brightness or opacity. The ranges will be handled automatically.

Parameters use semantic types to make common animation controls more intuitive and safer.
For example, instead of defining float ranges manually, use semantic types like `ratio`
for values like brightness or opacity. The ranges will be handled automatically.

> Note: The YAML configuration is converted to C++ code during the build process.
> This means that the parameters defined in YAML directly influence the behavior
> of your animation code.

> Note: Parameter types and flags in YAML and config() use lowercase with underscores.
> The generated code will use the appropriate data types in the resulting header file.

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

### Semantic Types (Fixed Ranges)

- ratio: 0.0 .. 1.0 (normalized value)
  - For intensities, sizes, and other normalized values
- signed_ratio: -1.0 .. 1.0 (bidirectional normalized value)
  - For speeds, factors, and bidirectional controls
- angle: 0.0 .. PI (radians)
  - For rotations and angular measurements
- signed_angle: -PI .. PI (radians)
  - For relative angles and bidirectional rotations

Note: Semantic types have fixed ranges that cannot be overridden. Any attempt to specify custom min/max values for these types will result in a validation error.

### Basic Types (Custom Ranges)

- range: min .. max (float, requires range)
- count: min .. max (integer, requires range)
Note: These types require explicit min/max values to be specified. For count, the default range is [0,100].

### Choice Types

- switch: Boolean true/false (a "checkbox" setting)
- select: Named options that map to integer indices
- Requires `values` field with either:
  - When a `select` parameter is used, the animation code receives an integer
  - corresponding to the index of the selected value in the `values` list.
  - For example, if "fountain" is selected, the code receives the value `1`.

  - Simple list: `values: ["sphere", "fountain", "cascade"]`
  - Value mapping: `values: {forward: 1, reverse: -1, oscillate: 0}`
- Requires `default` that matches one of the values
- Example: Animation patterns, modes, directions

### Resource Types

- palette: Reference to a color palette (see [Palettes.md](Palettes.md))
- bitmap: Reference to an image resource (TODO)

## [3.4] Parameter Flags

Parameter flags control how values are handled when they change. They help create
smoother animations and prevent unwanted behavior. Multiple flags can be combined
to achieve the desired effect.

Flags control value handling based on parameter type:

- Numeric types (ratio, signed_ratio, angle, signed_angle, range):
  • `clamp`: Values are limited to their defined range
  • `wrap`: Values wrap around their defined range

- Integer types (count, select):
  • `clamp`: Values are limited to their integer range
  • `wrap`: Values wrap around their integer range

- Non-numeric types (switch, palette, bitmap):
  No flags supported - values are validated directly

Example:

```yaml
speed:
  type: ratio
  default: 0.5
  flags: [clamp]  # Suggest range limiting
  description: Animation speed
```

Note: Flags are validated at compile time and runtime. Invalid flag combinations
(like using both clamp and wrap) or unsupported flags for a type will result
in validation errors.

## [3.5] Manual Configuration

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

Example from `scenes/fireworks/fireworks.yaml`:

```yaml
name: Fireworks
description: "Colorful particle-based firework simulation"

parameters:
  # Boolean parameter
  sparkle:
    type: switch
    default: true
    description: Enable sparkle effect
  
  # Integer with range
  num_particles:
    type: count
    range: [10, 1000]
    default: 100
    description: Number of particles
    
  # Float with semantic meaning
  speed:
    type: ratio  # Automatically sets range [0.0 .. 1.0]
    default: 0.5
    flags: [clamp]
    description: Animation speed multiplier
    
  # Selection from options
  pattern:
    type: select
    values: ["sphere", "fountain", "cascade"]
    default: "sphere"
    description: Animation pattern
```

## [3.6] Parameter Inheritance

Scenes can inherit parameters from base scenes to promote code reuse and maintain consistent behavior. This is useful for:

- Creating common base classes with shared parameters
- Specializing animations while preserving core controls
- Building parameter hierarchies

### Inheritance Rules

When a scene inherits parameters:

1. Base Parameters
   - All parameters are copied from base to derived
   - Values and metadata are preserved
   - Type safety is maintained

2. Parameter Overrides
   - Derived scenes can override parameter values
   - Flags can be changed in derived scenes
   - Type must remain compatible with base

3. Extensions
   - New parameters can be added to derived scenes
   - Base parameters remain unchanged
   - No naming conflicts allowed

Example:

```cpp
// Base scene with common parameters
class BaseEffect : public Scene {
    void setup() override {
        param("speed", "ratio", 0.5f, "clamp");
        param("enabled", "switch", true);
    }
};

// Derived scene inherits and extends
class DerivedEffect : public BaseEffect {
    void setup() override {
        // First inherit base parameters
        settings.inherit_from(base_settings);
        
        // Then add or override parameters
        param("speed", "ratio", 0.8f, "wrap");  // Override with new value/flags
        param("size", "ratio", 1.0f);           // Add new parameter
    }
};
```

> Note: Parameter inheritance happens at runtime during scene initialization.
> YAML-defined parameters and manually defined parameters can be mixed freely.
