---
category: Development
generated: 2025-02-10 02:07
version: 2.8.2
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

## [3.3] Parameter Types and Ranges

Parameters serve as the foundation of our animation control system, providing a bridge between the raw code and user-friendly controls. They're designed to be both intuitive for creators and safe for runtime execution.

**Core Concept**: At their heart, parameters are strongly-typed values that control how animations behave.

Parameters use semantic types to make common animation controls more intuitive and safer.
For example, instead of defining float ranges manually, use semantic types like `ratio`
for values like brightness or opacity. The ranges will be handled automatically.

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
Note: These types require explicit min/max values to be specified.

### Choice Types

- switch: Boolean true/false (a "checkbox" setting)
- select: Named options that map to integer indices
- Requires `values` field with either:
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

Flags are simple bit flags (up to 32) that can be present or not on a parameter.
The system only validates that flags are known - their actual behavior is implemented
by controllers and settings processors later.

Example flags:

• `clamp`: Suggests values should be limited to their range
• `wrap`: Suggests values should wrap around their range
• `slew`: Suggests value changes should be rate-limited (TODO)

Example:

```yaml
speed:
  type: ratio
  default: 0.5
  flags: [clamp]  # Suggest range limiting
  description: Animation speed
```

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
